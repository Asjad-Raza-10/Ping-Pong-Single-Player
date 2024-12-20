#include "raylib.h"
#include <iostream>
#include <fstream>
#include <cstring>

using namespace std;

// Window dimensions
const int screenWidth = 1700;
const int screenHeight = 980;

// Ball and paddle properties
const int ballRadius = 10;
const int paddleWidth = 10;
const int paddleHeight = 100;
const float BALL_SPEED_SLOW = 300.0f;     // Very slow and easy to play
const float BALL_SPEED_MEDIUM = 600.0f;   // Balanced speed
const float BALL_SPEED_FAST = 1000.0f;     // Very challenging speed
float currentBallSpeed = BALL_SPEED_MEDIUM;
const float paddleSpeed = 450.0f;

// Theme colors
Color backgroundColor = BLACK;
Color foregroundColor = WHITE;
bool isDarkTheme = true;

// Button properties
const int buttonWidth = 300;
const int buttonHeight = 60;
const int buttonSpacing = 80;
const int buttonTextSize = 30;

// Leaderboard constants
const int MAX_NAME_LENGTH = 50;
const int MAX_LEADERBOARD_ENTRIES = 100;

// Global leaderboard arrays
char leaderboardNames[MAX_LEADERBOARD_ENTRIES][MAX_NAME_LENGTH];
int leaderboardScores[MAX_LEADERBOARD_ENTRIES];

// Game states
enum GameState {
    MENU,
    GAME,
    LEADERBOARD,
    OPTIONS,
    QUIT
};

// Sound effects (global)
Sound bounceSound;
Sound buttonSound;
Sound gameoverSound;
Sound highscoreSound;
float transitionAlpha = 1.0f;

// Function prototypes
void UpdateGame(float deltaTime, Vector2 &ballPosition, Vector2 &ballVelocity, Rectangle &paddle, int &score, bool &gameOver);
void ResetGame(Vector2 &ballPosition, Vector2 &ballVelocity, Rectangle &paddle, int &score, bool &gameOver);
void SaveScore(const char* name, int score);
int LoadLeaderboard();
void SortLeaderboard(int size);
void HandleNameInput(char* playerName, bool &nameEntered);
void DrawMenu(GameState& currentState);
void DrawLeaderboard(int &leaderboardSize, int &scrollOffset, GameState &currentState);
void DrawOptions(GameState &currentState);
void DrawPaused();
void HandlePause(bool &paused, GameState &currentState);
int FindNameIndex(const char* name, int size);
void ClearLeaderboard();
void DrawTransition();
bool IsHighScore(int score, int leaderboardSize);

// Other helper functions
int FindNameIndex(const char* name, int size) {
    for (int i = 0; i < size; i++) {
        if (strcmp(name, leaderboardNames[i]) == 0) {
            return i;
        }
    }
    return -1;
}

void SortLeaderboard(int size) {
    for (int i = 0; i < size - 1; i++) {
        for (int j = 0; j < size - i - 1; j++) {
            if (leaderboardScores[j] < leaderboardScores[j + 1]) {
                // Swap scores
                int tempScore = leaderboardScores[j];
                leaderboardScores[j] = leaderboardScores[j + 1];
                leaderboardScores[j + 1] = tempScore;

                // Swap names
                char tempName[MAX_NAME_LENGTH];
                strcpy(tempName, leaderboardNames[j]);
                strcpy(leaderboardNames[j], leaderboardNames[j + 1]);
                strcpy(leaderboardNames[j + 1], tempName);
            }
        }
    }
}

void SaveScore(const char* name, int score) {
    ofstream file("scores.txt", ios::app);
    if (file.is_open()) {
        file << name << " " << score << endl;
        file.close();
    }
}

int LoadLeaderboard() {
    ifstream file("scores.txt");
    int count = 0;

    if (file.is_open()) {
        while (count < MAX_LEADERBOARD_ENTRIES && file >> leaderboardNames[count] >> leaderboardScores[count]) {
            count++;
        }
        file.close();
    }

    SortLeaderboard(count);
    return count;
}

