#include "gameInputs.hpp"

using namespace windowVars;

InputManager::InputManager(ControlsVars& controlsMulti, sf::RenderWindow& window, GameStateVars& gameState, game::IGameController& playerController) :
    m_controlsMulti(controlsMulti),
    m_window(window),
    m_gameState(gameState),
    m_playerController(playerController) {}

void InputManager::handle_events_close()
{
    sf::Event event;
    while (m_window.pollEvent(event))
    {
        if (event.type == sf::Event::Closed)
            m_window.close();
    }
}

void InputManager::handle_events_main()
{
    bool justUnpaused = false;
    sf::Event event;
    while (m_window.pollEvent(event))
    {
        switch (event.type)
        {
        case sf::Event::Closed:
            m_window.close();
            break;
        case sf::Event::KeyPressed:
            if (event.key.scancode == sf::Keyboard::Scan::Escape)
            {
                if (m_gameState.isPaused)
                {
                    m_gameState.isPaused = false;
                    justUnpaused = true;
                }
                else
                    m_gameState.isPaused = true;
            }
            if (event.key.scancode == sf::Keyboard::Scan::Space)
            {
                if (m_gameState.isLinearPersp)
                {
                    m_gameState.isLinearPersp = false;
                }
                else
                    m_gameState.isLinearPersp = true;
            }
            if (event.key.scancode == sf::Keyboard::Scan::R)
            {
                if (m_gameState.drawSky)
                {
                    m_gameState.drawSky = false;
                }
                else
                    m_gameState.drawSky = true;
            }
            if (event.key.scancode == sf::Keyboard::Scan::E)
            {
                m_gameState.isFindPathRequested = true;
            }
        }
    }
    if (m_window.hasFocus())
    {
        if (!m_gameState.isPaused)
        {
            if (!m_gameState.hadFocus || justUnpaused)
            {
                m_gameState.hadFocus = true;
                justUnpaused = false;
                m_window.setMouseCursorVisible(false);
                m_window.setMouseCursorGrabbed(true);
                sf::Mouse::setPosition(sf::Vector2i(g_windowWidth / 2, 0), m_window);
            }
            else
            {
                m_playerController.rotate((g_windowWidth / 2 - sf::Mouse::getPosition(m_window).x) * 0.1f);
                sf::Mouse::setPosition(sf::Vector2i(g_windowWidth / 2, 0), m_window);
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
            {
                m_playerController.move_foreward(+m_controlsMulti.movementSpeed);
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
            {
                m_playerController.move_strafe(-m_controlsMulti.movementSpeed);
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
            {
                m_playerController.move_strafe(+m_controlsMulti.movementSpeed);
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
            {
                m_playerController.move_foreward(-m_controlsMulti.movementSpeed);
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
            {
                m_playerController.rotate(+m_controlsMulti.mouseSens);
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
            {
                m_playerController.rotate(-m_controlsMulti.mouseSens);
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Tab))
            {
                m_gameState.isTabbed = true;
            }
            else
            {
                m_gameState.isTabbed = false;
            }
            if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
            {
                if (!m_isTriggerKeptPressed)
                {
                    m_isTriggerKeptPressed = true;
                    m_gameState.isTriggerPressed = true;
                }
            }
            else
            {
                m_isTriggerKeptPressed = false;
            }
            if (sf::Mouse::isButtonPressed(sf::Mouse::Right))
            {
                m_gameState.isTriggerPressed = true;
            }
        }
        else
        {
            m_window.setMouseCursorVisible(true);
            m_window.setMouseCursorGrabbed(false);
        }
    }
    else
    {
        if (m_gameState.hadFocus)
            m_gameState.hadFocus = false;
    }
}