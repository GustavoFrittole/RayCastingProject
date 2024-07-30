#include"gameHandler.hpp"
#include"gameGraphics.hpp"
#include <iostream>


constexpr auto WINDOW_NAME = "ray cast maze";

class GameHandler : public rcm::IGameHandler
{
public:
	GameHandler(const std::string& configPath);
	void run_game() override;
private:
	std::unique_ptr<GameGraphics> m_gameGraphics;
	std::unique_ptr<DataUtils::GameData> m_gameData;
	bool load_game_data(const std::string&) override;
	bool m_isGood = true;
	std::string m_errors;
};

GameHandler::GameHandler(const std::string& configPath) 
{
	m_isGood = load_game_data(configPath);
}

bool GameHandler::load_game_data(const std::string& filePath)
{
	try
	{
		m_gameData = DataUtils::load_game_data(filePath);
	}
	catch (std::exception& e)
	{
		m_errors.append(e.what());
		return false;
	}

	m_gameGraphics = std::make_unique<GameGraphics>(m_gameData, "pippo");
	return true;
}
void GameHandler::run_game()
{
	m_gameGraphics->start();

	debug::GameTimer gt;

	while (m_gameGraphics->is_running())
	{
		m_gameGraphics->performGameCycle();
		gt.add_frame();
		if (gt.get_frame_count() > 20)
		{
			std::cout << gt.get_frame_rate() << std::endl;
		}
	}
}

rcm::IGameHandler* rcm::create_gameHandler(const std::string& configPath)
{
	return new GameHandler(configPath);
}