void DrawMenu(GameState &currentState) {
    int centerX = screenWidth / 2;
    int centerY = screenHeight / 2;

    // Center the title
    const char* title = "Ping Pong Game";
    int titleWidth = MeasureText(title, 40);
    DrawText(title, centerX - titleWidth/2, 100, 40, foregroundColor);

    Rectangle buttons[] = {
        {static_cast<float>(centerX - buttonWidth / 2), static_cast<float>(centerY - buttonHeight - buttonSpacing), static_cast<float>(buttonWidth), static_cast<float>(buttonHeight)},
        {static_cast<float>(centerX - buttonWidth / 2), static_cast<float>(centerY), static_cast<float>(buttonWidth), static_cast<float>(buttonHeight)},
        {static_cast<float>(centerX - buttonWidth / 2), static_cast<float>(centerY + buttonHeight + buttonSpacing), static_cast<float>(buttonWidth), static_cast<float>(buttonHeight)},
        {static_cast<float>(centerX - buttonWidth / 2), static_cast<float>(centerY + 2 * (buttonHeight + buttonSpacing)), static_cast<float>(buttonWidth), static_cast<float>(buttonHeight)}
    };

    const char* buttonTexts[] = {"Play", "Leaderboard", "Options", "Quit"};
    Vector2 mousePoint = GetMousePosition();

    for (int i = 0; i < 4; i++) {
        bool hovered = CheckCollisionPointRec(mousePoint, buttons[i]);
        int textWidth = MeasureText(buttonTexts[i], buttonTextSize);
        DrawText(buttonTexts[i], buttons[i].x + (buttonWidth - textWidth)/2, buttons[i].y + 15, buttonTextSize, hovered ? RED : foregroundColor);

        if (hovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            PlaySound(buttonSound);
            switch (i) {
                case 0: currentState = GAME; break;
                case 1: currentState = LEADERBOARD; break;
                case 2: currentState = OPTIONS; break;
                case 3: currentState = QUIT; break;
            }
        }
    }
}

void DrawLeaderboard(int &leaderboardSize, int &scrollOffset, GameState &currentState) {
    DrawText("Leaderboard", screenWidth / 2 - 100, 50, 30, foregroundColor);

    const int entriesPerPage = 20;
    if (IsKeyDown(KEY_W) && scrollOffset > 0) scrollOffset--;
    if (IsKeyDown(KEY_S) && scrollOffset < leaderboardSize - entriesPerPage) scrollOffset++;

    // Only show entries if there are any
    if (leaderboardSize > 0) {
        int yOffset = 100;
        for (int i = scrollOffset; i < leaderboardSize && i < scrollOffset + entriesPerPage; i++) {
            DrawText(TextFormat("%d. %s - %d", i + 1, leaderboardNames[i], leaderboardScores[i]),
                     screenWidth / 2 - 200, yOffset, 20, foregroundColor);
            yOffset += 30;
        }
    } else {
        // Show "No scores yet" message if leaderboard is empty
        const char* emptyMessage = "No scores yet!";
        int messageWidth = MeasureText(emptyMessage, 30);
        DrawText(emptyMessage, screenWidth/2 - messageWidth/2, screenHeight/2 - 15, 30, foregroundColor);
    }
    
    // Return to Menu button
    Rectangle menuButton = {
        static_cast<float>(screenWidth/2 - 175),
        static_cast<float>(screenHeight - 100),
        150,
        40
    };
    
    // Clear Leaderboard button
    Rectangle clearButton = {
        static_cast<float>(screenWidth/2 + 25),
        static_cast<float>(screenHeight - 100),
        150,
        40
    };
    
    // Draw buttons
    DrawRectangleRec(menuButton, backgroundColor);
    DrawRectangleLines(menuButton.x, menuButton.y, menuButton.width, menuButton.height, foregroundColor);
    DrawText("Main Menu", menuButton.x + (150 - MeasureText("Main Menu", 20))/2, 
             menuButton.y + 10, 20, foregroundColor);
             
    DrawRectangleRec(clearButton, backgroundColor);
    DrawRectangleLines(clearButton.x, clearButton.y, clearButton.width, clearButton.height, RED);
    DrawText("Clear", clearButton.x + (150 - MeasureText("Clear", 20))/2, 
             clearButton.y + 10, 20, RED);
             
    // Handle button clicks
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        Vector2 mousePoint = GetMousePosition();
        if (CheckCollisionPointRec(mousePoint, menuButton)) {
            PlaySound(buttonSound);
            currentState = MENU;
        }
        else if (CheckCollisionPointRec(mousePoint, clearButton)) {
            ClearLeaderboard();
            leaderboardSize = 0;  // Reset size to 0 since leaderboard is now empty
            scrollOffset = 0;     // Reset scroll position
        }
    }
}

