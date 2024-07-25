
#include<gameGraphics.hpp>
#include<math.h>
#include"utils.hpp"
#include<iostream>
#include<thread>
#include<chrono>

using namespace screenStats;

GameGraphics::GameGraphics(GameCore& gameCore, const std::string& gameName) 
    : m_gameCore(gameCore), m_window{ sf::VideoMode(screenStats::g_screenWidth, screenStats::g_screenHeight), gameName },
    m_raysInfoVec(gameCore.getRayInfoArr()), m_playerController(gameCore), m_mapData(gameCore.getMapData())
{
    m_window.clear(sf::Color::Black);

    m_screenPixels = new sf::Uint8[g_screenWidth * g_screenHeight * 4];

    m_viewTexture.create(g_screenWidth, g_screenHeight);
    m_viewSprite.setTexture(m_viewTexture);

    m_mapTexture.create(g_screenWidth, g_screenHeight);
    m_mapSprite.setTexture(m_viewTexture);

    //create_minimap_frame();
}

void GameGraphics::start()
{
    m_gameCore.load_map_form_file();
    while (m_gameCore.generate_map_step())
    {
        std::thread sleep([] {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            });
        sf::Event event;
        while (m_window.pollEvent(event))
        {
            switch (event.type)
            {
            case sf::Event::Closed:
                m_window.close();
                break;
            }
        }
        m_window.clear(sf::Color::White);
        draw_map();
        m_window.display();
        sleep.join();
    }
    m_gameCore.start_internal_time();
    m_paused = true;
}
//TODO add flags
void GameGraphics::performGameCycle() 
{
    m_window.clear(sf::Color::Black);

    debug::GameTimer gt;
    gt.reset_timer();

    handle_events();
    m_gameCore.update_entities();
    m_gameCore.view_by_ray_casting();
    //std::cout << " casting    \t" << gt.reset_timer() << std::endl;
    draw_camera_view();
    //std::cout << " draw view  \t" << gt.reset_timer() << std::endl;
    //std::cout << " draw frame\t" << gt.reset_timer() << std::endl;
    
    //std::cout << " draw minimap\t" << gt.reset_timer() << std::endl;
    if (m_paused)
    {
        draw_map();
    }
    else
    {
        //m_window.draw(m_minimapFrame);
        draw_map_back();
        draw_minimap();
    }
    m_window.display();
}

void GameGraphics::create_minimap_frame() 
{
    float frameRadius = m_minimapInfo.minimapY;
    m_minimapFrame.setRadius(frameRadius);
    m_minimapFrame.setPosition(m_minimapInfo.minimapX - frameRadius, m_minimapInfo.minimapY - frameRadius);
    m_minimapFrame.setFillColor(sf::Color::Transparent);
    m_minimapFrame.setOutlineThickness(3.f);
    m_minimapFrame.setOutlineColor(sf::Color::Cyan);
}
//Very unoptimized. all static textures should not be re-generated every frame (mini map "light fade off", fp "radial light")

