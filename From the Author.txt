

-----------------------------------------------------------------------------------

This game is made using the C++ language, natively in macOS.

RayLib Library is used for GUI, and stream for file handling.

----------------------------------- HOW TO RUN ------------------------------------

Run this code in Mac from terminal.

Compile the C++ file:
g++ /path/to/your/ping_pong_game_code.cpp

Specify output file:
-o /path/to/output/Ping_Pong

Include directories for Raylib:
-I/opt/homebrew/Cellar/raylib/5.5/include

Link libraries:
-L/opt/homebred/Cellar/raylib/5.5/lib -lraylib

Linking with necessary macOS frameworks:
-framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo

C++ Standard:
-std=c++11

Link to / Create Resources Directory containing the audio files being used:
mkdir -p resources

Change directory to project folder:
cd /path/to/your/project

Run the compiled program:
./Ping_Pong


----------------------------------- REQUIREMENTS -----------------------------------

Install RayLib using Brew

Install SOL using Brew
