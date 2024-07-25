
#include "utils.hpp"
#include <iostream>
#include "gameGraphics.hpp"
#include <random>

int main()
{
    GameCore gameCore{ {screenStats::g_screenWidth, screenStats::g_screenHeight, 0.5f * (3.14f), 8.f, 0.02f}, "map.txt" };
    GameGraphics gameGraphics(gameCore, "RayCastingWorld");

    gameGraphics.start();

    debug::GameTimer gt;

    while (gameGraphics.is_running())
    {  
        gameGraphics.performGameCycle();
        gt.add_frame();
        if (gt.get_frame_count() > 20)
        {
            std::cout << gt.get_frame_rate() << std::endl;
        }
        
    }

    return 0;
}    

/*
TODO:
-PRE RENDER BACKGROUND
-IMPLEMENT MULTITHREADING IN TEXTURE GENERATION
-IMPLEMENT DDA ALGORITHM
-IMPLEMENT ZBUFFER
*/

