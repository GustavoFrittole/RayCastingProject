#include <random>

//----------------------------- Utils --------------------------------

using namespace rcm;

math::Vect2 find_free_cell();

//-------------------- Custom Entities Examples ----------------------

//------------------------- MyGameLogicsHandler ------------------------

class MyGameLogicsHandler : public IEntity
{
public:
    MyGameLogicsHandler(const EntityTransform& transform = { {0,0}, 0 }, int id = -1) : IEntity(id, transform)
    {}
    void on_create() override { m_gameHandler = &(rcm::get_gameHandler()); }
    void on_update() override {}
    void on_late_update() override;
    void on_hit(EntityType otherEntity) override {}

    //------------------------- MyProjectile -----------------------------

    class MyProjectile : public IEntity
    {
    public:
        MyProjectile(const EntityTransform& transform, int id = PROJECTILE_ID);
        void on_create() override {}
        void on_update() override {}
        void on_late_update() override {}
        void on_hit(EntityType otherEntity) override { m_destroyed = true; }
    };

    //------------------------- MyPlayer ---------------------------------

    struct MyPlayer : public IEntity
    {
        MyPlayer(const EntityTransform& transform, int id = -1);
        void on_create() override { m_gameHandler = &(rcm::get_gameHandler()); }
        void on_update() override;
        void on_late_update() override {}
        void on_hit(EntityType otherEntity) override;

        int get_playerHearts() { return m_playerHearts; }
        void set_playerHearths(int hearts) { m_playerHearts = hearts; }
    private:
        bool m_isTriggerPressed = false;
        bool m_isTriggerKeptPressed = false;
        utils::SimpleCooldown m_burstCooldown = utils::SimpleCooldown(50);
        rcm::IGameHandler* m_gameHandler = nullptr;
        int m_playerHearts = 0;
    };

    //------------------------- MyFallenEnemy ---------------------------

    struct MyFallenEnemy : public IEntity
    {
        MyFallenEnemy(const EntityTransform& transform, int id = 2);
        void on_create() override { m_gameHandler = &(rcm::get_gameHandler()); }
        void on_update() override;
        void on_late_update() override {}
        void on_hit(EntityType otherEntity) override {}
    private:
        utils::SimpleCooldown m_cooldown = utils::SimpleCooldown(2000);
        rcm::IGameHandler* m_gameHandler = nullptr;
    };

    //------------------------- MyEnemy ------------------------

    struct MyEnemy : public IEntity
    {
        MyEnemy(const EntityTransform& transform, int id = 1);
        void on_create() override { m_gameHandler = &(rcm::get_gameHandler()); ++m_enemyInstances; }
        void on_late_update() override {}
        void on_update() override;
        void on_hit(EntityType otherEntity) override;
    private:
        utils::SimpleCooldown m_heartCooldown = utils::SimpleCooldown(300);
        utils::SimpleCooldown m_turningCooldown = utils::SimpleCooldown(2000);
        rcm::IGameHandler* m_gameHandler = nullptr;
        std::mt19937 m_generator;
        std::uniform_real_distribution<float> m_distribution;
    };

    //------------------------- MySpawner ------------------------

    struct MySpawner : public IEntity
    {
        MySpawner(const EntityTransform& transform, int frame_number, const unsigned char* ids);
        void on_create() override;
        void on_update() override;
        void on_late_update() override {}
        void on_hit(EntityType) override {}
    private:
        const unsigned char* m_ids = nullptr;
        int m_countdown = 0;
        const int m_frameNumber = 0;
        utils::SimpleCooldown cooldown = utils::SimpleCooldown(500);
        rcm::IGameHandler* m_gameHandler = nullptr;
    };

    void set_player(MyPlayer* player) { m_player = player; }
private:
    utils::SimpleCooldown m_deathTimer = utils::SimpleCooldown(3000);
    IGameHandler* m_gameHandler = nullptr;
    MyPlayer* m_player = nullptr;
    inline static int m_enemyInstances = 0;
    int m_heartsCap = 10;
    bool m_isDying = false;
};

//------------------------ Implementation -----------------------

//------------------------- MyProjectile ------------------------

MyGameLogicsHandler::MyProjectile::MyProjectile(const EntityTransform& transform, int id) : IEntity(id, transform)
{
    m_type = EntityType::projectile;
    set_size(0.2f);
    m_physical.movementSpeed = { 16.f, 0.f };
    m_physical.movFrictionCoef = .2;
    m_physical.isGhosted = true;
    m_interactible = true;
}

//------------------------- MyPlayer ------------------------

MyGameLogicsHandler::MyPlayer::MyPlayer(const EntityTransform& transform, int id) : IEntity(id, transform)
{
    m_physical.movFrictionCoef = 5.;
    m_physical.mass = 1.f;
    m_collisionSize = .2f;
    m_type = EntityType::player;
    m_interactible = true;
}

void MyGameLogicsHandler::MyPlayer::on_update()
{
    if (m_gameHandler->get_input_cache().foreward != 0 || m_gameHandler->get_input_cache().lateral != 0)
        m_physical.movementSpeed = { m_gameHandler->get_input_cache().foreward , m_gameHandler->get_input_cache().lateral };

    m_physical.rotationSpeed = m_gameHandler->get_input_cache().rotatation;

    if (m_gameHandler->get_input_cache().leftTrigger)
    {
        if (!m_isTriggerKeptPressed)
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
        EntityTransform projectileTransform = { m_transform.coordinates + math::rad_to_vec(m_transform.forewardAngle) * (0.5f), m_transform.forewardAngle };
        m_gameHandler->add_entity(new MyProjectile(projectileTransform));
        m_isTriggerPressed = false;
    }
}

