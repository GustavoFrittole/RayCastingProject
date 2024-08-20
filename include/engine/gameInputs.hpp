#ifndef GAMEINPUTS_HPP
#define GAMEINPUTS_HPP

#include "gameDataStructures.hpp"
#include <SFML/Graphics.hpp>

class InputManager
{
public:
    InputManager(ControlsVars& controlsMulti, sf::RenderWindow& window, GameStateVars& gameState, game::IGameController& playerController);

    ControlsVars& m_controlsMulti;
    sf::RenderWindow& m_window;
    GameStateVars& m_gameState;
    game::IGameController& m_playerController;

    void handle_events_close();
    void handle_events_main();
};

#endif