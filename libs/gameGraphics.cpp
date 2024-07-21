
#include<gameGraphics.hpp>
#include<glm/ext/scalar_constants.hpp>
#include<glm/trigonometric.hpp>
#include<glm/mat2x2.hpp>

using namespace ScreenStats;

GameGraphics::GameGraphics(GameCore& gameCore, const std::string& gameName) 
    : m_gameCore(gameCore), m_raysInfoVec(0), m_window{ sf::VideoMode(ScreenStats::g_screenWidth, ScreenStats::g_screenHeight), gameName }, 
    m_playerController(gameCore), m_mapData(gameCore.getMapData())
{
    m_window.setVisible(false);
    m_screenPixels = new sf::Uint8[g_screenWidth * g_screenHeight * 4];
    m_raysInfoVec.reserve(ScreenStats::g_screenWidth);
    //window.setFramerateLimit(0);
    
    //create canvas
    m_viewTexture.create(g_screenWidth, ScreenStats::g_screenHeight);

    m_viewSprite.setTexture(m_viewTexture);

    std::vector<RayInfo> rayInfoVec;
}

void GameGraphics::start()
{
    m_window.setVisible(true);
    m_gameCore.start_internal_time();
}
//TODO add flags
void GameGraphics::performGameCycle() 
{
    m_window.clear(sf::Color::Black);
    m_gameCore.update_entities();
    m_raysInfoVec = m_gameCore.view_by_ray_casting();
    draw_camera_view();
    draw_minimap();
    m_window.display();
}

void GameGraphics::handle_events()
{
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
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
    {
        m_playerController.move_foreward(1.f);
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
    {
        m_playerController.move_strafe(-1.f);
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
    {
        m_playerController.move_strafe(+1.f);
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
    {
        m_playerController.move_foreward(-1.f);
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
    {
        m_playerController.rotate(1.0f);
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
    {
        m_playerController.rotate(-1.0f);
    }
    sf::Mouse::getPosition().x;

}

void GameGraphics::draw_camera_view()
{
    float half_fov = m_mapData.fov * .5f;
    float halfWallHeight = 0.5f;
    float oneOverViewAngle = (g_screenWidth / (g_screenHeight * half_fov));
    for (int i = 0; i < m_raysInfoVec.size(); ++i)
    {
        //if (raysInfo[i].entityHit == EntityType::Wall) 
        {
            float distance = glm::length(m_raysInfoVec[i].hitPos);
            float wallHeightFromHorizon = (0.5f * g_screenHeight * (1 - (glm::atan(halfWallHeight / distance) * oneOverViewAngle)));
            //float wallHeightFromHorizon = (g_height * ( 1 -  1/distance))*0.5f; //faster
            sf::Color pixelColor;
            sf::Uint8 wallShade = (1 - distance * 0.125f) * 0xFF;
            for (int y = 0; y < g_screenHeight * 4; y += 4)
            {
                int x = i * 4;
                if (!(m_raysInfoVec[i].entityHit == EntityType::Wall) || (y < wallHeightFromHorizon * 4 || y >(g_screenHeight - wallHeightFromHorizon) * 4))
                {
                    sf::Uint8 floorShade = 0xff * glm::abs(g_screenHeight * 2 - y) / (g_screenHeight * 4);
                    pixelColor = { 0x33, 0xff, 0xff, floorShade };
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
    }
    m_viewTexture.update(m_screenPixels);
    m_window.draw(m_viewSprite);
}
void GameGraphics::draw_minimap()
{
    sf::VertexArray lines(sf::Lines, m_raysInfoVec.size() * 2);
    float multip = 32;
    for (int i = 0; i < m_raysInfoVec.size(); ++i)
    {
        lines[i * 2] = sf::Vector2f(m_minimapX, m_minimapY);
        lines[i * 2 + 1] = { { m_minimapX + m_raysInfoVec[i].hitPos[0] * multip, m_minimapY + m_raysInfoVec[i].hitPos[1] * multip}, sf::Color(i % 256, 0xFF, 0xFF) };
    }
    m_window.draw(lines);
}