void MyGameLogicsHandler::MyPlayer::on_hit(EntityType otherEntity)
{
    if (otherEntity == EntityType::projectile)
    {
        ++m_playerHearts;
    }
}

//------------------------- MyFallenEnemy ------------------------

MyGameLogicsHandler::MyFallenEnemy::MyFallenEnemy(const EntityTransform& transform, int id) : IEntity(id, transform)
{
    m_billboard.size = 0.3f;
    m_billboard.alignment = SpriteAlignment::Floor;
    m_type = EntityType::prop;
    m_interactible = false;
}

void MyGameLogicsHandler::MyFallenEnemy::on_update()
{
    if (m_cooldown.is_ready())
    {
        m_gameHandler->add_entity(new MyEnemy(m_transform));
        m_destroyed = true;
    }
}

//------------------------- MyEnemy ------------------------

MyGameLogicsHandler::MyEnemy::MyEnemy(const EntityTransform& transform, int id) : IEntity(id, transform)
{
    m_physical.movementSpeed.x = 2.5f;
    m_physical.rotationSpeed = 0.8f;
    m_physical.rotFrictionCoef = .3;
    m_physical.mass = 1.f;
    m_collisionSize = 0.2f;
    m_type = EntityType::enemy;
    m_billboard.alignment = SpriteAlignment::Floor;
    m_billboard.size = 0.8f;
    m_interactible = true;

    std::random_device seed;
    m_generator = std::mt19937(seed());
    m_distribution = (std::uniform_real_distribution<float>(-2.5f, 2.5f) );
}

void MyGameLogicsHandler::MyEnemy::on_update()
{
    if (m_heartCooldown.is_ready())
    {
        EntityTransform projectileTransform = { m_transform.coordinates + math::rad_to_vec(m_transform.forewardAngle) * (m_collisionSize + 0.2f), m_transform.forewardAngle };
        m_gameHandler->add_entity(new MyProjectile(projectileTransform));
    }

    if (m_turningCooldown.is_ready())
    {
        m_physical.rotationSpeed = (m_distribution(m_generator));
    }
}

void MyGameLogicsHandler::MyEnemy::on_hit(EntityType otherEntity)
{
    if (otherEntity == EntityType::projectile)
    {
        m_gameHandler->add_entity(new MyFallenEnemy(m_transform));
        --m_enemyInstances;
        m_destroyed = true;
    }
}

//------------------------- MySpawner ------------------------

MyGameLogicsHandler::MySpawner::MySpawner(const EntityTransform& transform, int frame_number, const unsigned char* ids) : IEntity(ids[0], transform),
    m_frameNumber(frame_number),
    m_ids(ids)
{ 
    m_interactible = false; 
    m_physical.rotationSpeed = 2.f;
}

void MyGameLogicsHandler::MySpawner::on_update()
{
    if (cooldown.is_ready())
    {
        if (m_countdown >= m_frameNumber)
        {
            m_gameHandler->add_entity(new MyEnemy(m_transform));
            m_countdown = 0;
        }
        m_billboard.id = m_ids[m_countdown];
        ++m_countdown;
    }
}

void MyGameLogicsHandler::MySpawner::on_create()
{
    m_gameHandler = &(rcm::get_gameHandler());
    
    //if the map is generated finds a free spot there to place itself
    if (m_gameHandler->get_active_map().generated)
        m_transform.coordinates = find_free_cell();
}

math::Vect2 find_free_cell()
{
    IGameHandler& gh = get_gameHandler();
    const GameMap& map = gh.get_active_map();
    std::random_device seed;
    std::mt19937 generator(seed());
    std::uniform_int_distribution<int> xDist(0, map.width - 1);
    std::uniform_int_distribution<int> yDist(0, map.height - 1);

    int cellX = xDist(generator);
    int cellY = yDist(generator);
    //assuming the map has a substantial amount of free cells
    while (gh.get_entity_cell(cellX, cellY, map) != ' ')
    {
        cellX = xDist(generator);
        cellY = yDist(generator);
    }
    return math::Vect2{ static_cast<float>(cellX), static_cast<float>(cellY) } + math::Vect2{ 0.5f, 0.5f };
}

//------------------------- MyGameLogicsHandler ------------------------

void MyGameLogicsHandler::on_late_update()
{
    m_gameHandler->show_text_ui() = true;

    if (m_isDying)
    {
        m_gameHandler->set_text_ui(std::to_string(m_deathTimer.get_time() / 10000000), TextVerticalAlignment::Center, TextHorizontalAlignment::Center, 70);
        if (m_deathTimer.is_ready())
            m_gameHandler->close_game();
    }
    else if (m_player->get_playerHearts() > m_heartsCap)
    {
        m_isDying = true;
        m_deathTimer.reset();
    }
    else
    {
        if ('g' == m_gameHandler->get_entity_cell(m_player->m_transform, m_gameHandler->get_active_map()))
        {
            m_gameHandler->set_text_ui("Safe zone", TextVerticalAlignment::Center, TextHorizontalAlignment::Center, 50);
            m_player->set_playerHearths(0);
        }
        else
        {
            std::string text("<3 = ");
            text.append(std::to_string(m_player->get_playerHearts())).append("/").append(std::to_string(m_heartsCap).append("   </3 = ").append(std::to_string(m_enemyInstances)));
            m_gameHandler->set_text_ui(text, TextVerticalAlignment::TopWindow, TextHorizontalAlignment::Left, 50, 30, 10);
        }
    }
}