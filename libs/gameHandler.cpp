#include <iostream>
#include "gameHandler.hpp"
#include "dataManager.hpp"
#include "gameGraphics.hpp"
#include "gameCore.hpp"
#include "gameInputs.hpp"

#define WINDOW_NAME "ray cast maze"
#define GENERATION_TIME_STEP_MS 5

namespace rcm
{
	class GameHandler : public IGameHandler
	{
	public:
		void load_game_data(const std::string&, std::unique_ptr<IEntity>&) override;
		void create_assets(std::vector<std::unique_ptr<IEntity>>&) override;
		void add_entity(IEntity* entity) override;
		void run_game() override;
		void close_game() override;
		bool& show_text_ui() override { return m_gameState.drawTextUi; }
		void set_text_ui(const std::string&, const TextVerticalAlignment, const TextHorizontalAlignment, const int, const int, const int) override;
		const InputCache& get_input_cache() override { return m_inputManager->get_input_cache(); }
		inline char get_entity_cell(const EntityTransform& pos, const GameMap& map) override;
		inline char get_entity_cell(const int cellX, const int cellY, const GameMap& map) override;
		inline const GameMap& get_active_map() override { return m_gameData->gameMap; }
	private:
		void start();
		void performGameCycle();
		void add_cached_entities();
		void load_sprites(std::vector<std::unique_ptr<IEntity>>&);
		void handle_entities_actions(std::vector<std::unique_ptr<IEntity>>&);
		void handle_entities_interactions(std::vector<std::unique_ptr<IEntity>>&);
		void draw_text_ui();
		inline bool goal_reached(const EntityTransform& pos, const GameMap& map);

		std::unique_ptr<DataUtils::GameData> m_gameData;
		std::unique_ptr<GameCore> m_gameCore;
		std::unique_ptr<sf::RenderWindow> m_window;
		std::unique_ptr<GameGraphics> m_gameGraphics;
		std::unique_ptr<InputManager> m_inputManager;
		std::unique_ptr<GameCameraView> m_gameCameraView;
		GameStateVars m_gameState{};
		std::vector<std::unique_ptr<IEntity>> m_entitiesToAdd;
	};

	IGameHandler& get_gameHandler()
	{
		if (g_gameHandler.get() == nullptr)
			g_gameHandler.reset(new GameHandler);
		return *(g_gameHandler.get());
	}

	void GameHandler::close_game()
	{
		m_window->close();
	}

	void GameHandler::set_text_ui(const std::string& text, const TextVerticalAlignment vertAlign, const TextHorizontalAlignment horiAlign, const int size, const int offsetX, const int offsetY)
	{
		m_gameGraphics->set_text_ui(text, vertAlign, horiAlign, size, offsetX, offsetY);
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
			std::string err("Error while loading game assets:\n");
			err.append(e.what());
			throw std::runtime_error(err);
		}

		m_gameCore = std::make_unique<GameCore>(m_gameData->gameCameraVars, m_gameData->gameMap, player->m_transform);
		m_gameCameraView = std::make_unique<GameCameraView>(GameCameraView{ player->m_transform, m_gameData->gameCameraVars, m_gameCore->get_camera_vecs() });
		m_window = std::make_unique<sf::RenderWindow>(sf::VideoMode(windowVars::g_windowWidth, windowVars::g_windowHeight), WINDOW_NAME);
		m_gameGraphics = std::make_unique<GameGraphics>(*(m_window), m_gameData->graphicsVars);
		m_inputManager = std::make_unique<InputManager>(m_gameData->controlsMulti, *(m_window), m_gameState);

