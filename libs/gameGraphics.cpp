
#include <gameGraphics.hpp>
#include <math.h>
#include <stdexcept>
#include <queue>
#include <iostream>

#define DRAW_LINEAR_SKY

using namespace graphicsVars;

//---------------------------GAME-ASSET---

void GameView::create(int width, int height, bool createPixelArray = false)
{
    m_hasPixelArray = createPixelArray;
    if (createPixelArray)
        m_pixels = new sf::Uint8[width * height * 4];
    m_texture.create(width, height);
    m_sprite.setTexture(m_texture);
}

inline GameView::GameView(int width, int height, bool createPixelArray = false)
{
    create(width, height, createPixelArray);
}

//---------------------------TEXTURE------

void Texture::create(const std::string& filePath)
{
    m_image.loadFromFile(filePath);
    if (m_image.getSize() == sf::Vector2u(0, 0))
    {
        std::string err("Could not load file from path: ");
        err.append(filePath);
        throw std::invalid_argument(err);
    }
    m_texturePixels = m_image.getPixelsPtr();
}
const sf::Uint8& Texture::get_pixel_at(int index) const
{
    if (index < 0 || index >= width() * height() * 4)
        throw std::runtime_error("Index is out of range.");
    else
        return m_texturePixels[index];
}

//---------------------------Thread-pool-Helpers---

ViewRendSectionFactory::ViewRendSection::ViewRendSection(int start, int end, ViewRendSectionFactory* source) :
    IRenderingSection(start, end),
    m_source(source)
{}

void ViewRendSectionFactory::ViewRendSection::operator()() const
{
    if (m_source == nullptr)
        throw std::runtime_error("Render section called while no target was set.");

    //if the section is empty do nothing
    if (m_start == m_end)
        return;

    GameGraphics::draw_view_section(m_start, m_end, m_source->m_state->isLinearPersp, *(m_source->m_rays), *(m_source->m_view), *(m_source->m_graphVars), *(m_source->m_staticTex), *(m_source->m_camTransform));
}

void ViewRendSectionFactory::set_target(const RayInfoArr* rays, GameView* view, const StaticTextures* tex, const GameStateVars* state, const GraphicsVars* graphVars, const EntityTransform* camTransform)
{
    m_rays = rays; 
    m_view = view;
    m_staticTex = tex;
    m_state = state;
    m_graphVars = graphVars;
    m_camTransform = camTransform;
}

ViewRendSectionFactory::ViewRendSection ViewRendSectionFactory::create_section(int index)
{
    int sectionStart = m_sectionSize * index;
    return ViewRendSection(sectionStart, sectionStart + get_section_size(index), this);
}

//---------------------------GAME-GRAPHICS---

GameGraphics::GameGraphics(sf::RenderWindow& window, const GraphicsVars& graphicsVars, const RayInfoArr& raysInfoVec, const GameStateVars& gameState, const EntityTransform& camTransform, int frameRate) :
    m_window(window),
    m_pathToGoal(0),
    m_minimapInfo(graphicsVars.minimapScale, graphicsVars.maxSightDepth),
    m_viewSecFactory(g_screenWidth, get_thread_number()),
    m_rendThreadPool(get_thread_number())
{
    m_window.clear(sf::Color::Black);
    m_window.setFramerateLimit(frameRate);
    m_viewSecFactory.set_target(&raysInfoVec, &m_mainView, &m_staticTextures, &gameState, &graphicsVars, &camTransform);
}

//-------------------------compound-funtions--

void GameGraphics::create_assets(const GameAssets& gameAssets, const GameMap& gameMap)
{
    m_mainView.create(g_screenWidth, g_screenHeight, true);
    m_pathFinder = std::make_unique<PathFinder>(gameMap.x, gameMap.y, *(gameMap.cells), m_pathToGoal);
    m_mapSquareAsset.create(gameMap.x, gameMap.y);

    load_end_screen();
    load_textures(gameAssets);
    crete_view_sections();
}