void GameGraphics::draw_camera_view()
{
    float half_fov = m_mapData.fov * .5f;
    float halfWallHeight = 0.5f;
    float oneOverViewAngle = (g_screenWidth / (g_screenHeight * half_fov));
    for (int i = 0; i < m_raysInfoVec.arrSize; ++i)
    {

        float distance = m_raysInfoVec.const_at(i).hitPos.lenght();
        float wallHeightFromHorizon = (0.5f * g_screenHeight * (1 - (std::atan(halfWallHeight / distance) * oneOverViewAngle)));
        //float wallHeightFromHorizon = (g_height * ( 1 -  1/distance))*0.5f; //faster
        sf::Color pixelColor;
        sf::Uint8 wallShade = (1 - (distance/(m_mapData.maxRenderDist))) * 0xFF;
        for (int y = 0; y < g_screenHeight * 4; y += 4)
        {
            int x = i * 4;
            if ((m_raysInfoVec.const_at(i).entityHit != EntityType::Wall) || (y < wallHeightFromHorizon * 4 || y >(g_screenHeight - wallHeightFromHorizon) * 4))
            {
                sf::Uint8 floorShade;
                if (y < wallHeightFromHorizon * 4)
                {
                    floorShade = 0x88 *((std::abs(g_screenHeight * 2 - y)) / (float)(g_screenHeight * 2));
                    pixelColor = { 0x88, 0x88, 0xff, floorShade };
                }
                else 
                {
                    floorShade = 0xff * ((std::abs(g_screenHeight * 2 - y) * std::abs(g_screenHeight * 2 - y)) / (float)((g_screenHeight * 2) * (g_screenHeight * 2)));
                    pixelColor = { 0x88, 0x88, 0xff, floorShade };
                }
            }
            else
            {
                pixelColor = { 0xff, 0xff, 0xff , wallShade };
            }
            m_screenPixels[y * g_screenWidth + x] = pixelColor.r;
            m_screenPixels[y * g_screenWidth + x + 1] = pixelColor.g;
            m_screenPixels[y * g_screenWidth + x + 2] = pixelColor.b;
            m_screenPixels[y * g_screenWidth + x + 3] = pixelColor.a;
        }
    }
    m_viewTexture.update(m_screenPixels);
    m_window.draw(m_viewSprite);
}

void GameGraphics::draw_minimap_rays()
{
    sf::VertexArray lines(sf::Lines, m_raysInfoVec.arrSize*2);
    for (int i = 0; i < m_raysInfoVec.arrSize; ++i)
    {
        lines[i * 2] = sf::Vector2f(m_minimapInfo.minimapX, m_minimapInfo.minimapY);
        lines[i * 2 + 1] = { { m_minimapInfo.minimapX + m_raysInfoVec.const_at(i).hitPos.x * m_minimapInfo.minimapScale,
            m_minimapInfo.minimapY + m_raysInfoVec.const_at(i).hitPos.y * m_minimapInfo.minimapScale}, sf::Color::Cyan };
    }
    m_window.draw(lines);
}

void GameGraphics::draw_minimap()
{
    sf::VertexArray triangles(sf::TriangleFan, m_raysInfoVec.arrSize + 1);
    triangles[0] = sf::Vertex{ {(float)m_minimapInfo.minimapX, (float)m_minimapInfo.minimapY}, sf::Color::Cyan };
    for (int i = 0; i < m_raysInfoVec.arrSize; ++i)
    {
        triangles[i + 1] = { { m_minimapInfo.minimapX + m_raysInfoVec.const_at(i).hitPos.x * m_minimapInfo.minimapScale, m_minimapInfo.minimapY + m_raysInfoVec.const_at(i).hitPos.y * m_minimapInfo.minimapScale},
        sf::Color(0, 0xFF, 0xFF, (1 - m_raysInfoVec.const_at(i).hitPos.lenght() / m_mapData.maxRenderDist) * 0xFF) };
    }
    m_window.draw(triangles);
}

void GameGraphics::draw_map()
{
    int xoverw = g_screenWidth / m_mapData.gameMap.x;
    int yoverh = g_screenHeight / m_mapData.gameMap.y;
    int tileDim = std::min(xoverw, yoverh);
    //float tileThick = tileDim/10.f;
    int xoffset = (g_screenWidth - m_mapData.gameMap.x * tileDim)/2;
    int yoffset = (g_screenHeight - m_mapData.gameMap.y * tileDim)/2;
    sf::RectangleShape wallRect({ (float)tileDim, (float)tileDim });

    for (int i = 0; i < m_mapData.gameMap.y; ++i)
    {
        for (int c = 0; c < m_mapData.gameMap.x; ++c)
        {
            if (m_mapData.gameMap.cells.at(i * m_mapData.gameMap.x + c) != ' ')
            {
                if(m_mapData.gameMap.cells.at(i * m_mapData.gameMap.x + c) == 'e')
                    wallRect.setFillColor(sf::Color::Black);
                if (m_mapData.gameMap.cells.at(i * m_mapData.gameMap.x + c) == 'w')
                    wallRect.setFillColor(sf::Color::Magenta);
                if (m_mapData.gameMap.cells.at(i * m_mapData.gameMap.x + c) == 'g')
                    wallRect.setFillColor(sf::Color::Blue);

                wallRect.setPosition({ (float)c * tileDim +xoffset, (float)i * tileDim + yoffset});
                m_window.draw(wallRect);
            }
        }
    }

    sf::CircleShape playerC(tileDim / 2.f);
    playerC.setFillColor(sf::Color::Cyan);
    playerC.setPosition({ (float)(int(m_mapData.playerTransform.coords.x) * tileDim + xoffset), (float)(int(m_mapData.playerTransform.coords.y) * tileDim + yoffset )});
    
    m_window.draw(playerC);
        
}