void DrawOptions(GameState &currentState) {
    int centerX = screenWidth / 2;
    int centerY = screenHeight / 3;
    
    const int optionButtonWidth = 150;
    const int optionButtonHeight = 40;
    const int optionSpacing = 30;
    
    // Draw Theme heading
    DrawText("Theme", centerX - MeasureText("Theme", 40)/2, centerY - 80, 40, foregroundColor);
    
    // Theme buttons
    Rectangle lightButton = {
        static_cast<float>(centerX - optionButtonWidth - 20),
        static_cast<float>(centerY - 20),
        static_cast<float>(optionButtonWidth),
        static_cast<float>(optionButtonHeight)
    };
    Rectangle darkButton = {
        static_cast<float>(centerX + 20),
        static_cast<float>(centerY - 20),
        static_cast<float>(optionButtonWidth),
        static_cast<float>(optionButtonHeight)
    };
    
    // Draw theme buttons with selection indicator
    DrawRectangleRec(lightButton, !isDarkTheme ? LIGHTGRAY : backgroundColor);
    DrawRectangleLines(lightButton.x, lightButton.y, lightButton.width, lightButton.height, foregroundColor);
    DrawText("Light", lightButton.x + (optionButtonWidth - MeasureText("Light", 20))/2, 
             lightButton.y + 10, 20, foregroundColor);
    
    DrawRectangleRec(darkButton, isDarkTheme ? LIGHTGRAY : backgroundColor);
    DrawRectangleLines(darkButton.x, darkButton.y, darkButton.width, darkButton.height, foregroundColor);
    DrawText("Dark", darkButton.x + (optionButtonWidth - MeasureText("Dark", 20))/2, 
             darkButton.y + 10, 20, foregroundColor);
    
    // Draw Speed heading
    DrawText("Ball Speed", centerX - MeasureText("Ball Speed", 40)/2, centerY + 80, 40, foregroundColor);
    
    // Speed buttons
    Rectangle slowButton = {
        static_cast<float>(centerX - optionButtonWidth*1.5f - 20),
        static_cast<float>(centerY + 140),
        static_cast<float>(optionButtonWidth),
        static_cast<float>(optionButtonHeight)
    };
    Rectangle mediumButton = {
        static_cast<float>(centerX - optionButtonWidth/2),
        static_cast<float>(centerY + 140),
        static_cast<float>(optionButtonWidth),
        static_cast<float>(optionButtonHeight)
    };
    Rectangle fastButton = {
        static_cast<float>(centerX + optionButtonWidth/2 + 20),
        static_cast<float>(centerY + 140),
        static_cast<float>(optionButtonWidth),
        static_cast<float>(optionButtonHeight)
    };
    
    // Draw speed buttons with selection indicator
    DrawRectangleRec(slowButton, currentBallSpeed == BALL_SPEED_SLOW ? LIGHTGRAY : backgroundColor);
    DrawRectangleLines(slowButton.x, slowButton.y, slowButton.width, slowButton.height, foregroundColor);
    DrawText("Slow", slowButton.x + (optionButtonWidth - MeasureText("Slow", 20))/2, 
             slowButton.y + 10, 20, foregroundColor);
    
    DrawRectangleRec(mediumButton, currentBallSpeed == BALL_SPEED_MEDIUM ? LIGHTGRAY : backgroundColor);
    DrawRectangleLines(mediumButton.x, mediumButton.y, mediumButton.width, mediumButton.height, foregroundColor);
    DrawText("Medium", mediumButton.x + (optionButtonWidth - MeasureText("Medium", 20))/2, 
             mediumButton.y + 10, 20, foregroundColor);
    
    DrawRectangleRec(fastButton, currentBallSpeed == BALL_SPEED_FAST ? LIGHTGRAY : backgroundColor);
    DrawRectangleLines(fastButton.x, fastButton.y, fastButton.width, fastButton.height, foregroundColor);
    DrawText("Fast", fastButton.x + (optionButtonWidth - MeasureText("Fast", 20))/2, 
             fastButton.y + 10, 20, foregroundColor);
    
    // Return to Menu button
    Rectangle menuButton = {
        static_cast<float>(centerX - optionButtonWidth/2),
        static_cast<float>(centerY + 250),
        static_cast<float>(optionButtonWidth),
        static_cast<float>(optionButtonHeight)
    };
    DrawRectangleRec(menuButton, backgroundColor);
    DrawRectangleLines(menuButton.x, menuButton.y, menuButton.width, menuButton.height, foregroundColor);
    DrawText("Main Menu", menuButton.x + (optionButtonWidth - MeasureText("Main Menu", 20))/2, 
             menuButton.y + 10, 20, foregroundColor);
    
    // Handle button clicks
    Vector2 mousePoint = GetMousePosition();
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if (CheckCollisionPointRec(mousePoint, lightButton)) {
            PlaySound(buttonSound);
            isDarkTheme = false;
            backgroundColor = WHITE;
            foregroundColor = BLACK;
        }
        else if (CheckCollisionPointRec(mousePoint, darkButton)) {
            PlaySound(buttonSound);
            isDarkTheme = true;
            backgroundColor = BLACK;
            foregroundColor = WHITE;
        }
        else if (CheckCollisionPointRec(mousePoint, slowButton)) {
            PlaySound(buttonSound);
            currentBallSpeed = BALL_SPEED_SLOW;
        }
        else if (CheckCollisionPointRec(mousePoint, mediumButton)) {
            PlaySound(buttonSound);
            currentBallSpeed = BALL_SPEED_MEDIUM;
        }
        else if (CheckCollisionPointRec(mousePoint, fastButton)) {
            PlaySound(buttonSound);
            currentBallSpeed = BALL_SPEED_FAST;
        }
        else if (CheckCollisionPointRec(mousePoint, menuButton)) {
            PlaySound(buttonSound);
            currentState = MENU;
        }
    }
}