void GameGraphics::load_textures(const GameAssets& gameAssets)
{
    m_staticTextures.wallTexture.create(gameAssets.wallTexFilePath);
    m_staticTextures.baundryTexture.create(gameAssets.boundryTexFilePath);
    m_staticTextures.floorTexture.create(gameAssets.floorTexFilePath);
    m_staticTextures.ceilingTexture.create(gameAssets.ceilingTexFilePath);
    m_staticTextures.skyTexture.create(gameAssets.skyTexFilePath);
}

void GameGraphics::draw_view()
{
    render_view();

    m_mainView.m_texture.update(m_mainView.m_pixels);
    m_window.draw(m_mainView.m_sprite);
}

void GameGraphics::draw_map_gen(int mapWidth, int mapHeight, int posX, int posY, const std::string& cells)
{
    m_window.clear(sf::Color::Black);
    draw_map(mapWidth, mapHeight, posX, posY, cells);
    m_window.display();
}

void GameGraphics::calculate_shortest_path(const EntityTransform& startPos)
{
    m_pathFinder->find_path(startPos.coords.x, startPos.coords.y);
}

void GameGraphics::load_sprite(const std::string& texturePath)
{
    m_spriteTexturesDict.emplace_back(std::make_unique<Texture>(texturePath));
}

//----------------utils----------

void copy_pixels(sf::Uint8 * pixelsTo, const sf::Uint8 * pixelsFrom, int indexTo, int indexFrom, sf::Uint8 alpha)
{
            pixelsTo[indexTo + 0] = pixelsFrom[indexFrom + 0];
            pixelsTo[indexTo + 1] = pixelsFrom[indexFrom + 1];
            pixelsTo[indexTo + 2] = pixelsFrom[indexFrom + 2];
            pixelsTo[indexTo + 3] = alpha;
}

//----------------end-screen-----

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
/*
void GameGraphics::RenderingThreadPool::refresh_variables() 
{
    m_skyPixPerCircle = (m_gameGraphics.m_staticTextures.skyTexture.width() / (float)(2 * PI));
    m_skyUIncrement = m_skyPixPerCircle * m_gameGraphics.m_stateData.fov / graphicsVars::g_screenWidth;
    m_skyVIncrement = m_gameGraphics.m_staticTextures.skyTexture.height() / ((float)graphicsVars::g_screenHeight);
}
*/

void GameGraphics::crete_view_sections()
{
    for (int i = 0; i < m_rendThreadPool.get_size(); ++i)
        m_viewSectionsVec.push_back(m_viewSecFactory.create_section(i));
}

inline void GameGraphics::rend_view_secs()
{
    int lastSection = m_viewSecFactory.get_size() - 1;

    m_rendThreadPool.new_batch(m_viewSecFactory.get_size()-1);
    for (int i = 0; i < lastSection; ++i)
    {
        m_rendThreadPool.enqueue(&m_viewSectionsVec.at(i));
    }

    ViewRendSectionFactory::ViewRendSection lastSec(m_viewSecFactory.create_section(lastSection));
    lastSec();

    while (m_rendThreadPool.is_busy())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(0));
    }
}
/*

void GameGraphics::render_view()
{

    while (m_rendThreadPool.is_busy())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(0));
    }



    if ( m_lastSectionBackgroud != 0 && m_gameGraphics.isLinearPersp )
    {
        draw_background_section(m_lastSectionBackgroud, g_screenHeight/2, m_gameGraphics.drawSky);
    }

    while (jobs != m_poolSize)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(0));
    }

    if (m_lastSectionView != 0)
    {
        draw_veiw_section( m_lastSectionView, m_gameGraphics.m_raysInfoVec.arrSize, m_gameGraphics.isLinearPersp );
    }

    while (jobs != m_poolSize)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(0));
    }

}
*/

void GameGraphics::render_view()
{
    rend_view_secs();
}

