
#include <iostream>
#include "gameHandler.hpp"
#include <memory>

int main()
{
    std::unique_ptr<rcm::IGameHandler> gameHandler(rcm::create_gameHandler());

    try
    {
        std::cout << "Reading User defined variables..." << std::endl;
        gameHandler->load_game_data("assets/config.json");
        std::cout << "Creating assets..." << std::endl;
        gameHandler->create_assets();
        std::cout << "Starting..." << std::endl;
        gameHandler->run_game();
    }
    catch (std::exception& e)
    {
        std::cout << "An error has occured: \n" << e.what() << std::endl;
    }

    return 0;
}    

/*
TODO:
-Implement camera plane (camera vector)
-Floor and ceiling textures
-Comment code
*/

