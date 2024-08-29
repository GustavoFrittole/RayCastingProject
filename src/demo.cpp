
#include <iostream>
#include <memory>
#include <map>
#include <thread>
#include <chrono>
#include "gameHandler.hpp"
#include "../demo/demoEntities.hpp"

//-------------------------------------------------------------

int main()
{
    rcm::IGameHandler& gameHandler = rcm::get_gameHandler();

    //-------------- crete entities ------------

    std::vector<std::unique_ptr<IEntity>> entities;
    std::unique_ptr<IEntity> player(new MyPlayer({ {5,5}, 0 }));

    entities.emplace_back(new MyEnemy(EntityTransform{ {6,6},6 }));
    entities.emplace_back(new MySpawn({ {7,10},6 }));

    //---------------- run game ---------------
    try
    {
        std::cout << "Reading User defined variables..." << std::endl;
        gameHandler.load_game_data("assets/config.json", player);
        std::cout << "Creating assets..." << std::endl;
        gameHandler.create_assets(entities);
        std::cout << "Starting..." << std::endl;   
        gameHandler.run_game();
    }
    catch (std::exception& e)
    {
        std::cout << "An error has occured: \n" << e.what() << std::endl;
    }
    return 0;
}