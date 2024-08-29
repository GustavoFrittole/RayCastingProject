
#include <iostream>
#include <memory>
#include <map>
#include <thread>
#include <chrono>
#include "gameHandler.hpp"

using namespace rcm;

//------------------------- Custom Entities Examples ------------------------

class MyProjectile : public IEntity
{
public:
    MyProjectile(const EntityTransform& transform, int id = PROJECTILE_ID) : IEntity(id, transform)
    {
        m_type = EntityType::projectile;
        set_size(0.1f);
        m_physical.speed = { 16.f, 0.f };
        m_physical.isGhosted = true;
        interactible = true;
    }
    void on_create() override {};
    void on_update() override {};
    void on_hit(EntityType otherEntity) override { destroyed = true; }
};

struct MyEnemy : public IEntity
{
    MyEnemy(const EntityTransform& transform, int id = 1) : IEntity(id, transform)
    {
        m_physical.speed.x = 2.5f;
        m_physical.rotationSpeed = 0.8f;
        m_physical.movFrictionCoef = .5;
        m_physical.mass = 1.f;
        m_collisionSize = 0.2f;
        m_type = EntityType::enemy;
        interactible = true;
    }
    void on_create() override 
    {
        m_gameHandler = &(rcm::get_gameHandler());
    };
    void on_update() override
    {
        if (m_cooldown.is_ready())
        {
            EntityTransform projectileTransform = { m_transform.coords + math::rad_to_vec(m_transform.forewardAngle) * (m_collisionSize + 0.2f), m_transform.forewardAngle };
            m_gameHandler->add_entity(new MyProjectile(projectileTransform));
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
    utils::SimpleCooldown m_cooldown = utils::SimpleCooldown(500);
    rcm::IGameHandler* m_gameHandler = nullptr;
};

struct MyPlayer : public IEntity
{
    MyPlayer(const EntityTransform& transform, int id = -1) : IEntity(id, transform)
    {
        m_physical.speed = { 1,0 };
        m_physical.movFrictionCoef = 5.;
        m_physical.mass = 1.f;
        m_collisionSize = 0.2f;
        m_type = EntityType::player;
        interactible = true;
    }
    void on_create() override
    {
        m_gameHandler = &(rcm::get_gameHandler());
    };
    void on_update() override
    {
        if (m_gameHandler->get_input_cache().foreward != 0 || m_gameHandler->get_input_cache().lateral != 0)
            m_physical.speed = { m_gameHandler->get_input_cache().foreward , m_gameHandler->get_input_cache().lateral };

        m_physical.rotationSpeed = m_gameHandler->get_input_cache().rotatation;

        if (m_gameHandler->get_input_cache().leftTrigger)
        {
            if(!m_isTriggerKeptPressed)
            {
                m_isTriggerKeptPressed = true;
                m_isTriggerPressed = true;
            }
        }
        else
        {
            if (m_gameHandler->get_input_cache().rightTrigger && m_burstCooldown.is_ready())
            {
                m_isTriggerPressed = true;
            }
            else
            {
                m_isTriggerPressed = false;
                m_isTriggerKeptPressed = false;
            }
        }

        if (m_isTriggerPressed)
        {
        	EntityTransform projectileTransform = { m_transform.coords + math::rad_to_vec(m_transform.forewardAngle) * (0.5f), m_transform.forewardAngle };
            m_gameHandler->add_entity(new MyProjectile(projectileTransform));
        	m_isTriggerPressed = false;
        }
    }
    void on_hit(EntityType otherEntity) override
    {
        if (otherEntity == EntityType::projectile)
        {
            std::string text("Times hit: ");
            text.append(std::to_string(++m_deathCounter)).append("/5");
            m_gameHandler->set_text_ui(text);
            m_gameHandler->show_text_ui() = true;
            if (m_deathCounter == 6)
                m_gameHandler->close_game();
        }
    }
private:
    int m_deathCounter = 0;
    bool m_isTriggerPressed = false;
    bool m_isTriggerKeptPressed = false;
    utils::SimpleCooldown m_burstCooldown = utils::SimpleCooldown(50);
    rcm::IGameHandler* m_gameHandler = nullptr;
};

struct MySpawn : public IEntity
{
    MySpawn(const EntityTransform& transform, int id = 2) : IEntity(id, transform)
    {
        interactible = false;
    }
    void on_create() override
    {
        m_gameHandler = &(rcm::get_gameHandler());
    };
    void on_update() override
    {
        if(cooldown.is_ready())
        {
            m_gameHandler->add_entity(new MyEnemy(m_transform));
        }
    }
    void on_hit(EntityType) override {}
private:
    utils::SimpleCooldown cooldown = utils::SimpleCooldown(1000);
    rcm::IGameHandler* m_gameHandler = nullptr;
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