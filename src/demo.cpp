#include <iostream>
#include <memory>
#include <map>
#include <thread>
#include <chrono>
#include "gameHandler.hpp"
#include "../demo/demoEntities.hpp"

// for more informations and comments on IEntity see gameDataStructures.hpp : rcm::IEntity
// the implementation examples of all entities used in this file can be found in "demo/demoEntities.hpp"

int main()
{
    rcm::IGameHandler& gameHandler = rcm::get_gameHandler();

    //Implementation note: entities are heap allocated and each entity is free to hold pointers 
    //to other entities. Notice that entities destruction is handled by game handler, so if an entity
    //is destroied by other means, the behavior will be undefined. Also note that all unique_ptrs fed
    //to gameHandler are returned empty, as it takes ownership of them.

    //-------------- crete entities ------------

    //the player entity
    std::unique_ptr<IEntity> player(new MyGameLogicsHandler::MyPlayer({ {5,5}, 0 }));

    //vector of entities that will be created at game start
    std::vector<std::unique_ptr<IEntity>> entities;

    entities.emplace_back(new MyGameLogicsHandler());

    //sequence of sprites that "MySpawner" will play in sequence
    const unsigned char ids[] = { 3, 4, 5, 6 };

    entities.emplace_back(new MyGameLogicsHandler::MySpawner(EntityTransform{ {7,10}, 6 }, sizeof(ids) / sizeof(ids[0]), ids));


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

    //implementaion note, error handling:
    return 0;
}