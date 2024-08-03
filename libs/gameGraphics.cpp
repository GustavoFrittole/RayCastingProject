
#include<gameGraphics.hpp>
#include<math.h>
#include<stdexcept>
#include <iostream>

using namespace screenStats;

//---------------------------GAME-ASSET---

void GameAsset::create(int width, int height, bool createPixelArray = false)
{
    m_hasPixelArray = createPixelArray;
    if (createPixelArray)
        m_pixels = new sf::Uint8[width * height * 4];
    m_texture.create(width, height);
    m_sprite.setTexture(m_texture);
}

inline GameAsset::GameAsset(int width, int height, bool createPixelArray = false)
{
    create(width, height, createPixelArray);
}

void Texture::create(const std::string& filePath)
{
    m_texture.loadFromFile(filePath);
    m_texturePixels = m_texture.getPixelsPtr();
}
const
sf::Uint8& Texture::get_pixel_at(int index) const
{
    if (index < 0 || index >= width() * height() * 4)
        throw std::invalid_argument("Index is out of range.");
    else
        return m_texturePixels[index];
}

//---------------------------GAME-GRAPHICS---

GameGraphics::GameGraphics(std::unique_ptr<DataUtils::GameData>& gameData, const std::string& gameName)
    : m_gameCore(gameData->gCamera, gameData->gMap, gameData->playerTrasform), 
    m_window{ sf::VideoMode(screenStats::g_screenWidth, screenStats::g_screenHeight), gameName },
    m_raysInfoVec(m_gameCore.getRayInfoArr()), 
    m_playerController(m_gameCore.get_playerController()),
    m_pathToGoal(0), 
    m_minimapInfo(gameData->screenStats.minimapScale, m_mapData.maxRenderDist),
    m_halfWallHeight(gameData->screenStats.halfWallHeight),
    m_mapData(m_gameCore.getMapData()),
    m_gameAssets(gameData->gAssets),
    m_renderingThreadPool(*this)
{
    m_window.clear(sf::Color::Black);
    m_window.setFramerateLimit(0);
}

//----compund-funtions

