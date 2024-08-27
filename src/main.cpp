
#include <iostream>
#include <memory>
#include <map>
#include "gameHandler.hpp"

struct MyEntity : public IEntity
{
    MyEntity(const EntityTransform& transform, int id = 1) : IEntity(id, transform)
    {
        m_physical.speed.x = 0.5;
        m_physical.rotationSpeed = 0.4;
    }
    void on_update() override
    {
        rcm::get_gameHandler().add_entity(rcm::create_projectile(m_transform));
    }
};

struct MySpawn : public IEntity
{
    MySpawn(const EntityTransform& transform, int id = 2) : IEntity(id, transform)
    {
        m_physical.speed.x = 0.1;
    }
    void on_update() override
    {
        m_physical.rotationSpeed += 0.1;
    }
};

int main()
{
    rcm::IGameHandler& gameHandler = rcm::get_gameHandler();

    //-------------- crete entities ------------

    std::vector<std::unique_ptr<IEntity>> entities;

    //int start = 20;
    //int end = 50;

    //for (int x = start; x < end; x += 3)
    //{
    //    for (int y = start; y < end; y += 3)
    //    {
    //        IEntity* entity = new MyEntity((x % 2 ? 1 : 3), EntityTransform{ math::Vect2{ x / 10.f, (x + y) / 10.f }, 0.f });
    //        entity->vulnerable = true;
    //        entity->m_collisionSize = 0.3f;
    //        entities.emplace_back(entity);
    //    }
    //}
    std::cout << &rcm::get_gameHandler() << std::endl;

    entities.emplace_back(new MyEntity(EntityTransform{ {6,6},6 }));

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