		m_entitiesToAdd.emplace_back(player.release());
	}

	void GameHandler::create_assets(std::vector<std::unique_ptr<IEntity>>& entities)
	{
		m_gameGraphics->create_assets(m_gameData->gameAssets, m_gameData->gameMap, m_gameData->graphicsVars, m_gameCore->get_ray_info_arr(), m_gameState, *(m_gameCameraView.get()));
		load_sprites(entities);
	}

	void GameHandler::add_entity(IEntity* entity)
	{
		m_entitiesToAdd.emplace_back(entity);
	}

	void GameHandler::add_cached_entities()
	{
		while (!m_entitiesToAdd.empty())
		{
			m_entitiesToAdd.back()->on_create();
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
				std::cout << "fps: " << gt.get_frame_rate() << std::endl;
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

			m_gameGraphics->draw_map_gen(m_gameData->gameMap.width, m_gameData->gameMap.height, m_gameCameraView->transform.coordinates.x, m_gameCameraView->transform.coordinates.y, *(m_gameData->gameMap.cells));

			//wait if the generation time isn't over
			sleep.join();
		}
		m_gameCore->start_internal_time();
		m_gameState.isPaused = true;
		add_cached_entities();
	}

	char GameHandler::get_entity_cell(const EntityTransform& pos, const GameMap& map)
	{
		return map.cells->at(static_cast<int>(pos.coordinates.y) * map.width +
			static_cast<int>(pos.coordinates.x));
	}

	char GameHandler::get_entity_cell(const int cellX, const int cellY, const GameMap& map)
	{
		return map.cells->at(cellX * map.width + cellY);
	}

	bool GameHandler::goal_reached(const EntityTransform& pos, const GameMap& map)
	{
		return (get_entity_cell(pos, map) == 'g');
	}

	void GameHandler::handle_entities_actions(std::vector<std::unique_ptr<IEntity>>& entities)
	{
		for (std::unique_ptr<IEntity>& entity : entities)
		{
			if (entity->get_active())
				entity->on_update();
		}
		for (std::unique_ptr<IEntity>& entity : entities)
		{
			if (entity->get_active())
				entity->on_late_update();
		}
	}

	void GameHandler::handle_entities_interactions(std::vector<std::unique_ptr<IEntity>>& entities)
	{

		for (int i = 0; i < entities.size() - 1; ++i)
		{
			if (entities.at(i)->get_interactible())
			{
				//check if inside walls
				HitType cell = HitType::Nothing;
				m_gameCore->chech_position_in_map(entities.at(i)->m_transform.coordinates, cell);

				if (cell != HitType::Nothing)
					entities.at(i)->on_hit(EntityType::wall);

				//entity - entity interaction
				for (int c = i + 1; c < entities.size(); ++c)
				{
					if (entities.at(c)->get_interactible())
					{
						float distance = (entities.at(i)->m_transform.coordinates - entities.at(c)->m_transform.coordinates).Length();
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

		handle_entities_interactions(m_gameCore->get_entities());
		handle_entities_actions(m_gameCore->get_entities());

		m_gameCore->remove_destroyed_entities();
		add_cached_entities();

		if (m_gameState.isFindPathRequested && m_gameData->gameMap.generated)
		{
			m_gameGraphics->calculate_shortest_path(m_gameCameraView->transform);
			m_gameState.isFindPathRequested = false;
		}

		m_gameCore->view_by_ray_casting(m_gameState.isLinearPersp);
		m_gameGraphics->draw_view(m_gameState.isLinearPersp, m_gameCore->get_entities());

		if (m_gameState.isPaused || m_gameState.isTabbed)
		{
			m_gameGraphics->draw_map(m_gameData->gameMap.width, m_gameData->gameMap.height, m_gameCameraView->transform.coordinates.x, m_gameCameraView->transform.coordinates.y, *(m_gameData->gameMap.cells));
			m_gameGraphics->draw_path_out();
		}
		else
		{
			m_gameGraphics->draw_minimap_background(m_gameData->gameMap, m_gameCameraView->transform, m_gameData->graphicsVars);
			m_gameGraphics->draw_minimap_triangles(m_gameData->gameCameraVars.pixelWidth, m_gameCore->get_ray_info_arr(), m_gameData->graphicsVars);

			draw_text_ui();
		}
		m_window->display();

		m_gameCore->update_entities();
	}

	void GameHandler::load_sprites(std::vector<std::unique_ptr<IEntity>>& entities)
	{
		for (const std::pair<int, std::string>& sprite : m_gameData->gameSprites)
		{
			m_gameGraphics->load_sprite(sprite.first, sprite.second);
		}
		for (std::unique_ptr<IEntity>& e : entities)
		{
			m_entitiesToAdd.emplace_back(e.release());
		}
		entities.clear();
	}
}