void GameGraphics::draw_view_section(int start, int end, bool linear, const RayInfoArr& rays, GameView& view, const GraphicsVars& graphVars, const StaticTextures& tex, const EntityTransform& camTransform)
{
    for (int i = start; i < end; ++i)
    {
        const RayInfo& currRay = rays.const_at(i);
        float distance = currRay.length;

        //--this version maintains correct proportions, but causes texture distortion--
        //float wallAngle = (std::atan(m_gameGraphics.m_halfWallHeight / distance) );
        //float screenWallHeight = g_screenHeight  * wallAngle / (m_verticalVisibleAngle * m_gameGraphics.m_halfWallHeight);

        //--this version is faster, has easy texture mapping but locks th evertical view angle at 90� (45� up 45� down)--

        float screenWallHeight = (g_screenHeight / distance) * graphVars.halfWallHeight;
        float floorHeight = (g_screenHeight - screenWallHeight) / 2;

        //alpha of walls, bleanding them with the dark backgroud 
        sf::Uint8 boxShade = (1 - (distance / (graphVars.maxSightDepth))) * 0xFF;

        bool dontDraw = false;
        bool flatShading = true;
        const Texture* currentTexture;
        sf::Color flatColor(sf::Color::Transparent);
        switch (currRay.entityHit)
        {
        case EntityType::Wall:
            currentTexture = &tex.wallTexture;
            flatShading = false;
            break;
        case EntityType::Baudry:
            currentTexture = &tex.baundryTexture;
            flatShading = false;
            //flat shading example
            //flatShading = true;
            //flatColor = { 0xff, (currRay.lastSideChecked == CellSide::Hori) ? (sf::Uint8)0xff : (sf::Uint8)0x0, 0xff , boxShade };
            break;
        default:
            //dontDraw = true;
            break;
        }

        sf::Color currentColor;

        //--variables for texture navigation--
        //relative position in wall based on what face is being textured
        float posOnWallSide = 0;

        int textureU = 0;
        float textureV = 0;
        float texVStep = 0;

        //dont set up texture variables if not needed 
        if (!flatShading)
        {
            if (currRay.lastSideChecked == CellSide::Hori)
                posOnWallSide = currRay.hitPos.x + camTransform.coords.x;

            else
                posOnWallSide = currRay.hitPos.y + camTransform.coords.y;

            posOnWallSide -= std::floor(posOnWallSide);
            texVStep = currentTexture->height() / screenWallHeight;
            textureU = (int)(posOnWallSide * currentTexture->width());

            //if the wall is seen from the south or west side, its texture's u needs to be inverted 
            if (currRay.lastSideChecked == CellSide::Hori && currRay.hitPos.y < 0 ||
                currRay.lastSideChecked == CellSide::Vert && currRay.hitPos.x > 0)
                textureU = currentTexture->width() - textureU - 1;

            //if the textured box is bigger then the screen (hight wise), the initial unseen part of pixels must be skipped
            textureV = (screenWallHeight > g_screenHeight)
                ? texVStep * ((screenWallHeight - g_screenHeight) / 2)
                : 0;
        }

        int x = i * 4;
        //vertical scan line
        for (int y = 0; y < g_screenHeight * 4; y += 4)
        {
            //in the wall range
            if (y > floorHeight * 4 && y <= (g_screenHeight - floorHeight) * 4)
            {
                if (flatShading)
                {
                    view.m_pixels[y * g_screenWidth + x] = flatColor.r;
                    view.m_pixels[y * g_screenWidth + x + 1] = flatColor.g;
                    view.m_pixels[y * g_screenWidth + x + 2] = flatColor.b;
                    view.m_pixels[y * g_screenWidth + x + 3] = flatColor.a;
                }
                else
                {
                    //if the end of the texture is reached, start over
                    if (textureV >= currentTexture->height())
                        textureV = 0;

                    int viewPixel = (y * g_screenWidth + x);
                    int texturePixel = (textureU + int(textureV) * currentTexture->width()) * 4;

                    copy_pixels(view.m_pixels, currentTexture->m_texturePixels, viewPixel, texturePixel, boxShade);

                    textureV += texVStep;
                }
            }
            else if (!linear && y <= floorHeight * 4)
            {

                float rayLength = (g_screenHeight * graphVars.halfWallHeight) / (g_screenHeight - (0.5f * y));
                math::Vect2 xyPos = camTransform.coords + ((currRay.hitPos) / currRay.length) * rayLength;
                int uvPos[2]{};

                //ceiling
                uvPos[0] = std::abs((int)((xyPos.x - int(xyPos.x)) * tex.ceilingTexture.width()));
                uvPos[1] = std::abs((int)((xyPos.y - int(xyPos.y)) * tex.ceilingTexture.height()));

                int viewPixel = (y * g_screenWidth + x);
                int texturePixel = (uvPos[1] * tex.ceilingTexture.width() + uvPos[0]) * 4;

                copy_pixels(view.m_pixels, tex.ceilingTexture.m_texturePixels, viewPixel, texturePixel, 0xFF);

                //floor
                uvPos[0] = std::abs((int)((xyPos.x - int(xyPos.x)) * tex.floorTexture.width()));
                uvPos[1] = std::abs((int)((xyPos.y - int(xyPos.y)) * tex.floorTexture.height()));

                viewPixel = (g_screenHeight * 4 - y - 4) * g_screenWidth + x;
                texturePixel = (uvPos[1] * tex.floorTexture.width() + uvPos[0]) * 4;

                copy_pixels(view.m_pixels, tex.floorTexture.m_texturePixels, viewPixel, texturePixel, 0xFF);
            }
        }
    }
}

