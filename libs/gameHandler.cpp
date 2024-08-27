#include <iostream>
#include "gameHandler.hpp"
#include "dataManager.hpp"
#include "gameGraphics.hpp"
#include "gameCore.hpp"
#include "gameInputs.hpp"

#define WINDOW_NAME "ray cast maze"
#define GENERATION_TIME_STEP_MS 5

class GameHandler : public rcm::IGameHandler
{
public:
	void load_game_data(const std::string&) override;
	void create_assets(const std::vector<Entity>&) override;
	void add_entity(const Entity& entity) override;
	void run_game() override;
private:
	void start();
	void performGameCycle();
	void load_sprites(const std::vector<Entity>&);
	inline char get_entity_cell(const EntityTransform& pos, const GameMap& map);
	inline bool goal_reached(const EntityTransform& pos, const GameMap& map);
	void handle_entities_actions();
	void handle_entities_interactions(std::vector<Entity>&);

	void shoot_projectile(const math::Vect2& position, float direction, float speed);

	std::unique_ptr<DataUtils::GameData> m_gameData;
	std::unique_ptr<GameCore> m_gameCore;
	std::unique_ptr<sf::RenderWindow> m_window;
	std::unique_ptr<GameGraphics> m_gameGraphics;
	std::unique_ptr<InputManager> m_inputManager;
	std::unique_ptr<GameCameraView> m_gameCameraView;
	GameStateVars m_gameState{};
};

class MyProjectile : public Entity
{
public:
	MyProjectile( const EntityTransform& transform, int id = PROJECTILE_ID) : Entity(id, transform)
	{
		type = EntityType::projectile;
		set_size(0.1f);
		m_physical.speed = { 6.f, 0.f };
		m_physical.isGhosted = true;
		m_physical.isAirBorne = true;
	}
};

Entity rcm::create_projectile(const EntityTransform& position)
{
	return MyProjectile(position);
}

rcm::IGameHandler& rcm::get_gameHandler()
{
	if (rcm::g_gameHandler.get() == nullptr)
		rcm::g_gameHandler.reset(new GameHandler);
	return *(rcm::g_gameHandler.get());
}

void GameHandler::load_game_data(const std::string& filePath)
{
	try
	{
		m_gameData = DataUtils::load_game_data(filePath);
	}
	catch (std::exception& e)
	{
		std::string err(e.what());
		err.append("\nError while loading game assets.\n");
		throw std::runtime_error(err);
	}

	m_gameCore = std::make_unique<GameCore>(m_gameData->gameCameraVars, m_gameData->gameMap, m_gameData->playerTrasform);
	m_gameCameraView = std::make_unique<GameCameraView>( GameCameraView{m_gameData->playerTrasform, m_gameData->gameCameraVars, m_gameCore->get_camera_vecs()} );
	m_window = std::make_unique<sf::RenderWindow>(sf::VideoMode(windowVars::g_windowWidth, windowVars::g_windowHeight), WINDOW_NAME);
	m_gameGraphics = std::make_unique<GameGraphics>(*(m_window), m_gameData->graphicsVars);
	m_inputManager = std::make_unique<InputManager>(m_gameData->controlsMulti, *(m_window), m_gameState, m_gameCore->get_playerController());
}

void GameHandler::create_assets(const std::vector<Entity>& entities)
{
	m_gameGraphics->create_assets(m_gameData->gameAssets, m_gameData->gameMap, m_gameData->graphicsVars, m_gameCore->get_ray_info_arr(), m_gameState, *(m_gameCameraView.get()));
	load_sprites(entities);
}

void GameHandler::add_entity(const Entity& entity)
{
	m_gameCore->add_entity(entity);
}

void GameHandler::run_game()
{
	start();

	//game timer
	debug::GameTimer gt;
	gt.reset_timer();

	while (m_gameGraphics->is_running())
	{
		performGameCycle();

		//frame counter
		gt.add_frame();
		if (gt.get_frame_rate_noreset() == 8)
		{
			std::cout << "fps: " << gt.get_frame_rate()  <<  std::endl;
		}
	}
}

