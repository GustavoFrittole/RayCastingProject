
#include<gameGraphics.hpp>
#include<math.h>
#include"utils.hpp"
#include<thread>
#include<chrono>
#include<stdexcept>
#include <iostream>
using namespace screenStats;

constexpr float HALF_WALL_HEIGHT = 0.5f;


GameGraphics::GameAsset::GameAsset(int width, int height, bool createPixelArray = false)
{
    create(width, height, createPixelArray);
}

void GameGraphics::GameAsset::create(int width, int height, bool createPixelArray )
{
    m_hasPixelArray = createPixelArray;
    if (createPixelArray)
        m_pixels = new sf::Uint8[width * height * 4];
    m_texture.create(width, height);
    m_sprite.setTexture(m_texture);
}

GameGraphics::GameGraphics(GameCore& gameCore, const std::string& gameName) 
    : m_gameCore(gameCore), m_window{ sf::VideoMode(screenStats::g_screenWidth, screenStats::g_screenHeight), gameName },
    m_raysInfoVec(gameCore.getRayInfoArr()), m_playerController(gameCore), m_mapData(gameCore.getMapData())
{
    m_window.clear(sf::Color::Black);
    m_mainView.create(g_screenWidth, g_screenHeight, true);
    m_mainBackground.create(g_screenWidth, g_screenHeight, true);
    generate_background();
    load_end_screen();
    //create_minimap_frame();
}