/*
//----------------background--

 void GameGraphics::draw_background_section(float startY, float endY, bool drawSky)
{
    //the algorithm operates by linear inerpolating between the left and rightmost rays cast by the camera to obtain world coordinates 
    //that are then translated into uv space. The lenght of the rays is calculated each scanline from the corresponding screen height.
    // 
    //ceiling vars
    math::Vect2 leftmostRayDir = m_gameGraphics.m_stateData.m_cameraDir - m_gameGraphics.m_stateData.m_cameraPlane / 2;
    math::Vect2 rightmostRayDir = m_gameGraphics.m_stateData.m_cameraDir + m_gameGraphics.m_stateData.m_cameraPlane / 2;

    //sky vars
    float skyUStart = -m_skyPixPerCircle * (m_gameGraphics.m_stateData.playerTransform.forewardAngle - m_gameGraphics.m_stateData.fov / 2);
    float skyVPos = m_skyVIncrement * startY;

    for (int y = startY; y < endY; ++y)
    {
        //inverse of the formula used in draw_view() to calculate wall height, witch is:
                    //  float screenWallHeight = (g_screenHeight / distance) * m_gameGraphics.m_halfWallHeight;
                    //  float floorHeight = (g_screenHeight - screenWallHeight) / 2;
        float rayLength = (g_screenHeight * m_gameGraphics.m_halfWallHeight) / (g_screenHeight - (2.f * y));

        //increment for interpolation
        math::Vect2 xyIncrement = ((rightmostRayDir - leftmostRayDir) * rayLength) / g_screenWidth;

        //start from the left, interpolete towards the right 
        math::Vect2 worldPosLeft = m_gameGraphics.m_stateData.playerTransform.coords + (leftmostRayDir * rayLength);
        math::Vect2 xyPos = worldPosLeft;
        int uvPos[2]{};

        Texture& sky = m_gameGraphics.m_staticTextures.skyTexture;
        Texture& ceiling = m_gameGraphics.m_staticTextures.ceilingTexture;
        Texture& floor = m_gameGraphics.m_staticTextures.floorTexture;
        GameView& view = m_gameGraphics.m_mainView;

        sf::Uint8 shading = 0xFF * (1 - (rayLength / m_gameGraphics.m_stateData.maxRenderDist));

        float skyUPos = skyUStart;

        for (int x = 0; x < g_screenWidth; ++x)
        {

            if (drawSky)
                //sky
            {

                if (skyUPos >= sky.width())
                    skyUPos -= sky.width();
                else if (skyUPos < 0)
                    skyUPos += sky.width();

                int viewPixel = (y * view.m_texture.getSize().x + x) * 4;
                int texturePixel = ((int)skyVPos * sky.width() + (int)skyUPos) * 4;

                copy_pixels(view.m_pixels, sky.m_texturePixels, viewPixel, texturePixel, 0xFF);
            }
            else
                //ceiling
            {
                uvPos[0] = std::abs((int)((xyPos.x - int(xyPos.x)) * ceiling.width()));
                uvPos[1] = std::abs((int)((xyPos.y - int(xyPos.y)) * ceiling.height()));

                int viewPixel = (y * g_screenWidth + x) * 4;
                int texturePixel = (uvPos[1] * ceiling.width() + uvPos[0]) * 4;

                copy_pixels(view.m_pixels, ceiling.m_texturePixels, viewPixel, texturePixel, shading);
            }

            //floor
            uvPos[0] = std::abs((int)((xyPos.x - int(xyPos.x)) * floor.width()));
            uvPos[1] = std::abs((int)((xyPos.y - int(xyPos.y)) * floor.height()));

            int viewPixel = (g_screenHeight - y - 1) * g_screenWidth * 4 + x * 4;
            int texturePixel = (uvPos[1] * floor.width() + uvPos[0]) * 4;

            copy_pixels(view.m_pixels, floor.m_texturePixels, viewPixel, texturePixel, shading);

            xyPos += xyIncrement;
            skyUPos += m_skyUIncrement;
        }
        skyVPos += m_skyVIncrement;
    }
}


//-------------------Sprites-----------

struct CompareBillboards
{
    bool operator()(const Billboard* firstB, const Billboard* secondB) {
        return firstB->distance < secondB->distance;
    }
};

void GameGraphics::draw_sprites()
{
    const std::vector<Billboard>& billboards = m_gameCore.get_billboards_info_arr();
    std::priority_queue<const Billboard*, std::vector<const Billboard*>, CompareBillboards> billbByDistMaxQ;

    //sort by distance (in order to use the painter's algorithm)
    for (const Billboard& b : billboards)
    {
        if (b.active && b.visible)
        {
            billbByDistMaxQ.push(&b);
        }
    }

    while(!billbByDistMaxQ.empty())
    {
        auto billb = billbByDistMaxQ.top();
        const Texture* spriteTex;
        try
        {
            spriteTex = m_spriteTexturesDict.at(billb->id).get();
        }
        catch (std::out_of_range& oor)
        {
            std::string err("Error: a sprite is trying to access a non existing texture of id: ");
            err.append(std::to_string(billb->id).append("\n").append(oor.what()));
            throw std::runtime_error(err);
        }
        
        draw_sprite_on_view(billb->distance, billb->positionOnScreen, *(spriteTex));
        billbByDistMaxQ.pop();
    }

    m_mainView.m_texture.update(m_mainView.m_pixels);
    m_window.draw(m_mainView.m_sprite);
}

void GameGraphics::draw_sprite_on_view(float distance, float centerPositionOnScreen, const Texture& spriteTex)
{

    if (distance < 0.02)
        return;
    //only once
    float screenSpriteHeight = (g_screenHeight / distance) * m_halfWallHeight;
    float floorHeight = (g_screenHeight - screenSpriteHeight) / 2;
    //

    float screenSpriteWidth = screenSpriteHeight * (spriteTex.width() / (float)spriteTex.height());

    sf::Uint8 shade = (1 - (distance / (m_stateData.maxRenderDist))) * 0xFF;

    float texVStep = spriteTex.height() / screenSpriteHeight;
    float texUStep = spriteTex.width() / screenSpriteWidth;

    float textureVstart = (screenSpriteHeight > g_screenHeight)
        ? texVStep * ((screenSpriteHeight - g_screenHeight) / 2)
        : 0;

    int screenU = (centerPositionOnScreen - screenSpriteWidth / 2);

    float textureU = 0;

    if (screenU < 0)
    {
        textureU = -screenU * texUStep;
        screenU = 0;
    }

    int screenUEnd = centerPositionOnScreen + screenSpriteHeight / 2;
    int screenVEnd = floorHeight + screenSpriteHeight;
    
    for (; screenU < screenUEnd && screenU < g_screenWidth; ++screenU )
    {
        if (distance < m_raysInfoVec.const_at( screenU ).length)
        {
            int screenV = (floorHeight) < 0 
                ? 0 
                : floorHeight;

            float textureV = textureVstart;

            for (; screenV < screenVEnd && screenV < g_screenHeight; ++screenV)
            {

                int uvPos[2]{};

                uvPos[0] = int(textureU);
                uvPos[1] = int(textureV);

                if (spriteTex.m_texturePixels[(uvPos[1] * spriteTex.width() + uvPos[0]) * 4 + 3] == 0xFF)
                {
                    int viewPixel = (screenV * g_screenWidth + screenU) * 4;
                    int texturePixel = (uvPos[1] * spriteTex.width() + uvPos[0]) * 4;

                    copy_pixels(m_mainView.m_pixels, spriteTex.m_texturePixels, viewPixel, texturePixel, 0xFF);
                }
                else
                {
                    //TODO semi transparency
                }

                textureV += texVStep;
            }
        }
        textureU += texUStep;
    }
}
*/
//----------------map-and-mini-map