void GameHandler::start()
{	
	//step by step map generation (if generation is requested)
	while (m_gameCore->generate_map_step())
	{
		std::thread sleep([] { std::this_thread::sleep_for(std::chrono::milliseconds(GENERATION_TIME_STEP_MS)); });

		m_inputManager->handle_events_close();

		m_gameGraphics->draw_map_gen(m_gameData->gameMap.x, m_gameData->gameMap.y, m_gameData->playerTrasform.coords.x, m_gameData->playerTrasform.coords.y, *(m_gameData->gameMap.cells));

		//wait if the generation time isn't over
		sleep.join();
	}
	m_gameCore->start_internal_time();
	m_gameState.isPaused = true;
}

char GameHandler::get_entity_cell(const EntityTransform& pos, const GameMap& map)
{
	return map.cells->at(	static_cast<int>(pos.coords.y) * map.x +
							static_cast<int>(pos.coords.x));
}

bool GameHandler::goal_reached(const EntityTransform& pos, const GameMap& map)
{
	return (get_entity_cell(pos, map)== 'g');
}

void GameHandler::shoot_projectile(const math::Vect2& position, float direction, float speed)
{
	m_gameCore->add_entity(MyProjectile(EntityTransform{ position, direction }));
	m_gameState.isTriggerPressed = false;
}

void GameHandler::handle_entities_actions()
{
	if (m_gameState.isTriggerPressed)
		shoot_projectile(m_gameCameraView->transform.coords, m_gameCameraView->transform.forewardAngle, 1);
}

void GameHandler::handle_entities_interactions(std::vector<Entity>& entities)
{
	for (Entity& eProjectile : entities)
	{
		if(eProjectile.active && eProjectile.type == EntityType::projectile)
		{
			//destroy projectiles if inside walls
			char cell = get_entity_cell(eProjectile.m_transform, m_gameData->gameMap);
			if (!(cell == ' ' || cell == 'g'))
				eProjectile.active = false;

			//destroy active destructible entities toghether with the projectile itself on hit
			for (Entity& eTarget : entities)
			{
				if (eTarget.active && eTarget.vulnerable)
				{
					float distance = (eProjectile.m_transform.coords - eTarget.m_transform.coords).Length();
					if (eProjectile.m_collisionSize + eTarget.m_collisionSize >= distance)
					{
						eProjectile.active = false;
						eTarget.active = false;
					}
				}
			}
		}
	}
}

void GameHandler::performGameCycle()
{
	m_window->clear(sf::Color::Black);

	m_inputManager->handle_events_main();
	
	m_gameCore->update_entities();

	handle_entities_interactions(m_gameCore->get_entities());
	handle_entities_actions();

	if (m_gameState.isFindPathRequested && m_gameData->gameMap.generated)
	{
		m_gameGraphics->calculate_shortest_path(m_gameData->playerTrasform);
		m_gameState.isFindPathRequested = false;
	}

	m_gameCore->view_by_ray_casting(m_gameState.isLinearPersp);

	m_gameGraphics->draw_view(m_gameState.isLinearPersp, m_gameCore->get_entities());

	if (m_gameState.isPaused || m_gameState.isTabbed)
	{
		m_gameGraphics->draw_map(m_gameData->gameMap.x, m_gameData->gameMap.y, m_gameData->playerTrasform.coords.x, m_gameData->playerTrasform.coords.y, *(m_gameData->gameMap.cells));
		m_gameGraphics->draw_path_out();
	}
	else
	{
		m_gameGraphics->draw_minimap_background(m_gameData->gameMap, m_gameData->playerTrasform, m_gameData->graphicsVars);
		m_gameGraphics->draw_minimap_triangles(m_gameData->gameCameraVars.pixelWidth, m_gameCore->get_ray_info_arr(), m_gameData->graphicsVars);

		if (goal_reached(m_gameData->playerTrasform, m_gameData->gameMap))
		{
			m_gameGraphics->draw_end_screen();
		}
	}
	m_window->display();
}

void GameHandler::load_sprites(const std::vector<Entity>& entities)
{
	for (const std::pair<int, std::string>& sprite : m_gameData->gameSprites)
	{
		m_gameGraphics->load_sprite(sprite.first, sprite.second);
	}
	for (const Entity& e : entities)
	{
		m_gameCore->add_entity(e);
	}
}