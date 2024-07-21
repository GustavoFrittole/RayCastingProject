
#include "utils.hpp"
#include <iostream>
#include "gameGraphics.hpp"

int main()
{
    GameCore gameCore{ {ScreenStats::g_screenWidth, ScreenStats::g_screenHeight, 0.5f * (1.6f), 8.f*8, 0.02f} };
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


