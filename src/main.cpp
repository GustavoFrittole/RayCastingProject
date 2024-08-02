
#include <iostream>
#include "gameHandler.hpp"
#include <memory>

int main()
{
    std::unique_ptr<rcm::IGameHandler> gameHandler(rcm::create_gameHandler("assets\\config.json"));
    if (gameHandler->is_good())
        gameHandler->run_game();
    else
        std::cerr << gameHandler->get_errors();
    return 0;
}    

/*
TODO:
-Implement camera plane (camera vector)
-Floor and sky textures
-Comment code
*/