void DrawPaused() {
    const char* pausedText = "Game Paused";
    const char* resumeText = "Press 'P' to Resume";
    int pausedWidth = MeasureText(pausedText, 60);    // Stays at 60
    int resumeWidth = MeasureText(resumeText, 25);    // Increased from 20 to 25
    
    DrawText(pausedText, screenWidth/2 - pausedWidth/2, screenHeight/2 - 50, 60, foregroundColor);
    DrawText(resumeText, screenWidth/2 - resumeWidth/2, screenHeight/2 + 20, 25, LIGHTGRAY);
}

void HandlePause(bool &paused, GameState &currentState) {
    if (IsKeyPressed(KEY_P)) {
        paused = !paused;
    }
}

void UpdateGame(float deltaTime, Vector2 &ballPosition, Vector2 &ballVelocity, Rectangle &paddle, int &score, bool &gameOver) {
    // Update ball position
    ballPosition.x += ballVelocity.x * deltaTime;
    ballPosition.y += ballVelocity.y * deltaTime;

    // Handle top and bottom wall collisions with proper boundary checking
    if (ballPosition.y - ballRadius <= 0) {
        ballPosition.y = ballRadius;
        ballVelocity.y = fabs(ballVelocity.y);
        PlaySound(bounceSound);
    }
    if (ballPosition.y + ballRadius >= screenHeight) {
        ballPosition.y = screenHeight - ballRadius;
        ballVelocity.y = -fabs(ballVelocity.y);
        PlaySound(bounceSound);
    }

    // Handle paddle collision with improved physics
    if (CheckCollisionCircleRec(ballPosition, ballRadius, paddle)) {
        if (ballVelocity.x < 0) {  // Only bounce if moving towards paddle
            PlaySound(bounceSound);
            
            // Calculate relative intersection position with the paddle (-1 to 1)
            float relativeIntersectY = (ballPosition.y - (paddle.y + paddle.height/2)) / (paddle.height/2);
            float bounceAngle = relativeIntersectY * 60;  // Max 60 degree bounce angle
            
            // Get current speed and increase it by 5%
            float speed = sqrt(ballVelocity.x * ballVelocity.x + ballVelocity.y * ballVelocity.y);
            speed *= 1.05f;  // Increase speed by 5% with each hit
            
            // Apply the new speed with the bounce angle
            ballVelocity.x = fabs(speed * cos(bounceAngle * DEG2RAD));
            ballVelocity.y = speed * sin(bounceAngle * DEG2RAD);
            
            score++;
        }
    }

    // Handle game over condition
    if (ballPosition.x - ballRadius <= 0) {
        gameOver = true;
        PlaySound(gameoverSound);  // Play game over sound
        
        // Check and play high score sound if achieved
        int leaderboardSize = LoadLeaderboard();
        if (IsHighScore(score, leaderboardSize)) {
            PlaySound(highscoreSound);
        }
    }

    // Handle right wall collision
    if (ballPosition.x + ballRadius >= screenWidth) {
        ballPosition.x = screenWidth - ballRadius;
        ballVelocity.x *= -1;
        PlaySound(bounceSound);
    }

    // Update paddle position with boundary checking
    if (IsKeyDown(KEY_UP)) {
        paddle.y = fmax(0, paddle.y - paddleSpeed * deltaTime);
    }
    if (IsKeyDown(KEY_DOWN)) {
        paddle.y = fmin(screenHeight - paddle.height, paddle.y + paddleSpeed * deltaTime);
    }
}

