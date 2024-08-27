
#include <iostream>
#include <memory>
#include <map>
#include "gameHandler.hpp"

int main()
{
    rcm::IGameHandler& gameHandler = rcm::get_gameHandler();

    //-------------- crete entitys ------------

    std::vector<Entity> entities;

    int start = 20;
    int end = 50;

    for (int x = start; x < end; x += 3)
    {
        for (int y = start; y < end; y += 3)
        {
            Entity entity((x % 2 ? 1 : 3), EntityTransform{ math::Vect2{ x / 10.f, (x + y) / 10.f }, 0.f });
            entity.vulnerable = true;
            entity.m_collisionSize = 0.3f;
            entities.push_back(entity);
        }
    }

    //---------------- run game ---------------
    try
    {
        std::cout << "Reading User defined variables..." << std::endl;
        gameHandler.load_game_data("assets/config.json");
        std::cout << "Creating assets..." << std::endl;
        gameHandler.create_assets(entities);
        std::cout << "Starting..." << std::endl;
    }
    catch (std::exception& e)
    {
        std::cout << "An error has occured: \n" << e.what() << std::endl;
    }

    gameHandler.run_game();

    return 0;
}