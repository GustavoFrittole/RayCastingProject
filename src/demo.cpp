
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

    MyGameLogicsHandler* handlerP = new MyGameLogicsHandler();

    MyGameLogicsHandler::MyPlayer* playerP = new MyGameLogicsHandler::MyPlayer({ {5,5}, 0 });
    handlerP->set_player(playerP);
    std::unique_ptr<IEntity> player(playerP);

    const unsigned char ids[] = { 3, 4, 5, 6 };
    IEntity* spawnerP = new MyGameLogicsHandler::MySpawner(EntityTransform{ {7,10}, 6 }, sizeof(ids) / sizeof(ids[0]), ids);
    spawnerP->m_billboard.size = 0.5f;
    spawnerP->m_billboard.alignment = SpriteAlignment::Floor;

    entities.emplace_back(spawnerP);
    entities.emplace_back(handlerP);

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