void GameGraphics::draw_minimap_rays(int winPixWidth, const RayInfoArr& rays)
{
    sf::VertexArray lines(sf::Lines, winPixWidth * 2);
    for (int i = 0; i < winPixWidth; ++i)
    {
        lines[i * 2] = sf::Vector2f(m_minimapInfo.minimapCenterY,
                                    m_minimapInfo.minimapCenterX);

        lines[i * 2 + 1] = { {  m_minimapInfo.minimapCenterY + rays.const_at(i).hitPos.x * m_minimapInfo.minimapScale,
                                m_minimapInfo.minimapCenterX + rays.const_at(i).hitPos.y * m_minimapInfo.minimapScale  },
                                sf::Color::Cyan };
    }
    m_window.draw(lines);
}

void GameGraphics::draw_minimap_triangles(int winPixWidth, const RayInfoArr& rays, const GraphicsVars& graphVars)
{
    sf::VertexArray triangles(sf::TriangleFan, winPixWidth + 1);
    triangles[0] = sf::Vertex{ {(float)m_minimapInfo.minimapCenterY, 
                                (float)m_minimapInfo.minimapCenterX}, sf::Color::Red };

    for (int i = 0; i < winPixWidth; ++i)
    {
        triangles[i + 1] = { {  m_minimapInfo.minimapCenterY + rays.const_at(i).hitPos.x * m_minimapInfo.minimapScale,
                                m_minimapInfo.minimapCenterX + rays.const_at(i).hitPos.y * m_minimapInfo.minimapScale  },
                                sf::Color(0xFF, 0, 0, (1 - rays.const_at(i).length / graphVars.maxSightDepth) * 0xFF) };
    }
    m_window.draw(triangles);
}

