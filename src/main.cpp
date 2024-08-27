
#include <iostream>
#include <memory>
#include <map>
#include <thread>
#include <chrono>
#include "gameHandler.hpp"

struct MyEnemy : public IEntity
{
    MyEnemy(const EntityTransform& transform, int id = 1) : IEntity(id, transform)
    {
        m_physical.speed.x = 3.f;
        m_physical.rotationSpeed = 0.8f;
        m_collisionSize = 0.3;
        m_type = EntityType::enemy;
        vulnerable = false;
    }
    void on_update() override
    {
        rcm::get_gameHandler().add_entity(rcm::create_projectile(m_transform));
    }
};

struct MySpawn : public IEntity
{
    MySpawn(const EntityTransform& transform, int id = 3) : IEntity(id, transform)
    {
        vulnerable = false;
        matata = std::thread([this] () mutable{
            while (this->active) 
            {
                this->cooldown = false;
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            }
                
            });
    }
    void on_update() override
    {
        if(!cooldown)
        {
            cooldown = true;
            rcm::get_gameHandler().add_entity(new MyEnemy(m_transform));
        }
    }
    ~MySpawn() { active = false;  matata.join(); }
private:
    std::thread matata;
    bool cooldown = false;
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
    //        IEntity* entity = new MyEnemy((x % 2 ? 1 : 3), EntityTransform{ math::Vect2{ x / 10.f, (x + y) / 10.f }, 0.f });
    //        entity->vulnerable = true;
    //        entity->m_collisionSize = 0.3f;
    //        entities.emplace_back(entity);
    //    }
    //}

    entities.emplace_back(new MyEnemy(EntityTransform{ {6,6},6 }));
    entities.emplace_back(new MySpawn({ {7,10},6 }));
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