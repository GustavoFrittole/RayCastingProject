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
	void load_game_data(const std::string&, std::unique_ptr<IEntity>&) override;
	void create_assets(std::vector<std::unique_ptr<IEntity>>&) override;
	void add_entity(IEntity* entity) override;
	void run_game() override;
	void close_game() override;
	bool& show_text_ui() override { return m_gameState.drawTextUi; }
	void set_text_ui(const std::string&) override;
private:
	void start();
	void performGameCycle();
	void add_cached_entities();
	void load_sprites(std::vector<std::unique_ptr<IEntity>>&);
	inline char get_entity_cell(const EntityTransform& pos, const GameMap& map);
	inline bool goal_reached(const EntityTransform& pos, const GameMap& map);
	void handle_entities_actions(std::vector<std::unique_ptr<IEntity>>&);
	void handle_entities_interactions(std::vector<std::unique_ptr<IEntity>>&);
	void draw_text_ui();

	std::unique_ptr<DataUtils::GameData> m_gameData;
	std::unique_ptr<GameCore> m_gameCore;
	std::unique_ptr<sf::RenderWindow> m_window;
	std::unique_ptr<GameGraphics> m_gameGraphics;
	std::unique_ptr<InputManager> m_inputManager;
	std::unique_ptr<GameCameraView> m_gameCameraView;
	GameStateVars m_gameState{};
	std::vector<std::unique_ptr<IEntity>> m_entitiesToAdd;
};

class MyProjectile : public IEntity
{
public:
	MyProjectile( const EntityTransform& transform, int id = PROJECTILE_ID) : IEntity(id, transform)
	{
		m_type = EntityType::projectile;
		set_size(0.1f);
		m_physical.speed = { 16.f, 0.f };
		m_physical.isGhosted = true;
		interactible = true;
	}
	void on_update() override {};
	void on_hit(EntityType otherEntity) override { destroyed = true; }
};

IEntity* rcm::create_projectile(const EntityTransform& position)
{
	return new MyProjectile(position);
}

rcm::IGameHandler& rcm::get_gameHandler()
{
	if (rcm::g_gameHandler.get() == nullptr)
		rcm::g_gameHandler.reset(new GameHandler);
	return *(rcm::g_gameHandler.get());
}

void GameHandler::close_game()
{
	m_window->close();
}

void GameHandler::set_text_ui(const std::string& text)
{
	m_gameGraphics->set_text_ui(text);
}

void GameHandler::draw_text_ui()
{
	if (m_gameState.drawTextUi)
	{
		m_gameGraphics->draw_text_ui();
	}
}

void GameHandler::load_game_data(const std::string& filePath, std::unique_ptr<IEntity>& player)
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

	m_gameCore = std::make_unique<GameCore>(m_gameData->gameCameraVars, m_gameData->gameMap, player->m_transform);
	m_gameCameraView = std::make_unique<GameCameraView>( GameCameraView{player->m_transform, m_gameData->gameCameraVars, m_gameCore->get_camera_vecs()} );
	m_window = std::make_unique<sf::RenderWindow>(sf::VideoMode(windowVars::g_windowWidth, windowVars::g_windowHeight), WINDOW_NAME);
	m_gameGraphics = std::make_unique<GameGraphics>(*(m_window), m_gameData->graphicsVars);
	m_inputManager = std::make_unique<InputManager>(m_gameData->controlsMulti, *(m_window), m_gameState, m_gameCore->get_playerController());

	m_entitiesToAdd.emplace_back(player.release());
}

void GameHandler::create_assets(std::vector<std::unique_ptr<IEntity>>& entities)
{
	m_gameGraphics->create_assets(m_gameData->gameAssets, m_gameData->gameMap, m_gameData->graphicsVars, m_gameCore->get_ray_info_arr(), m_gameState, *(m_gameCameraView.get()));
	load_sprites(entities);
	std::cout << "pluto:" << std::endl;
}

void GameHandler::add_entity(IEntity* entity)
{
	m_entitiesToAdd.emplace_back(entity);
}