void GameGraphics::start()
{
    create_assets();
    //step by step map generation (if generation is requested)
    while (m_gameCore.generate_map_step())
    {
        std::thread sleep([] { std::this_thread::sleep_for(std::chrono::milliseconds(GENERATION_TIME_STEP_MS)); });
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

void GameGraphics::create_assets()
{
    m_mainView.create(g_screenWidth, g_screenHeight, true);
    m_mainBackground.create(g_screenWidth, g_screenHeight, true);
    generate_background();
    load_end_screen();
    m_pathFinder = std::make_unique<PathFinder>(m_mapData.gameMap.x, m_mapData.gameMap.y, *(m_mapData.gameMap.cells), m_pathToGoal);
    m_igMapAssets.create(*this);
    //wip
    m_wallTexture.create(m_gameAssets.wallTexFilePath);
    m_baundryTexture.create(m_gameAssets.boundryTexFilePath);
    m_floorTexture.create(m_gameAssets.floorTexFilePath);
}

//TODO add flags
void GameGraphics::performGameCycle() 
{
    m_window.clear(sf::Color::Black);

    handle_events();
    m_gameCore.update_entities();

    if (m_findPathRequested && m_mapData.gameMap.generated)
    {
        m_pathFinder->find_path(m_mapData.playerTransform.coords.x, m_mapData.playerTransform.coords.y);
        m_findPathRequested = false;
    }

    //debug::GameTimer gt;
    //gt.reset_timer();

    m_gameCore.view_by_ray_casting();
    //std::cout << " casting    \t" << gt.reset_timer() << std::endl;
    draw_textured_background();
    draw_background();
    //std::cout << " draw back  \t" << gt.reset_timer() << std::endl;
    //std::cout << " draw view  \t" << gt.reset_timer() << std::endl;
    draw_view();
    //gt.reset_timer();
    if (m_paused || m_tabbed)
    {
        draw_map();
        draw_path_out();
    }
    else
    {
        draw_minimap_background();
        draw_minimap_triangles();
        if (goal_reached())
        {
            draw_end_screen();
        }
    }
    //std::cout << " draw minimap\t" << gt.reset_timer() << std::endl;
    m_window.display();
}

inline void GameGraphics::draw_view()
{
    m_renderingThreadPool.render_view();
    m_mainView.m_texture.update(m_mainView.m_pixels);
    m_window.draw(m_mainView.m_sprite);
}

//------

inline bool GameGraphics::goal_reached()
{
    return (m_mapData.gameMap.cells->at(static_cast<int>(m_mapData.playerTransform.coords.x)
        + static_cast<int>(m_mapData.playerTransform.coords.y) * m_mapData.gameMap.x) == 'g');
}

//----------------end-screen

void GameGraphics::load_end_screen()
{
    if (m_endGameFont.loadFromFile("assets\\Roboto-Regular.ttf"))
    {
        m_endGameText.setFont(m_endGameFont);
    }
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

//------------------THREAD-POOL---

GameGraphics::RenderingThreadPool::RenderingThreadPool(GameGraphics& gg) : 
                                    m_poolSize(get_thread_number()), 
                                    m_threads(0), 
                                    m_mutexVecStart(m_poolSize),
                                    m_mutexVecEnd(m_poolSize),
                                    m_gameGraphics(gg),
                                    jobs(m_poolSize)
{
    for (int i = 0; i < m_poolSize; ++i)
    {
        m_mutexVecStart.at(i).lock();
    }

    int sectionsSize = m_gameGraphics.m_raysInfoVec.arrSize / (m_poolSize);
    int currentSection;
    int id = 0;
    for (currentSection = 0; currentSection + sectionsSize <= m_gameGraphics.m_raysInfoVec.arrSize; currentSection += sectionsSize, ++id)
    {
        m_threads.emplace_back([this, currentSection, sectionsSize, id] () mutable
            {
                while (true)
                {
                    jobs--;
                    {
                        std::unique_lock<std::mutex> lock(m_mutexVecStart.at(id));
                        if (!m_isActive) {
                            return;
                        }
                    }
                    draw_section(currentSection, (currentSection + sectionsSize));
                    jobs++;
                    {
                        std::unique_lock<std::mutex> lock(m_mutexVecEnd.at(id));
                    }
                    
                }
            });
    }
    m_lastSectionSize = m_gameGraphics.m_raysInfoVec.arrSize - currentSection;
}

GameGraphics::RenderingThreadPool::~RenderingThreadPool()
{
    m_isActive = false;
    for (auto& m : m_mutexVecStart)
        m.unlock();

    for (auto& t : m_threads)
    {
        t.join();
    }
}

void GameGraphics::RenderingThreadPool::render_view()
{
    while (jobs != 0)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(0));
    }

    for (auto& m : m_mutexVecEnd)
        m.lock();
    for (auto& m : m_mutexVecStart)
        m.unlock();

    if (m_lastSectionSize != 0)
    {
        draw_section(m_gameGraphics.m_raysInfoVec.arrSize - m_lastSectionSize, m_gameGraphics.m_raysInfoVec.arrSize);
    }

    while (jobs != m_poolSize)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(0));
    }

    for (auto& m : m_mutexVecStart)
        m.lock();
    for (auto& m : m_mutexVecEnd)
        m.unlock();
}