void GameGraphics::start()
{
    m_gameCore.load_map_form_file();

    //step by step map generation (if requested)
    while (m_gameCore.generate_map_step())
    {
        std::thread sleep([]{ std::this_thread::sleep_for(std::chrono::milliseconds(GENERATION_TIME_STEP_MS)); });
        sf::Event event;
        //could be handled in handle_events() but requires the addition of a dedicated object state
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

    handle_events();
    m_gameCore.update_entities();

    debug::GameTimer gt;
    gt.reset_timer();

    m_gameCore.view_by_ray_casting();
    //std::cout << " casting    \t" << gt.reset_timer() << std::endl;
    draw_background();
    draw_camera_view();
    //std::cout << " draw view  \t" << gt.reset_timer() << std::endl;
    if (m_paused)
    {
        draw_map();
    }
    else
    {
        draw_minimap_background();
        draw_minimap_triangles();
        if (goal_reached())
        {
            //TODO not to call every cycle
            draw_end_screen();
        }
    }
    //std::cout << " draw minimap\t" << gt.reset_timer() << std::endl;
    m_window.display();
}

bool GameGraphics::goal_reached()
{
    return (m_mapData.gameMap.cells.at(static_cast<int>(m_mapData.playerTransform.coords.x)
        + static_cast<int>(m_mapData.playerTransform.coords.y) * m_mapData.gameMap.x) == 'g');
}

void GameGraphics::load_end_screen()
{
    if (!m_endGameFont.loadFromFile("assets\\Roboto-Regular.ttf"))
    {
        throw std::runtime_error("Could not load asset: font.");
    }
    m_endGameText.setFont(m_endGameFont);
    m_endGameText.setString("GOAL REACHED");
    m_endGameText.setCharacterSize(50);
    m_endGameText.setFillColor(sf::Color::Magenta);
    m_endGameText.setPosition((g_screenWidth - m_endGameText.getLocalBounds().width)/2,
                                (g_screenHeight- m_endGameText.getLocalBounds().height)/2);
    
}
void GameGraphics::draw_end_screen()
{
    m_window.draw(m_endGameText);
}

void GameGraphics::create_minimap_frame() 
{
    float frameRadius = m_minimapInfo.minimapH;
    m_minimapFrame.setRadius(frameRadius);
    m_minimapFrame.setPosition(m_minimapInfo.minimapW - frameRadius, m_minimapInfo.minimapH - frameRadius);
    m_minimapFrame.setFillColor(sf::Color::Transparent);
    m_minimapFrame.setOutlineThickness(3.f);
    m_minimapFrame.setOutlineColor(sf::Color::Cyan);
}

//Very unoptimized. all static textures should not be re-generated every frame (mini map "light fade off", fp "radial light")
void GameGraphics::draw_camera_view()
{
    float half_fov = m_mapData.fov * .5f;
    float halfWallHeight = HALF_WALL_HEIGHT;
    float oneOverVerticalVisibleAngle = (g_screenWidth / (g_screenHeight * half_fov));
    for (int i = 0; i < m_raysInfoVec.arrSize; ++i)
    {

        float distance = m_raysInfoVec.const_at(i).hitPos.lenght();
        float wallHeightFromHorizon = (0.5f * g_screenHeight * (1 - (std::atan(halfWallHeight / distance) * oneOverVerticalVisibleAngle)));
        //float wallHeightFromHorizon = (g_height * ( 1 -  1/distance))*0.5f; //faster
        sf::Color pixelColor;
        sf::Uint8 wallShade = (1 - (distance/(m_mapData.maxRenderDist))) * 0xFF;
        for (int y = 0; y < g_screenHeight * 4; y += 4)
        {
            int x = i * 4;
            if (y > wallHeightFromHorizon * 4 && y <= (g_screenHeight - wallHeightFromHorizon) * 4)
            {
                switch (m_raysInfoVec.const_at(i).entityHit)
                {
                case EntityType::Wall:
                    pixelColor = { 0xff, 0xff, 0xff , wallShade };
                    break;
                case EntityType::Baudry:
                    pixelColor = { 0xff, 0xaa, 0xff , wallShade };
                    break;
                default: 
                    pixelColor = sf::Color::Transparent;
                    break;
                }
            }
            else
            {
                pixelColor = sf::Color::Transparent;
            }
            m_mainView.m_pixels[y * g_screenWidth + x] = pixelColor.r;
            m_mainView.m_pixels[y * g_screenWidth + x + 1] = pixelColor.g;
            m_mainView.m_pixels[y * g_screenWidth + x + 2] = pixelColor.b;
            m_mainView.m_pixels[y * g_screenWidth + x + 3] = pixelColor.a;
        }
    }
    m_mainView.m_texture.update(m_mainView.m_pixels);
    m_window.draw(m_mainView.m_sprite);
}

void GameGraphics::generate_background()
{
    for (int i = 0; i < g_screenWidth; ++i)
    {
        sf::Color pixelColor;
        for (int y = 0; y < g_screenHeight * 4; y += 4)
        {
            int x = i * 4;
            
            sf::Uint8 floorShade;
            if (y < g_screenHeight * 2)
            {
                floorShade = 0x88 * ((std::abs(g_screenHeight * 2 - y)) / (float)(g_screenHeight * 2));
                pixelColor = { 0x88, 0x88, 0xff, floorShade };
            }
            else
            {
                floorShade = 0xff * ((std::abs(g_screenHeight * 2 - y) * std::abs(g_screenHeight * 2 - y)) / (float)((g_screenHeight * 2) * (g_screenHeight * 2)));
                pixelColor = { 0x88, 0x88, 0xff, floorShade };
            }
            m_mainBackground.m_pixels[y * g_screenWidth + x] = pixelColor.r;
            m_mainBackground.m_pixels[y * g_screenWidth + x + 1] = pixelColor.g;
            m_mainBackground.m_pixels[y * g_screenWidth + x + 2] = pixelColor.b;
            m_mainBackground.m_pixels[y * g_screenWidth + x + 3] = pixelColor.a;
        }
    }
    m_mainBackground.m_texture.update(m_mainBackground.m_pixels);
}

void GameGraphics::draw_background()
{
    m_window.draw(m_mainBackground.m_sprite);
}

void GameGraphics::draw_minimap_rays()
{
    sf::VertexArray lines(sf::Lines, m_raysInfoVec.arrSize*2);
    for (int i = 0; i < m_raysInfoVec.arrSize; ++i)
    {
        lines[i * 2] = sf::Vector2f(m_minimapInfo.minimapW, m_minimapInfo.minimapH);
        lines[i * 2 + 1] = { { m_minimapInfo.minimapW + m_raysInfoVec.const_at(i).hitPos.x * m_minimapInfo.minimapScale,
            m_minimapInfo.minimapH + m_raysInfoVec.const_at(i).hitPos.y * m_minimapInfo.minimapScale}, sf::Color::Cyan };
    }
    m_window.draw(lines);
}

void GameGraphics::draw_minimap_triangles()
{
    sf::VertexArray triangles(sf::TriangleFan, m_raysInfoVec.arrSize + 1);
    triangles[0] = sf::Vertex{ {(float)m_minimapInfo.minimapW, (float)m_minimapInfo.minimapH}, sf::Color::Cyan };
    for (int i = 0; i < m_raysInfoVec.arrSize; ++i)
    {
        triangles[i + 1] = { { m_minimapInfo.minimapW + m_raysInfoVec.const_at(i).hitPos.x * m_minimapInfo.minimapScale, m_minimapInfo.minimapH + m_raysInfoVec.const_at(i).hitPos.y * m_minimapInfo.minimapScale},
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
            char currentCell = m_mapData.gameMap.cells.at(i * m_mapData.gameMap.x + c);

            if (currentCell != ' ')
            {
                if(currentCell == 'b')
                    wallRect.setFillColor(sf::Color::Black);
                if (currentCell == 'w')
                    wallRect.setFillColor(sf::Color::Magenta);
                if (currentCell == 'g')
                    wallRect.setFillColor(sf::Color::Blue);

                wallRect.setPosition({ (float)c * tileDim + xoffset, (float)i * tileDim + yoffset});
                m_window.draw(wallRect);
            }
        }
    }

    sf::CircleShape playerC(tileDim / 2.f);
    playerC.setFillColor(sf::Color::Cyan);
    playerC.setPosition({ (float)(int(m_mapData.playerTransform.coords.x) * tileDim + xoffset),
                          (float)(int(m_mapData.playerTransform.coords.y) * tileDim + yoffset )});
    
    m_window.draw(playerC);
}

void GameGraphics::draw_minimap_background()
{
    int tileDim = m_minimapInfo.minimapScale;
    int xoffset = m_minimapInfo.minimapW;
    int yoffset = m_minimapInfo.minimapH;
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
            char currentCell = m_mapData.gameMap.cells.at(i * m_mapData.gameMap.x + c);

            if (currentCell != ' ')
            {
                char currentCell = m_mapData.gameMap.cells.at(i * m_mapData.gameMap.x + c);
                if (currentCell == 'b' || currentCell == 'w')
                    wallRect.setFillColor(sf::Color::Black);
                else if (currentCell == 'g')
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