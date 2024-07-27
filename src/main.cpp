
#include <iostream>
#include "gameGraphics.hpp"

int main()
{
    GameGraphics gameGraphics( {screenStats::g_screenWidth, screenStats::g_screenHeight, 0.5f * (3.14f), 8.f, 0.02f}, "assets\\map.txt" , "RayCastingWorld");

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
-IMPLEMENT DDA ALGORITHM
-IMPLEMENT ZBUFFER
-ACCELLERATION IN CONTROLS
-WAY OUT FINDER
*/