void GameGraphics::RenderingThreadPool::draw_section(int start, int end)
{
    for (int i = start; i < end; ++i)
    {
        const RayInfo& currRay = m_gameGraphics.m_raysInfoVec.const_at(i);
        float distance = currRay.length;

        //--this version maintains correct proportions, but causes texture distortion--
        //float wallAngle = (std::atan(m_gameGraphics.m_halfWallHeight / distance) );
        //float screenWallHeight = g_screenHeight  * wallAngle / (m_verticalVisibleAngle * m_gameGraphics.m_halfWallHeight);

        //--this version is faster, has easy texture mapping but locks th evertical view angle at 90° (45° up 45° down)--
        float screenWallHeight = (g_screenHeight / distance) * m_gameGraphics.m_halfWallHeight;
        float floorHeight = (g_screenHeight - screenWallHeight) / 2;

        

        //can multiply with the alpha of walls, bleanding them with the dark backgroud 
        sf::Uint8 boxShade = (1 - (distance / (m_gameGraphics.m_mapData.maxRenderDist))) * 0xFF;

        bool flatShading = false;
        Texture* currentTexture;
        sf::Color flatColor;
        switch (currRay.entityHit)
        {
        case EntityType::Wall:
            currentTexture = &m_gameGraphics.m_wallTexture;
            break;
        case EntityType::Baudry:
            currentTexture = &m_gameGraphics.m_baundryTexture;

            //flatShading = true;
            //flatColor = { 0xff, (currRay.lastSideChecked == CellSide::Hori) ? (sf::Uint8)0xff : (sf::Uint8)0x0, 0xff , boxShade };
            break;
        default:
            flatShading = true;
            flatColor = sf::Color::Transparent;
            break;
        }

        sf::Color currentColor;

        //--variables for texture navigation--
        //relative position in wall based on what face is being textured
        float posOnWallSide = 0;

        //x alias u
        int textureU = 0;
        //y alias v
        float textureV = 0;
        float texVStep = 0;

        if(!flatShading)
        {
            if (currRay.lastSideChecked == CellSide::Hori)
                posOnWallSide = currRay.hitPos.x + m_gameGraphics.m_mapData.playerTransform.coords.x;
            else
                posOnWallSide = currRay.hitPos.y + m_gameGraphics.m_mapData.playerTransform.coords.y;

            posOnWallSide -= std::floorf(posOnWallSide);

            texVStep = currentTexture->height() / screenWallHeight;

            textureU = (int)(posOnWallSide * currentTexture->width());

            //if the wall is seen from the south or west side, its texture's us need to be inverted 
            if (currRay.lastSideChecked == CellSide::Hori && currRay.hitPos.y < 0 ||
                currRay.lastSideChecked == CellSide::Vert && currRay.hitPos.x > 0) 
                textureU = currentTexture->width() - textureU - 1;

            //if the textured box is bigger then the screen (hight wise), the initial unseen part of pixels must be skipped
            textureV = (screenWallHeight > g_screenHeight) ? texVStep * ((screenWallHeight - g_screenHeight) / 2) : 0;
        }

        //scan y axis
        int x = i * 4;
        for (int y = 0; y < g_screenHeight * 4; y += 4)
        {
            //wall or ceiling\floor?
            if (y > floorHeight * 4 && y <= (g_screenHeight - floorHeight) * 4)
            {
                if (flatShading)
                {
                    currentColor = flatColor;
                }
                else
                {
                    //if the end of the texture is reached, start over
                    if (textureV >= currentTexture->height())
                        textureV = 0;
                    //or textureV = textureV%currentTexture->height();

                    currentColor.r = currentTexture->m_texturePixels
                        [(textureU + int(textureV) * currentTexture->width())*4 +0];
                    currentColor.g = currentTexture->m_texturePixels
                        [(textureU + int(textureV) * currentTexture->width())*4 +1];
                    currentColor.b = currentTexture->m_texturePixels
                        [(textureU + int(textureV) * currentTexture->width())*4 +2];
                    currentColor.a = boxShade;

                    textureV += texVStep;
                }    
            }
            else
            {
                currentColor = sf::Color::Transparent;
            }
            m_gameGraphics.m_mainView.m_pixels[y * g_screenWidth + x] = currentColor.r;
            m_gameGraphics.m_mainView.m_pixels[y * g_screenWidth + x + 1] = currentColor.g;
            m_gameGraphics.m_mainView.m_pixels[y * g_screenWidth + x + 2] = currentColor.b;
            m_gameGraphics.m_mainView.m_pixels[y * g_screenWidth + x + 3] = currentColor.a;
        }
    }
}