void HandleNameInput(char* playerName, bool &nameEntered) {
    const char* prompt = "Enter your name:";
    int promptWidth = MeasureText(prompt, 30);
    int nameWidth = MeasureText(playerName, 30);
    
    // Calculate dynamic box width based on name length, with minimum width
    int boxWidth = fmax(10, nameWidth + 40);  // minimum width of 10px, or name width + padding
    const int boxHeight = 40;
    
    // Calculate vertical positions
    int boxY = screenHeight/2 - 10;
    int textY = boxY + (boxHeight - 30)/2;  // Center text vertically within box (30 is text height)
    
    // Draw prompt and input box centered
    DrawText(prompt, screenWidth/2 - promptWidth/2, boxY - 50, 30, foregroundColor);
    DrawRectangleLines(
        screenWidth/2 - boxWidth/2,  // Center the box horizontally
        boxY,
        boxWidth,                    // Dynamic width
        boxHeight,
        LIGHTGRAY
    );
    DrawText(playerName, screenWidth/2 - nameWidth/2, textY, 30, foregroundColor);

    int key = GetKeyPressed();
    if (key >= 32 && key <= 126 && strlen(playerName) < MAX_NAME_LENGTH - 1) {
        int len = strlen(playerName);
        playerName[len] = (char)key;
        playerName[len + 1] = '\0';
    }
    else if (key == KEY_BACKSPACE && strlen(playerName) > 0) {
        playerName[strlen(playerName) - 1] = '\0';
    }
    else if (key == KEY_ENTER && strlen(playerName) > 0) {
        nameEntered = true;
        PlaySound(buttonSound);  // Play click sound when name is entered
    }
}

void ResetGame(Vector2 &ballPosition, Vector2 &ballVelocity, Rectangle &paddle, int &score, bool &gameOver) {
    ballPosition = {(float)screenWidth / 2, (float)screenHeight / 2};
    // Set initial velocity based on current speed setting - directly use selected speed
    ballVelocity = {-currentBallSpeed, currentBallSpeed};  // 45-degree angle
    paddle.y = (float)screenHeight / 2 - paddle.height / 2;
    score = 0;
    gameOver = false;
}

void ClearLeaderboard() {
    // Clear the file
    ofstream file("scores.txt", ios::trunc);
    file.close();
}

void DrawTransition() {
    if (transitionAlpha > 0.0f) {
        DrawRectangle(0, 0, screenWidth, screenHeight, ColorAlpha(BLACK, transitionAlpha));
        transitionAlpha -= 0.02f;
    }
}

bool IsHighScore(int score, int leaderboardSize) {
    if (leaderboardSize == 0) return true;
    // Only compare with the highest score (first score after sorting)
    return score > leaderboardScores[0];
}

