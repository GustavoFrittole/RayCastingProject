
#include <iostream>
#include <memory>
#include <map>
#include <thread>
#include <chrono>
#include "gameHandler.hpp"

//------------------------- Custom Entities Examples ------------------------

struct MyEnemy : public IEntity
{
    MyEnemy(const EntityTransform& transform, int id = 1) : IEntity(id, transform)
    {
        m_physical.speed.x = 2.5f;
        m_physical.rotationSpeed = 0.8f;
        m_physical.mass = 1.f;
        m_collisionSize = 0.2f;
        m_type = EntityType::enemy;
        interactible = true;
    }
    void on_update() override
    {
        if (cooldown.is_ready())
        {
            EntityTransform projectileTransform = { m_transform.coords + math::rad_to_vec(m_transform.forewardAngle) * (m_collisionSize + 0.2f), m_transform.forewardAngle };
            rcm::get_gameHandler().add_entity(rcm::create_projectile(projectileTransform));
        }
    }
    void on_hit(EntityType otherEntity) override
    {
        if (otherEntity == EntityType::projectile)
        {
            destroyed = true;
        }
    }
private:
    utils::SimpleCooldown cooldown = utils::SimpleCooldown(500);
};

struct MyPlayer : public IEntity
{
    MyPlayer(const EntityTransform& transform, int id = -1) : IEntity(id, transform)
    {
        m_physical.speed = { 1,0 };
        m_physical.movFrictionCoef = 0.05;
        m_physical.mass = 1.f;
        m_collisionSize = 0.2f;
        m_type = EntityType::player;
        interactible = true;
    }
    void on_update() override
    {

    }
    void on_hit(EntityType otherEntity) override
    {
        if (otherEntity == EntityType::projectile)
        {
            std::string text("Times hit: ");
            text.append(std::to_string(++m_deathCounter)).append("/5");
            rcm::get_gameHandler().set_text_ui(text);
            rcm::get_gameHandler().show_text_ui() = true;
            if (m_deathCounter == 6)
                rcm::get_gameHandler().close_game();
        }
    }
private:
    int m_deathCounter = 0;
};

struct MySpawn : public IEntity
{
    MySpawn(const EntityTransform& transform, int id = 2) : IEntity(id, transform)
    {
        interactible = false;
    }
    void on_update() override
    {
        if(cooldown.is_ready())
        {
            rcm::get_gameHandler().add_entity(new MyEnemy(m_transform));
        }
    }
    void on_hit(EntityType) override {}
private:
    utils::SimpleCooldown cooldown = utils::SimpleCooldown(1000);
};

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