void GameGraphics::draw_map_back()
{
    int tileDim = m_minimapInfo.minimapScale;
    int xoffset = m_minimapInfo.minimapX;
    int yoffset = m_minimapInfo.minimapY;
    int startX = m_mapData.playerTransform.coords.x - m_mapData.maxRenderDist;
    if (startX < 0)
        startX = 0;
    int endX = m_mapData.playerTransform.coords.x + m_mapData.maxRenderDist;
    if (endX > m_mapData.gameMap.x)
        endX = m_mapData.gameMap.x;
    int startY = m_mapData.playerTransform.coords.y - m_mapData.maxRenderDist;
    if (startY < 0)
        startY = 0;
    int endY = m_mapData.playerTransform.coords.y + m_mapData.maxRenderDist;
    if (endY > m_mapData.gameMap.y)
        endY = m_mapData.gameMap.y;

    sf::RectangleShape wallRect({ (float)tileDim, (float)tileDim });

    for (int i = startY; i < endY; ++i)
    {
        for (int c = startX; c < endX; ++c)
        {
            if (m_mapData.gameMap.cells.at(i * m_mapData.gameMap.x + c) != ' ')
            {
                if (m_mapData.gameMap.cells.at(i * m_mapData.gameMap.x + c) == 'e' || m_mapData.gameMap.cells.at(i * m_mapData.gameMap.x + c) == 'w')
                    wallRect.setFillColor(sf::Color::Black);
                if (m_mapData.gameMap.cells.at(i * m_mapData.gameMap.x + c) == 'g')
                    wallRect.setFillColor(sf::Color::Blue);

                wallRect.setPosition({ tileDim * ((float)c   - m_mapData.playerTransform.coords.x)+ xoffset,
                                        tileDim * ((float)i - m_mapData.playerTransform.coords.y)+ yoffset });
                m_window.draw(wallRect);
            }
        }
    }
}

void GameGraphics::handle_events()
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
                if (m_paused == true)
                {
                    m_paused = false;
                    justUnpaused = true;
                }
                else
                    m_paused = true;
            }
        }
    }

    if (m_window.hasFocus())
    {
        if (!m_paused)
        {
            if (!m_hadFocus || justUnpaused)
            {
                m_hadFocus = true;
                justUnpaused = false;
                m_window.setMouseCursorVisible(false);
                m_window.setMouseCursorGrabbed(true);
                sf::Mouse::setPosition(sf::Vector2i(100, 0), m_window);
            }
            else
            {
                m_playerController.rotate((-sf::Mouse::getPosition(m_window).x + 100) * 0.1);
                sf::Mouse::setPosition(sf::Vector2i(100, 0), m_window);
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
            {
                m_playerController.move_foreward(2.f);
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
            {
                m_playerController.move_strafe(-2.f);
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
            {
                m_playerController.move_strafe(+2.f);
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
            {
                m_playerController.move_foreward(-2.f);
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
            {
                m_playerController.rotate(2.0f);
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
            {
                m_playerController.rotate(-2.0f);
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
        if (m_hadFocus)
            m_hadFocus = false;
    }
}