void GameHandler::add_cached_entities()
{
	while(!m_entitiesToAdd.empty())
	{
		m_gameCore->add_entity(m_entitiesToAdd.back().release());
		m_entitiesToAdd.pop_back();
	}
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

		m_gameGraphics->draw_map_gen(m_gameData->gameMap.x, m_gameData->gameMap.y, m_gameCameraView->transform.coords.x, m_gameCameraView->transform.coords.y, *(m_gameData->gameMap.cells));

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

void GameHandler::handle_entities_actions(std::vector<std::unique_ptr<IEntity>>& entities)
{
	if (m_gameState.isTriggerPressed)
	{
		EntityTransform projectileTransform = { m_gameCameraView->transform.coords + math::rad_to_vec(m_gameCameraView->transform.forewardAngle) * (0.5f), m_gameCameraView->transform.forewardAngle };
		rcm::get_gameHandler().add_entity(rcm::create_projectile(projectileTransform));
		m_gameState.isTriggerPressed = false;
	}

	for (std::unique_ptr<IEntity>& entity : entities)
	{
		if(entity->active)
			entity->on_update();
	}
}

void GameHandler::handle_entities_interactions(std::vector<std::unique_ptr<IEntity>>& entities)
{

	for (int i = 0; i < entities.size()-1; ++i)
	{
		if(entities.at(i)->interactible)
		{
			//check if inside walls
			HitType cell = HitType::Nothing;
			m_gameCore->chech_position_in_map(entities.at(i)->m_transform.coords, cell);

			if (cell != HitType::Nothing)
				entities.at(i)->on_hit(EntityType::wall);

			//entity - entity interaction
			for (int c = i+1; c < entities.size(); ++c)
			{
				if(entities.at(c)->interactible)
				{
					float distance = (entities.at(i)->m_transform.coords - entities.at(c)->m_transform.coords).Length();
					if (distance < entities.at(i)->m_collisionSize + entities.at(c)->m_collisionSize)
					{
						entities.at(i)->on_hit(entities.at(c)->m_type);
						entities.at(c)->on_hit(entities.at(i)->m_type);
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

	add_cached_entities();
	m_gameCore->update_entities();
	handle_entities_interactions(m_gameCore->get_entities());
	handle_entities_actions(m_gameCore->get_entities());

	m_gameCore->remove_destroyed_entities();

	if (m_gameState.isFindPathRequested && m_gameData->gameMap.generated)
	{
		m_gameGraphics->calculate_shortest_path(m_gameCameraView->transform);
		m_gameState.isFindPathRequested = false;
	}

	m_gameCore->view_by_ray_casting(m_gameState.isLinearPersp);

	m_gameGraphics->draw_view(m_gameState.isLinearPersp, m_gameCore->get_entities());

	if (m_gameState.isPaused || m_gameState.isTabbed)
	{
		m_gameGraphics->draw_map(m_gameData->gameMap.x, m_gameData->gameMap.y, m_gameCameraView->transform.coords.x, m_gameCameraView->transform.coords.y, *(m_gameData->gameMap.cells));
		m_gameGraphics->draw_path_out();
	}
	else
	{
		m_gameGraphics->draw_minimap_background(m_gameData->gameMap, m_gameCameraView->transform, m_gameData->graphicsVars);
		m_gameGraphics->draw_minimap_triangles(m_gameData->gameCameraVars.pixelWidth, m_gameCore->get_ray_info_arr(), m_gameData->graphicsVars);

		if (goal_reached(m_gameCameraView->transform, m_gameData->gameMap) && !m_gameState.drawTextUi)
		{
			set_text_ui("Goal Reached");
			show_text_ui() = true;
		}

		draw_text_ui();
	}
	m_window->display();
}

void GameHandler::load_sprites(std::vector<std::unique_ptr<IEntity>>& entities)
{
	for (const std::pair<int, std::string>& sprite : m_gameData->gameSprites)
	{
		std::cout << "pippo:" << m_gameData->gameSprites.size() << std::endl;
		m_gameGraphics->load_sprite(sprite.first, sprite.second);
	}
	for (std::unique_ptr<IEntity>& e : entities)
	{
		m_gameCore->add_entity(e.release());
	}
}