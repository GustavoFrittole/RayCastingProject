#include"gameHandler.hpp"
#include<iostream>
#include"dataManager.hpp"
#include"gameGraphics.hpp"
#include"gameInputs.hpp"

constexpr auto WINDOW_NAME = "ray cast maze";

class GameHandler : public rcm::IGameHandler
{
public:
	void load_game_data(const std::string&) override;
	void create_assets();
	void run_game() override;
private:
	void start();
	void performGameCycle();

	std::unique_ptr<GameCore> m_gameCore;
	std::unique_ptr<sf::RenderWindow> m_window;
	std::unique_ptr<GameGraphics> m_gameGraphics;
	std::unique_ptr<InputManager> m_inputManager;
	std::unique_ptr<DataUtils::GameData> m_gameData;

	GameStateVars m_gameState{};
};

rcm::IGameHandler* rcm::create_gameHandler(const std::string& configPath)
{
	return new GameHandler(configPath);
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

	m_gameCore = std::make_unique<GameCore>(m_gameData->gameCamera, m_gameData->gameMap, m_gameData->playerTrasform);
	m_window = std::make_unique<sf::RenderWindow>(sf::VideoMode(screenStats::g_screenWidth, screenStats::g_screenHeight), WINDOW_NAME);
	m_gameGraphics = std::make_unique<GameGraphics>(m_window);
	m_inputManager = std::make_unique<InputManager>(m_gameData->controlsMulti, m_window, m_gameState, m_gameCore->get_playerController());
}

void GameHandler::create_assets() 
{
	m_gameGraphics->create_assets(m_gameData->gameAssets, m_gameData->gameMap);
}

void GameHandler::run_game()
{
	try
	{
		create_assets();
	}
	catch(std::exception& e)
	{
		std::string err(e.what());
		err.append("\nError while creating game assets.\n");
		throw std::runtime_error(err);
	}

	start();

	//game timer
	debug::GameTimer gt;

	while (m_gameGraphics->is_running())
	{
		performGameCycle();

		//frame counter
		gt.add_frame();
		if (gt.get_frame_count() > 20)
		{
			std::cout << gt.get_frame_rate() << std::endl;
		}
		//
	}
}

void GameHandler::start()
{	
	//step by step map generation (if generation is requested)
	while (m_gameCore->generate_map_step())
	{
		std::thread sleep([] { std::this_thread::sleep_for(std::chrono::milliseconds(GENERATION_TIME_STEP_MS)); });

		m_inputManager->handle_events_close();

		m_gameGraphics->draw_map_gen();

		//wait if the generation time isn't over
		sleep.join();
	}
	m_gameCore->start_internal_time();
	m_gameState.isPaused = true;
}

void GameHandler::performGameCycle()
{
	m_window->clear(sf::Color::Black);

	m_inputManager->handle_events_main();

	m_gameCore->update_entities();

	if (m_gameState.isFindPathRequested && m_gameData->gameMap.generated)
	{
		m_gameGraphics->calculate_shortest_path(m_gameData->playerTrasform);
		m_gameState.isFindPathRequested = false;
	}

	m_gameCore->view_by_ray_casting(m_gameState.isLinearPersp);

	m_gameGraphics->draw_view();
	m_gameGraphics->draw_sprites();

	if (m_gameState.isPaused || m_gameState.isTabbed)
	{
		m_gameGraphics->draw_map();
		m_gameGraphics->draw_path_out();
	}
	else
	{
		m_gameGraphics->draw_minimap_background();
		m_gameGraphics->draw_minimap_triangles();

		if (m_gameGraphics->goal_reached(m_gameData->playerTrasform, m_gameData->gameMap))
		{
			m_gameGraphics->draw_end_screen();
		}
	}
	m_window->display();
}
