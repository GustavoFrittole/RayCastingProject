
#include "utils.hpp"
#include <iostream>
#include "gameGraphics.hpp"

int main()
{
    GameCore gameCore{ {screenStats::g_screenWidth, screenStats::g_screenHeight, 0.5f * (3.14f), 20.f, 0.02f} };
    if (!gameCore.load_map("map.txt"))
    {
        std::cerr << "Missing \"map\" file\n";
        return -1;
    }
    GameGraphics gameGraphics(gameCore, "RayCastingWorld");

    gameGraphics.start();

    debug::GameTimer gt;

    while (gameGraphics.is_running())
    {  
        gameGraphics.handle_events();
        gameGraphics.performGameCycle();
        gt.add_frame();
        std::cout << gt.get_frame_rate() << std::endl;
    }

    return 0;
}    

/*
TODO:
-PRE RENDER BACKGROUND
-IMPLEMENT MULTITHREADING IN TEXTURE GENERATION
-IMPLEMENT PAUSE
-IMPLEMENT DDA ALGORITHM
*/