void GameGraphics::draw_map(int mapWidth, int mapHeight, int posX, int posY, const std::string& cells)
{
    for (int y = 0; y < mapHeight; ++y)
    {
        for (int x = 0; x < mapWidth; ++x)
        {
            char currentCell = cells.at(y * mapWidth + x);

            if (currentCell != ' ')
            {
                if (currentCell == 'b')
                    m_mapSquareAsset.wallRect.setFillColor(sf::Color(0x99, 0x99,0x99));
                if (currentCell == 'w')
                    m_mapSquareAsset.wallRect.setFillColor(sf::Color::White);
                if (currentCell == 'g')
                    m_mapSquareAsset.wallRect.setFillColor(sf::Color::Red);

                m_mapSquareAsset.wallRect.setPosition({ (float)x * m_mapSquareAsset.tileDim + m_mapSquareAsset.xoffset, 
                                                        (float)y * m_mapSquareAsset.tileDim + m_mapSquareAsset.yoffset });
                m_window.draw(m_mapSquareAsset.wallRect);
            }
        }
    }

    sf::CircleShape playerC(m_mapSquareAsset.tileDim / 2.f);
    playerC.setFillColor(sf::Color::Red);
    playerC.setPosition({ (float)(posX * m_mapSquareAsset.tileDim + m_mapSquareAsset.xoffset),
                          (float)(posY * m_mapSquareAsset.tileDim + m_mapSquareAsset.yoffset) });

    m_window.draw(playerC);
}

