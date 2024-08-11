
#include <iostream>
#include "gameHandler.hpp"
#include <memory>

int main()
{
    std::unique_ptr<rcm::IGameHandler> gameHandler(rcm::create_gameHandler("assets/config.json"));
    if (gameHandler->is_good())
    {
        
        try
        {
            gameHandler->run_game();
        }
        catch (std::exception& e)
        {
            std::cout << "An error has occured: \n" << e.what() << std::endl;
        }
    }
    else
    {
        std::cerr << gameHandler->get_errors();
    }
        
    return 0;
}    

/*
TODO:
-Implement camera plane (camera vector)
-Floor and ceiling textures
-Comment code
*/

