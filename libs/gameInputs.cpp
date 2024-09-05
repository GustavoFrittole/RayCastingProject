#include "gameInputs.hpp"

using namespace windowVars;
using namespace rcm;

InputManager::InputManager(ControlsSensitivity& controlsSens, sf::RenderWindow& window, GameStateVars& gameState) :
    m_controlsSens(controlsSens),
    m_window(window),
    m_gameState(gameState)
{}

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
    bool justPaused = false;
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
                {
                    m_gameState.isPaused = true;
                    justPaused = true;
                }
                    
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
            //----player input----
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
            {
                m_inputCache.foreward = + m_controlsSens.movementSpeed;
            }
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
            {
                m_inputCache.foreward = - m_controlsSens.movementSpeed;
            }
            else
            {
                m_inputCache.foreward = 0;
            }

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
            {
                m_inputCache.lateral = + m_controlsSens.movementSpeed;
            }
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
            {
                m_inputCache.lateral = - m_controlsSens.movementSpeed;
            }
            else
            {
                m_inputCache.lateral = 0;
            }

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
            {
                m_inputCache.rotatation = +m_controlsSens.mouseSens;
            }
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
            {
                m_inputCache.rotatation = -m_controlsSens.mouseSens;
            }
            else
            {
                m_inputCache.rotatation = 0;
            }

            if (sf::Mouse::isButtonPressed(sf::Mouse::Left) || sf::Keyboard::isKeyPressed(sf::Keyboard::Q))
            {
                m_inputCache.leftTrigger = true;
            }
            else
            {
                m_inputCache.leftTrigger = false;
            }
            if ((sf::Mouse::isButtonPressed(sf::Mouse::Right) || sf::Keyboard::isKeyPressed(sf::Keyboard::E)))
            {
                m_inputCache.rightTrigger = true;
            }
            else
            {
                m_inputCache.rightTrigger = false;
            }

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
                float rotation = (g_windowWidth / 2 - sf::Mouse::getPosition(m_window).x) * 0.1f;
                if (rotation != 0)
                    m_inputCache.rotatation = rotation;
                sf::Mouse::setPosition(sf::Vector2i(g_windowWidth / 2, 0), m_window);
            }


            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Tab))
            {
                m_gameState.isTabbed = true;
            }
            else
            {
                m_gameState.isTabbed = false;
            }
        }
        else
        {
            if(justPaused)
            {
                m_inputCache.foreward = 0.f;
                m_inputCache.lateral = 0.f;
                m_inputCache.rotatation = 0.f;
                m_inputCache.leftTrigger = false;
                m_inputCache.rightTrigger = false;
                m_window.setMouseCursorVisible(true);
                m_window.setMouseCursorGrabbed(false);
            }
        }
    }
    else
    {
        if (m_gameState.hadFocus)
            m_gameState.hadFocus = false;
    }
}