void GameGraphics::draw_path_out()
{
    if (m_pathToGoal.empty())
        return;
    sf::VertexArray lines(sf::Lines, m_pathToGoal.size() * 2);

    lines[0] = { sf::Vector2f(  m_mapSquareAsset.xoffset + m_mapSquareAsset.tileDim * (m_pathToGoal.at(0).first + 0.5f),
                                m_mapSquareAsset.yoffset + m_mapSquareAsset.tileDim * (m_pathToGoal.at(0).second + 0.5f)), 
                                sf::Color::Magenta };

    lines[1] = { sf::Vector2f(  m_mapSquareAsset.xoffset + m_mapSquareAsset.tileDim * (m_pathToGoal.at(1).first + 0.5f),
                                m_mapSquareAsset.yoffset + m_mapSquareAsset.tileDim * (m_pathToGoal.at(1).second + 0.5f)),
                                sf::Color::Red };

    for (int i = 2; i < m_pathToGoal.size(); ++i)
    {
        lines[(i - 1) * 2] = lines[(i - 1) * 2 - 1];

        lines[(i - 1) * 2].color = sf::Color::Red;

        lines[(i - 1) * 2 + 1] = { sf::Vector2f(m_mapSquareAsset.xoffset + m_mapSquareAsset.tileDim * (m_pathToGoal.at(i).first + 0.5f),
                                                m_mapSquareAsset.yoffset + m_mapSquareAsset.tileDim * (m_pathToGoal.at(i).second + 0.5f)), 
                                                sf::Color::Red };
    }
    m_window.draw(lines);
}

void GameGraphics::draw_minimap_background(const GameMap& gameMap, const EntityTransform& cameraTransform, const GraphicsVars& graphVars)
{
    int tileDim = m_minimapInfo.minimapScale;
    int xoffset = m_minimapInfo.minimapCenterY;
    int yoffset = m_minimapInfo.minimapCenterX;

    //limit minimap to a square with sides length equal to double of the render distance 
    int startX = cameraTransform.coords.x - graphVars.maxSightDepth;

    //avoid going out of bounds
    if (startX < 0)
        startX = 0;

    int endX = cameraTransform.coords.x + graphVars.maxSightDepth;
    if (endX > gameMap.x)
        endX = gameMap.x;

    int startY = cameraTransform.coords.y - graphVars.maxSightDepth;
    if (startY < 0)
        startY = 0;

    int endY = cameraTransform.coords.y + graphVars.maxSightDepth;
    if (endY > gameMap.y)
        endY = gameMap.y;

    sf::RectangleShape wallRect({ (float)tileDim, (float)tileDim });

    for (int y = startY; y < endY; ++y)
    {
        for (int x = startX; x < endX; ++x)
        {
            char currentCell = gameMap.cells->at(y * gameMap.x + x);

            if (currentCell != ' ')
            {
                if (currentCell == 'b' || currentCell == 'w')
                    wallRect.setFillColor(sf::Color::White);

                else if (currentCell == 'g')
                    wallRect.setFillColor(sf::Color::Yellow);

                wallRect.setPosition({  tileDim * ((float)x - cameraTransform.coords.x) + xoffset,
                                        tileDim * ((float)y - cameraTransform.coords.y) + yoffset });
                m_window.draw(wallRect);
            }
        }
    }
}

//---------------IG-assets---

void GameGraphics::MapSquareAsset::create(int mapWidth, int mapHeight)
{
    int xoverw = g_screenWidth / mapWidth;
    int yoverh = g_screenHeight / mapHeight;
    tileDim = std::min(xoverw, yoverh);
    xoffset = (g_screenWidth - mapWidth * tileDim) / 2;
    yoffset = (g_screenHeight - mapHeight * tileDim) / 2;
    wallRect.setSize({ (float)tileDim, (float)tileDim });
}