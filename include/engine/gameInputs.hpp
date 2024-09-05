#ifndef GAMEINPUTS_HPP
#define GAMEINPUTS_HPP

#include "gameDataStructures.hpp"
#include <SFML/Graphics.hpp>
#include <thread>

class InputManager
{
public:
    InputManager(rcm::ControlsSensitivity& controlsSens, sf::RenderWindow& window, rcm::GameStateVars& gameState);

    const rcm::InputCache& get_input_cache() const { return m_inputCache; }
    void handle_events_close();
    void handle_events_main();
private:
    rcm::InputCache m_inputCache;
    rcm::ControlsSensitivity& m_controlsSens;
    sf::RenderWindow& m_window;
    rcm::GameStateVars& m_gameState;
};

#endif