
#include <iostream>
#include <memory>
#include "gameHandler.hpp"
#include <map>

int main()
{
    std::unique_ptr<rcm::IGameHandler> gameHandler(rcm::create_gameHandler());
    std::vector<Entity> entities;

    int start = 20;
    int end = 80;

    for (int i = start; i < end; i += 3)
    {
        entities.emplace_back(i % 2, EntityTransform{ math::Vect2{ i / 10.f, i / 10.f }, 0.f });
    }

    entities.push_back(Entity(1, EntityTransform{ math::Vect2(5.f,5.f), 0.1f }));

    try
    {
        std::cout << "Reading User defined variables..." << std::endl;
        gameHandler->load_game_data("assets/config.json");
        std::cout << "Creating assets..." << std::endl;
        gameHandler->create_assets(entities);
        std::cout << "Starting..." << std::endl;
    }
    catch (std::exception& e)
    {
        std::cout << "An error has occured: \n" << e.what() << std::endl;
    }
    gameHandler->run_game();
    return 0;
}