//-------pre-rendered-background

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
                //floorShade = 0xff * (1 -((std::abs(g_screenHeight * 2 - y)) / (float)(g_screenHeight * 2)));
                pixelColor = { 0x88, 0x88, 0xff, floorShade };
            }
            else
            {
                floorShade = 0xff * ((std::abs(g_screenHeight * 2 - y) * std::abs(g_screenHeight * 2 - y)) / (float)((g_screenHeight * 2) * (g_screenHeight * 2)));
                //floorShade = 0xff * (1 - ((std::abs(g_screenHeight * 2 - y)) / (float)(g_screenHeight * 2)));
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

inline void GameGraphics::draw_background()
{
    m_mainBackground.m_texture.update(m_mainBackground.m_pixels);
    m_window.draw(m_mainBackground.m_sprite);
}

//----------------back-ground--



void GameGraphics::draw_textured_background()
{
    math::Vect2 leftmostRayDir = { std::cos(m_mapData.playerTransform.forewardAngle), std::sin(m_mapData.playerTransform.forewardAngle)};
    math::Vect2 rightmostRayDir = leftmostRayDir;
    leftmostRayDir *= math::rotation_mat2x2( + m_mapData.fov/2 );
    rightmostRayDir *= math::rotation_mat2x2( - m_mapData.fov/2 );

    for (int y = 1; y < g_screenHeight/2; ++y)
    {
        float rayLength = (g_screenHeight *2)/ ((float)g_screenHeight - (2 * y));

        math::Vect2 uvIncrement = ((rightmostRayDir - leftmostRayDir) * rayLength) / g_screenWidth;
        math::Vect2 uvPosLeft = m_mapData.playerTransform.coords + (leftmostRayDir * rayLength);
        math::Vect2 uvPosright = m_mapData.playerTransform.coords + (rightmostRayDir * rayLength);
        math::Vect2 uvPos = uvPosLeft;

        for (int x = 0; x < g_screenWidth; ++x)
        {

            m_mainBackground.m_pixels[(y * g_screenWidth + x)*4] = 
                m_floorTexture.m_texturePixels[(int((uvPos.y -int(uvPos.y))* m_floorTexture.height())) * m_floorTexture.width() * 4 + int((uvPos.x - int(uvPos.x))*m_floorTexture.width()) * 4 ];
            m_mainBackground.m_pixels[(y * g_screenWidth + x) * 4 + 3] = 0xFF;
            uvPos += uvIncrement;
        }
    }
    
}

//----------------map-and-mini-map

void GameGraphics::draw_minimap_rays()
{
    sf::VertexArray lines(sf::Lines, m_raysInfoVec.arrSize*2);
    for (int i = 0; i < m_raysInfoVec.arrSize; ++i)
    {
        lines[i * 2] = sf::Vector2f(m_minimapInfo.minimapCenterY, m_minimapInfo.minimapCenterX);
        lines[i * 2 + 1] = { { m_minimapInfo.minimapCenterY + m_raysInfoVec.const_at(i).hitPos.x * m_minimapInfo.minimapScale,
            m_minimapInfo.minimapCenterX + m_raysInfoVec.const_at(i).hitPos.y * m_minimapInfo.minimapScale}, sf::Color::Cyan };
    }
    m_window.draw(lines);
}

void GameGraphics::draw_minimap_triangles()
{
    sf::VertexArray triangles(sf::TriangleFan, m_raysInfoVec.arrSize + 1);
    triangles[0] = sf::Vertex{ {(float)m_minimapInfo.minimapCenterY, (float)m_minimapInfo.minimapCenterX}, sf::Color::Cyan };
    for (int i = 0; i < m_raysInfoVec.arrSize; ++i)
    {
        triangles[i + 1] = { { m_minimapInfo.minimapCenterY + m_raysInfoVec.const_at(i).hitPos.x * m_minimapInfo.minimapScale, m_minimapInfo.minimapCenterX + m_raysInfoVec.const_at(i).hitPos.y * m_minimapInfo.minimapScale},
        sf::Color(0, 0xFF, 0xFF, (1 - m_raysInfoVec.const_at(i).length / m_mapData.maxRenderDist) * 0xFF) };
    }
    m_window.draw(triangles);
}

void GameGraphics::draw_map()
{
    for (int i = 0; i < m_mapData.gameMap.y; ++i)
    {
        for (int c = 0; c < m_mapData.gameMap.x; ++c)
        {
            char currentCell = m_mapData.gameMap.cells->at(i * m_mapData.gameMap.x + c);

            if (currentCell != ' ')
            {
                if (currentCell == 'b')
                    m_igMapAssets.wallRect.setFillColor(sf::Color::Black);
                if (currentCell == 'w')
                    m_igMapAssets.wallRect.setFillColor(sf::Color::Magenta);
                if (currentCell == 'g')
                    m_igMapAssets.wallRect.setFillColor(sf::Color::Blue);

                m_igMapAssets.wallRect.setPosition({ (float)c * m_igMapAssets.tileDim + m_igMapAssets.xoffset, (float)i * m_igMapAssets.tileDim + m_igMapAssets.yoffset });
                m_window.draw(m_igMapAssets.wallRect);
            }
        }
    }

    sf::CircleShape playerC(m_igMapAssets.tileDim / 2.f);
    playerC.setFillColor(sf::Color::Cyan);
    playerC.setPosition({ (float)(int(m_mapData.playerTransform.coords.x) * m_igMapAssets.tileDim + m_igMapAssets.xoffset),
                          (float)(int(m_mapData.playerTransform.coords.y) * m_igMapAssets.tileDim + m_igMapAssets.yoffset) });

    m_window.draw(playerC);
}

void GameGraphics::draw_path_out()
{
    if (goal_reached() || m_pathToGoal.empty())
        return;
    sf::VertexArray lines(sf::Lines, m_pathToGoal.size() * 2);

    lines[0] = { sf::Vector2f(m_igMapAssets.xoffset + m_igMapAssets.tileDim * (m_pathToGoal.at(0).first + 0.5f),
        m_igMapAssets.yoffset + m_igMapAssets.tileDim * (m_pathToGoal.at(0).second + 0.5f)), sf::Color::Cyan };

    lines[1] = { sf::Vector2f(m_igMapAssets.xoffset + m_igMapAssets.tileDim * (m_pathToGoal.at(1).first + 0.5f),
        m_igMapAssets.yoffset + m_igMapAssets.tileDim * (m_pathToGoal.at(1).second + 0.5f)),  sf::Color::Blue };

    for (int i = 2; i < m_pathToGoal.size(); ++i)
    {
        lines[(i - 1) * 2] = lines[(i - 1) * 2 - 1];

        lines[(i - 1) * 2].color = sf::Color::Blue;

        lines[(i - 1) * 2 + 1] = { sf::Vector2f(m_igMapAssets.xoffset + m_igMapAssets.tileDim * (m_pathToGoal.at(i).first + 0.5f),
            m_igMapAssets.yoffset + m_igMapAssets.tileDim * (m_pathToGoal.at(i).second + 0.5f)), sf::Color::Blue };
    }
    m_window.draw(lines);
}

void GameGraphics::draw_minimap_background()
{
    int tileDim = m_minimapInfo.minimapScale;
    int xoffset = m_minimapInfo.minimapCenterY;
    int yoffset = m_minimapInfo.minimapCenterX;
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
            char currentCell = m_mapData.gameMap.cells->at(i * m_mapData.gameMap.x + c);

            if (currentCell != ' ')
            {
                char currentCell = m_mapData.gameMap.cells->at(i * m_mapData.gameMap.x + c);
                if (currentCell == 'b' || currentCell == 'w')
                    wallRect.setFillColor(sf::Color::Black);
                else if (currentCell == 'g')
                    wallRect.setFillColor(sf::Color::Blue);

                wallRect.setPosition({ tileDim * ((float)c - m_mapData.playerTransform.coords.x) + xoffset,
                                        tileDim * ((float)i - m_mapData.playerTransform.coords.y) + yoffset });
                m_window.draw(wallRect);
            }
        }
    }
}

//----------------INPUTS---

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
            if (event.key.scancode == sf::Keyboard::Scan::E)
            {
                m_findPathRequested = true;
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
                sf::Mouse::setPosition(sf::Vector2i(g_screenWidth / 2, 0), m_window);
            }
            else
            {
                m_playerController.rotate((g_screenWidth / 2 - sf::Mouse::getPosition(m_window).x) * 0.1f);
                sf::Mouse::setPosition(sf::Vector2i(g_screenWidth / 2, 0), m_window);
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
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Tab))
            {
                m_tabbed = true;
            }
            else
            {
                m_tabbed = false;
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

//---------------IG-assets---

void GameGraphics::InGameMapAssets::create(const GameGraphics& gg)
{
    int xoverw = g_screenWidth / gg.m_mapData.gameMap.x;
    int yoverh = g_screenHeight / gg.m_mapData.gameMap.y;
    tileDim = std::min(xoverw, yoverh);
    //float tileThick = tileDim/10.f;
    xoffset = (g_screenWidth - gg.m_mapData.gameMap.x * tileDim) / 2;
    yoffset = (g_screenHeight - gg.m_mapData.gameMap.y * tileDim) / 2;
    wallRect.setSize({ (float)tileDim, (float)tileDim });
}