int main() {
    InitWindow(screenWidth, screenHeight, "Ping Pong Game");
    InitAudioDevice();      // Initialize audio device
    SetTargetFPS(120);

    // Load sounds directly
    bounceSound = LoadSound("resources/bounce.wav");    // Default raylib sound
    buttonSound = LoadSound("resources/button.wav");    // Default raylib sound
    gameoverSound = LoadSound("resources/gameover.wav");
    highscoreSound = LoadSound("resources/highscore.wav");
    
    // Set volumes
    SetSoundVolume(bounceSound, 0.5f);
    SetSoundVolume(buttonSound, 0.5f);
    SetSoundVolume(gameoverSound, 0.5f);
    SetSoundVolume(highscoreSound, 0.8f);  // Higher volume for high score sound

    Vector2 ballPosition = {(float)screenWidth / 2, (float)screenHeight / 2};
    Vector2 ballVelocity = {-currentBallSpeed, currentBallSpeed};
    Rectangle paddle = {30, (float)screenHeight / 2 - paddleHeight / 2, (float)paddleWidth, (float)paddleHeight};
    int score = 0;
    bool gameOver = false;

    bool nameEntered = false;
    char playerName[MAX_NAME_LENGTH] = {'\0'};
    GameState currentState = MENU;

    int leaderboardSize = LoadLeaderboard();
    int scrollOffset = 0;

    bool paused = false;

    while (!WindowShouldClose()) {
        // Update music stream
        // UpdateMusicStream(backgroundMusic);
        
        BeginDrawing();
        ClearBackground(backgroundColor);

        // Draw fade transition
        DrawTransition();

        switch (currentState) {
            case MENU:
                DrawMenu(currentState);
                if (currentState == GAME) {  // If state changed to GAME in DrawMenu
                    PlaySound(buttonSound);  // Play click sound when starting game
                    ResetGame(ballPosition, ballVelocity, paddle, score, gameOver);
                    nameEntered = false;
                    memset(playerName, 0, MAX_NAME_LENGTH);
                }
                break;

            case GAME:
                if (!nameEntered) {
                    HandleNameInput(playerName, nameEntered);
                } else if (paused) {
                    DrawPaused();
                    HandlePause(paused, currentState);
                } else if (gameOver) {
                    const char* gameOverText = "Game Over!";
                    const char* scoreText = TextFormat("Score: %d", score);
                    const char* menuText = "Press ENTER to return to menu";
                    
                    int gameOverWidth = MeasureText(gameOverText, 60);
                    int scoreWidth = MeasureText(scoreText, 40);
                    int menuWidth = MeasureText(menuText, 25);
                    
                    DrawText(gameOverText, screenWidth/2 - gameOverWidth/2, screenHeight/2 - 120, 60, RED);
                    DrawText(scoreText, screenWidth/2 - scoreWidth/2, screenHeight/2, 40, foregroundColor);
                    DrawText(menuText, screenWidth/2 - menuWidth/2, screenHeight/2 + 100, 25, LIGHTGRAY);

                    // Check and display high score message
                    if (IsHighScore(score, leaderboardSize)) {
                        const char* newHighScoreText = "New High Score!";
                        int highScoreWidth = MeasureText(newHighScoreText, 50);
                        DrawText(newHighScoreText, screenWidth/2 - highScoreWidth/2, 
                               screenHeight/2 - 180, 50, GOLD);
                    }

                    if (IsKeyPressed(KEY_ENTER)) {
                        PlaySound(buttonSound);  // Play click sound when returning to menu
                        SaveScore(playerName, score);
                        leaderboardSize = LoadLeaderboard();
                        ResetGame(ballPosition, ballVelocity, paddle, score, gameOver);
                        currentState = MENU;
                    }
                } else {
                    float deltaTime = GetFrameTime();
                    UpdateGame(deltaTime, ballPosition, ballVelocity, paddle, score, gameOver);
                    HandlePause(paused, currentState);

                    DrawCircleV(ballPosition, ballRadius, foregroundColor);
                    DrawRectangleRec(paddle, foregroundColor);
                    DrawText(TextFormat("Score: %d", score), 20, 20, 20, foregroundColor);
                }
                break;

            case LEADERBOARD:
                DrawLeaderboard(leaderboardSize, scrollOffset, currentState);
                if (IsKeyPressed(KEY_M)) currentState = MENU;
                break;

            case OPTIONS:
                DrawOptions(currentState);
                break;

            case QUIT:
                CloseWindow();
                break;
        }

        EndDrawing();
    }

    // Cleanup
    UnloadSound(bounceSound);
    UnloadSound(buttonSound);
    UnloadSound(gameoverSound);
    UnloadSound(highscoreSound);
    CloseAudioDevice();
    CloseWindow();
    return 0;
}