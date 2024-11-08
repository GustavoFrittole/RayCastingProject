#include <math.h>
#include <stdexcept>
#include <queue>
#include <cassert>
#include "gameGraphics.hpp"

using namespace windowVars;
using namespace rcm;

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

//---------------------------THREAD-POOL-HELPERS---

//---view---

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
    return ViewRendSection(sectionStart, sectionStart + get_section(index), this);
}

//--background--

BackgroundRendSectionFactory::BackgroundRendSection::BackgroundRendSection(int start, int end, BackgroundRendSectionFactory* source) :
    IRenderingSection(start, end),
    m_source(source)
{}

void BackgroundRendSectionFactory::BackgroundRendSection::operator()() const
{
    if (m_source == nullptr)
        throw std::runtime_error("Render section called while no target was set.");

    //if the section is empty do nothing
    if (m_start == m_end)
        return;

    GameGraphics::draw_background_section(m_start, m_end, (m_source->m_state->drawSky), *(m_source->m_view), m_source->m_backgroundVars, *(m_source->m_graphVars), *(m_source->m_camera), *(m_source->m_staticTex));
}

void BackgroundRendSectionFactory::set_target(GameView* view, const StaticTextures* tex, const GameStateVars* state, const GraphicsVars* graphVars, GameCameraView* camera)
{
    m_view = view;
    m_staticTex = tex;
    m_state = state;
    m_graphVars = graphVars;
    m_camera = camera;
    m_backgroundVars.skyPixPerCircle = (m_staticTex->skyTexture.width() / (float)(2 * PI));
    m_backgroundVars.skyUIncrement = m_backgroundVars.skyPixPerCircle * m_camera->vars.fov / g_windowWidth;
    m_backgroundVars.skyVIncrement = m_staticTex->skyTexture.height() / ((float)g_windowHeight / 2);
}

BackgroundRendSectionFactory::BackgroundRendSection BackgroundRendSectionFactory::create_section(int index)
{
    int sectionStart = m_sectionSize * index;
    return BackgroundRendSection(sectionStart, sectionStart + get_section(index), this);
}

//---sprite---

SpriteRendSectionFactory::SpriteRendSection::SpriteRendSection(int start, int end, SpriteRendSectionFactory* source) :
    IRenderingSection(start, end),
    m_source(source)
{}

void SpriteRendSectionFactory::SpriteRendSection::operator()() const
{
    //Render section functor must be used after a target is set.
    assert(m_source != nullptr);

    //if the section is empty do nothing
    if (m_start == m_end)
        return;

    GameGraphics::draw_sprite_section(m_start, m_end, *(m_source->m_view), m_source->m_sVars, *(m_source->m_billboard), *(m_source->m_tex), *(m_source->m_graphVars), *(m_source->m_rays));
}

void SpriteRendSectionFactory::set_environment(GameView* view, const GraphicsVars* graphVars, const RayInfoArr* rays)
{
    m_view = view;
    m_graphVars = graphVars;
    m_rays = rays;
}

void SpriteRendSectionFactory::set_target(const Billboard& billboard, const Texture& billTex)
{
    m_billboard = &billboard;
    m_tex = &billTex;

    //set sprite dimensions on screen
    float wallHeight = (g_windowHeight / billboard.distance) * m_graphVars->halfWallHeight;
    m_sVars.screenSpriteHeight = wallHeight * billboard.size;
    m_sVars.screenSpriteWidth = m_sVars.screenSpriteHeight * (billTex.width() / (float)billTex.height());

    //from where to start drawing the sprite (sprites are drawn top to bottom)
    switch (billboard.alignment)
    {
    case SpriteAlignment::TopWindow : 
        m_sVars.floorHeight = 0;
        break;
    case SpriteAlignment::Ceiling :
        m_sVars.floorHeight = (g_windowHeight - wallHeight) / 2;
        break;
    case SpriteAlignment::Center:
        m_sVars.floorHeight = (g_windowHeight - m_sVars.screenSpriteHeight) / 2;
            break;
    case SpriteAlignment::Floor:
        m_sVars.floorHeight =
            m_sVars.floorHeight = (g_windowHeight + wallHeight) / 2 - m_sVars.screenSpriteHeight;
            break;
    case SpriteAlignment::BottomWindow:
        m_sVars.floorHeight = g_windowHeight - m_sVars.screenSpriteHeight;
            break;
    }
    

    m_sVars.shade = (1 - (billboard.distance / (m_graphVars->maxSightDepth))) * 0xFF;

    //set texture reading steps
    m_sVars.texVStep = billTex.height() / m_sVars.screenSpriteHeight;
    m_sVars.texUStep = billTex.width() / m_sVars.screenSpriteWidth;

    //set texture start and end in V dimension
    m_sVars.textureVstart = (m_sVars.floorHeight < 0)
        ? m_sVars.texVStep * (-m_sVars.floorHeight)
        : 0;
    m_sVars.screenVEnd = m_sVars.floorHeight + m_sVars.screenSpriteHeight;

    //set number of tasks based on width
    set_task_number(m_sVars.screenSpriteWidth);
}

SpriteRendSectionFactory::SpriteRendSection SpriteRendSectionFactory::create_section(int index)
{
    int sectionStart = m_sectionSize * index;
    return SpriteRendSection(sectionStart, sectionStart + get_section(index), this);
}

//---------------------------GAME-GRAPHICS---

GameGraphics::GameGraphics(sf::RenderWindow& window, const GraphicsVars& graphicsVars) :
    m_window(window),
    m_pathToGoal(0),
    m_minimapInfo(graphicsVars.minimapScale, graphicsVars.maxSightDepth),
    m_rendThreadPool(utils::get_thread_number()),
    m_viewSecFactory(g_windowWidth, m_rendThreadPool.get_size()),
    m_backgroundSecFactory(g_windowHeight/2, m_rendThreadPool.get_size()),
    m_spriteSecFactory(g_windowWidth, m_rendThreadPool.get_size())
{
    m_window.clear(sf::Color::Black);
    m_window.setFramerateLimit(graphicsVars.frameRate);
}

//-----compound-funtions--

void GameGraphics::create_assets(const GameAssets& gameAssets, const GameMap& gameMap, const GraphicsVars& graphicsVars, const RayInfoArr& raysInfoVec, const GameStateVars& gameState, GameCameraView& gameCamera)
{
    m_mainView.create(g_windowWidth, g_windowHeight, true);
    m_pathFinder = std::make_unique<PathFinder>(gameMap.width, gameMap.height, *(gameMap.cells), m_pathToGoal);
    m_mapSquareAsset.create(gameMap.width, gameMap.height);

    load_text_ui(gameAssets.fontFilePath);
    load_textures(gameAssets);

    m_viewSecFactory.set_target(&raysInfoVec, &m_mainView, &m_staticTextures, &gameState, &graphicsVars, &gameCamera.transform);
    m_backgroundSecFactory.set_target(&m_mainView, &m_staticTextures, &gameState, &graphicsVars, &gameCamera);
    m_spriteSecFactory.set_environment(&m_mainView, &graphicsVars, &raysInfoVec);

    create_background_sections();
    create_view_sections();
    create_sprite_sections();
}

void GameGraphics::load_textures(const GameAssets& gameAssets)
{
    m_staticTextures.wallTexture.create(gameAssets.wallTexFilePath);
    m_staticTextures.baundryTexture.create(gameAssets.boundryTexFilePath);
    m_staticTextures.floorTexture.create(gameAssets.floorTexFilePath);
    m_staticTextures.ceilingTexture.create(gameAssets.ceilingTexFilePath);
    m_staticTextures.skyTexture.create(gameAssets.skyTexFilePath);
}

void GameGraphics::draw_view(bool linearPersp, const std::vector<std::unique_ptr<IEntity>>& entities)
{
    render_view(linearPersp);

    render_sprites(entities);

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
    m_pathFinder->find_path(startPos.coordinates.x, startPos.coordinates.y);
}

void GameGraphics::load_sprite(int id, const std::string& texturePath)
{
    m_spriteTexturesDict[id] = std::make_unique<Texture>(texturePath);
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

void GameGraphics::load_text_ui(const std::string& path)
{
    if (m_gameFont.loadFromFile(path))
    {
        m_gameText.setFont(m_gameFont);
    }
    m_gameText.setString("GOAL REACHED");
    m_gameText.setCharacterSize(50);
    m_gameText.setFillColor(sf::Color::Red);
}

void GameGraphics::set_text_ui(const std::string& text, const TextVerticalAlignment vertAlign, const TextHorizontalAlignment horiAlign, const int size, const int offsetX, const int offsetY)
{
    if (size > 0)
        m_gameText.setCharacterSize(size);
        //m_gameText.setScale(size/10.f,size/10.f);

    m_gameText.setString(text);
    math::Vect2 textPos{};
    switch (vertAlign)
    {
    case TextVerticalAlignment::BottomWindow:
        textPos.y = g_windowHeight - m_gameText.getLocalBounds().height*2 + offsetY;
        break;
    case TextVerticalAlignment::Center:
        textPos.y = (g_windowHeight - m_gameText.getLocalBounds().height) / 2 + offsetY;
        break;
    case TextVerticalAlignment::TopWindow:
        textPos.y = 0 + offsetY;
        break;
    default:
        break;
    }
    switch (horiAlign)
    {
    case TextHorizontalAlignment::Left:
        textPos.x = 0 + offsetX;
        break;
    case TextHorizontalAlignment::Center:
        textPos.x = (g_windowWidth - m_gameText.getLocalBounds().width) / 2 + offsetX;;
        break;
    case TextHorizontalAlignment::Right:
        textPos.x = g_windowWidth - m_gameText.getLocalBounds().width + offsetX;;
        break;
    default:
        break;
    }
    m_gameText.setPosition( textPos.x, textPos.y );
}

void GameGraphics::draw_text_ui()
{
    m_window.draw(m_gameText);
}

void GameGraphics::create_view_sections()
{
    for (int i = 0; i < m_viewSecFactory.get_size(); ++i)
        m_viewSectionsVec.push_back(m_viewSecFactory.create_section(i));
}

void GameGraphics::create_background_sections()
{
    for (int i = 0; i < m_backgroundSecFactory.get_size(); ++i)
        m_backgroundSectionsVec.push_back(m_backgroundSecFactory.create_section(i));
}

void GameGraphics::create_sprite_sections()
{
    m_spriteSectionsVec.resize(m_spriteSecFactory.get_size());
}

void GameGraphics::update_sprite_sections()
{
    for (int i = 0; i < m_spriteSecFactory.get_size(); ++i)
        m_spriteSectionsVec.at(i) = m_spriteSecFactory.create_section(i);
}

void GameGraphics::render_view(bool linearPersp)
{
    int lastSection = 0;

    //----background-----

    if (linearPersp)
    {
        lastSection = m_backgroundSecFactory.get_size() - 1;
        m_rendThreadPool.new_batch(m_backgroundSecFactory.get_size() - 1);

        for (int i = 0; i < lastSection; ++i)
        {
            m_rendThreadPool.enqueue(&m_backgroundSectionsVec.at(i));
        }
        {
            BackgroundRendSectionFactory::BackgroundRendSection lastSec(m_backgroundSecFactory.create_section(lastSection));
            lastSec();
        }
        while (m_rendThreadPool.is_busy())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(0));
        }
    }

    //-----main-view-----

    lastSection = m_viewSecFactory.get_size() - 1;
    m_rendThreadPool.new_batch(m_viewSecFactory.get_size() - 1);
    for (int i = 0; i < lastSection; ++i)
    {
        m_rendThreadPool.enqueue(&m_viewSectionsVec.at(i));
    }
    {
        ViewRendSectionFactory::ViewRendSection lastSec(m_viewSecFactory.create_section(lastSection));
        lastSec();
    }
    while (m_rendThreadPool.is_busy())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(0));
    }
}



void GameGraphics::draw_view_section(int startY, int endY, bool linear, const RayInfoArr& rays, GameView& view, const GraphicsVars& graphVars, const StaticTextures& tex, const EntityTransform& camTransform)
{
    for (int i = startY; i < endY; ++i)
    {
        const RayInfo& currRay = rays.const_at(i);
        float distance = currRay.length;

        //--this code version maintains correct proportions, but causes texture distortion (not compatible with other parts of the code, which use the inverse function)--
        //float wallAngle = (std::atan(m_gameGraphics.m_halfWallHeight / distance) );
        //float screenWallHeight = g_windowHeight  * wallAngle / (m_verticalVisibleAngle * m_gameGraphics.m_halfWallHeight);

        //--this version is faster, has easy texture mapping but locks the vertical view angle at 90 deg (45 deg up 45 deg down)--
        float screenWallHeight = (g_windowHeight / distance) * graphVars.halfWallHeight;
        float floorHeight = (g_windowHeight - screenWallHeight) / 2;

        //alpha of walls, bleanding them with the dark backgroud 
        sf::Uint8 boxShade = (1 - (distance / (graphVars.maxSightDepth))) * 0xFF;

        bool dontDraw = false;
        bool flatShading = true;
        const Texture* currentTexture;
        sf::Color flatColor(sf::Color::Transparent);
        switch (currRay.entityHit)
        {
        case HitType::Wall:
            currentTexture = &tex.wallTexture;
            flatShading = false;
            break;
        case HitType::Baudry:
            currentTexture = &tex.baundryTexture;
            flatShading = false;
            //-------flat shading example-------
            //flatShading = true;
            //flatColor = { 0xff, (currRay.lastSideChecked == CellSide::Hori) ? (sf::Uint8)0xff : (sf::Uint8)0x0, 0xff , boxShade };
            break;
        case HitType::Nothing:
            //this occurs when sight is clear up to a distance equivalent to graphVars.maxSightDepth
            //viable options might be to draw nothing or to draw a black wall 
            //here nothing is drawn (works better with linear persp)
            screenWallHeight = 0;
            floorHeight = g_windowHeight / 2;
            break;
        default:

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
                posOnWallSide = currRay.hitPos.x + camTransform.coordinates.x;

            else
                posOnWallSide = currRay.hitPos.y + camTransform.coordinates.y;

            posOnWallSide -= std::floor(posOnWallSide);
            texVStep = currentTexture->height() / screenWallHeight;
            textureU = (int)(posOnWallSide * currentTexture->width());

            //if the wall is seen from the south or west side, its texture's u needs to be inverted 
            if (currRay.lastSideChecked == CellSide::Hori && currRay.hitPos.y < 0 ||
                currRay.lastSideChecked == CellSide::Vert && currRay.hitPos.x > 0)
                textureU = currentTexture->width() - textureU - 1;

            //if the textured box is bigger then the screen (hight wise), the initial unseen part of pixels must be skipped
            textureV = (screenWallHeight > g_windowHeight)
                ? texVStep * ((screenWallHeight - g_windowHeight) / 2)
                : 0;
        }

        int x = i * 4;
        //vertical scan line
        for (int y = 0; y < g_windowHeight * 4; y += 4)
        {
            //in the wall range
            if (y > floorHeight * 4 && y <= (g_windowHeight - floorHeight) * 4)
            {
                if (flatShading)
                {
                    view.m_pixels[y * g_windowWidth + x] = flatColor.r;
                    view.m_pixels[y * g_windowWidth + x + 1] = flatColor.g;
                    view.m_pixels[y * g_windowWidth + x + 2] = flatColor.b;
                    view.m_pixels[y * g_windowWidth + x + 3] = flatColor.a;
                }
                else
                {
                    //if the end of the texture is reached, start over
                    if (textureV >= currentTexture->height())
                        textureV = 0;

                    int viewPixel = (y * g_windowWidth + x);
                    int texturePixel = (textureU + int(textureV) * currentTexture->width()) * 4;

                    if(!linear || !(currentTexture->m_texturePixels[texturePixel + 3] == 0))
                        copy_pixels(view.m_pixels, currentTexture->m_texturePixels, viewPixel, texturePixel, boxShade);

                    textureV += texVStep;
                }
            }
            else if (!linear && y <= floorHeight * 4)
            {

                float rayLength = (g_windowHeight * graphVars.halfWallHeight) / (g_windowHeight - (0.5f * y));
                math::Vect2 xyPos = camTransform.coordinates + ((currRay.hitPos) / currRay.length) * rayLength;
                int uvPos[2]{};

                //ceiling
                uvPos[0] = std::abs((int)((xyPos.x - int(xyPos.x)) * tex.ceilingTexture.width()));
                uvPos[1] = std::abs((int)((xyPos.y - int(xyPos.y)) * tex.ceilingTexture.height()));

                int viewPixel = (y * g_windowWidth + x);
                int texturePixel = (uvPos[1] * tex.ceilingTexture.width() + uvPos[0]) * 4;

                copy_pixels(view.m_pixels, tex.ceilingTexture.m_texturePixels, viewPixel, texturePixel, 0xFF);

                //floor
                uvPos[0] = std::abs((int)((xyPos.x - int(xyPos.x)) * tex.floorTexture.width()));
                uvPos[1] = std::abs((int)((xyPos.y - int(xyPos.y)) * tex.floorTexture.height()));

                viewPixel = (g_windowHeight * 4 - y - 4) * g_windowWidth + x;
                texturePixel = (uvPos[1] * tex.floorTexture.width() + uvPos[0]) * 4;

                copy_pixels(view.m_pixels, tex.floorTexture.m_texturePixels, viewPixel, texturePixel, 0xFF);
            }
        }
    }
}


//----------------background--

 void GameGraphics::draw_background_section(int startX, int endX, bool drawSky, GameView& view, const BackgroundVars& bgVars, const GraphicsVars& windowVars, GameCameraView& camera, const StaticTextures& tex)
{
    //the algorithm operates by linear inerpolating between the left and rightmost rays cast by the camera to obtain world coordinates 
    //that are then translated into uv space. The lenght of the rays is calculated each scanline from the corresponding screen height.
    // 
    //ceiling vars
    math::Vect2 leftmostRayDir = camera.vecs.forewardDirection - camera.vecs.plane / 2;
    math::Vect2 rightmostRayDir = camera.vecs.forewardDirection + camera.vecs.plane / 2;

    //sky vars
    float skyUStart = -bgVars.skyPixPerCircle * (camera.transform.forewardAngle - camera.vars.fov / 2);
    float skyVPos = bgVars.skyVIncrement * startX;

    for (int y = startX; y < endX; ++y)
    {
        //inverse of the formula used in draw_view() to calculate wall height, witch is:
                    //  float screenWallHeight = (g_windowHeight / distance) * m_gameGraphics.m_halfWallHeight;
                    //  float floorHeight = (g_windowHeight - screenWallHeight) / 2;
        float rayLength = (g_windowHeight * windowVars.halfWallHeight) / (g_windowHeight - (2.f * y));

        //increment for interpolation
        math::Vect2 xyIncrement = ((rightmostRayDir - leftmostRayDir) * rayLength) / g_windowWidth;

        //start from the left, interpolete towards the right 
        math::Vect2 worldPosLeft = camera.transform.coordinates + (leftmostRayDir * rayLength);
        math::Vect2 xyPos = worldPosLeft;
        int uvPos[2]{};

        sf::Uint8 shading = 0x0;

        if (rayLength <= windowVars.maxSightDepth)
            shading = 0xFF * (1 - (rayLength / windowVars.maxSightDepth));

        float skyUPos = skyUStart;

        for (int x = 0; x < g_windowWidth; ++x)
        {
            if (drawSky)
                //sky
            {

                if (skyUPos >= tex.skyTexture.width())
                    skyUPos -= tex.skyTexture.width();
                else if (skyUPos < 0)
                    skyUPos += tex.skyTexture.width();

                int viewPixel = (y * view.m_texture.getSize().x + x) * 4;
                int texturePixel = ((int)skyVPos * tex.skyTexture.width() + (int)skyUPos) * 4;

                copy_pixels(view.m_pixels, tex.skyTexture.m_texturePixels, viewPixel, texturePixel, 0xFF);
            }
            else
                //ceiling
            {
                uvPos[0] = std::abs((int)((xyPos.x - int(xyPos.x)) * tex.ceilingTexture.width()));
                uvPos[1] = std::abs((int)((xyPos.y - int(xyPos.y)) * tex.ceilingTexture.height()));

                int viewPixel = (y * g_windowWidth + x) * 4;
                int texturePixel = (uvPos[1] * tex.ceilingTexture.width() + uvPos[0]) * 4;

                copy_pixels(view.m_pixels, tex.ceilingTexture.m_texturePixels, viewPixel, texturePixel, shading);
            }

            //floor
            uvPos[0] = std::abs((int)((xyPos.x - int(xyPos.x)) * tex.floorTexture.width()));
            uvPos[1] = std::abs((int)((xyPos.y - int(xyPos.y)) * tex.floorTexture.height()));

            int viewPixel = (g_windowHeight - y - 1) * g_windowWidth * 4 + x * 4;
            int texturePixel = (uvPos[1] * tex.floorTexture.width() + uvPos[0]) * 4;

            copy_pixels(view.m_pixels, tex.floorTexture.m_texturePixels, viewPixel, texturePixel, shading);

            xyPos += xyIncrement;
            skyUPos += bgVars.skyUIncrement;
        }
        skyVPos += bgVars.skyVIncrement;
    }
}

 
//-------------------Sprites-----------

struct CompareBillboards
{
    bool operator()(const Billboard* firstB, const Billboard* secondB) {
        return firstB->distance < secondB->distance;
    }
};

void GameGraphics::render_sprites(const std::vector<std::unique_ptr<IEntity>>& entities)
{
    std::priority_queue<const Billboard*, std::vector<const Billboard*>, CompareBillboards> billbByDistMaxQ;

    //sort by distance (in order to use the painter's algorithm)
    for (const std::unique_ptr<IEntity>& entity : entities)
    {
        if (entity->m_visible && entity->m_billboard.distance > 0.2f)
        {
            billbByDistMaxQ.push(&(entity->m_billboard));
        }
    }

    while(!billbByDistMaxQ.empty())
    {
        const Billboard* billb = billbByDistMaxQ.top();
        const Texture* spriteTex = nullptr;

        int currentId = -1;

        if (billb->hasTurnAroundSprites)
        {
            if (billb->cameraAngle <= -(PI * 5 / 6))
                currentId = billb->id;

            else if (billb->cameraAngle <= -(PI / 2))
                currentId = billb->turnAroundTexIds.nw;

            else if (billb->cameraAngle <= -(PI / 6))
                currentId = billb->turnAroundTexIds.sw;

            else if (billb->cameraAngle <= PI / 6)
                 currentId = billb->turnAroundTexIds.s;

            else if (billb->cameraAngle <= PI / 2)
                currentId = billb->turnAroundTexIds.se;
                

            else if (billb->cameraAngle <= PI * 5 / 6)
                currentId = billb->turnAroundTexIds.ne;
                

            else
                currentId = billb->id;
        }
        else
        {
            currentId = billb->id;
        }

        if (currentId != -1)
        {
            spriteTex = m_spriteTexturesDict[currentId].get();

            if (spriteTex == nullptr)
            {
                std::string err("Error: a sprite is trying to access a non existing texture of id: ");
                err.append(std::to_string(currentId).append("\n"));
                throw std::runtime_error(err);
            }

            m_spriteSecFactory.set_target(*billb, *spriteTex);
            update_sprite_sections();
            render_sprite();
        }

        billbByDistMaxQ.pop();
    }

    m_mainView.m_texture.update(m_mainView.m_pixels);
    m_window.draw(m_mainView.m_sprite);
}

void GameGraphics::render_sprite()
{
    int lastSection = m_spriteSecFactory.get_size() - 1;
    m_rendThreadPool.new_batch(m_spriteSecFactory.get_size() - 1);

    for (int i = 0; i < lastSection; ++i)
    {
        m_rendThreadPool.enqueue(&m_spriteSectionsVec.at(i));
    }
    {
        SpriteRendSectionFactory::SpriteRendSection lastSec(m_spriteSecFactory.create_section(lastSection));
        lastSec();
    }
    while (m_rendThreadPool.is_busy())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(0));
    }
}

void GameGraphics::draw_sprite_section(int startU, int endU, GameView& view, const SpriteRendVars& vars, const Billboard& billboard, const Texture& spriteTex, const GraphicsVars& graphicsVars, const RayInfoArr& rays)
{
    int screenU = startU + (billboard.positionOnScreen - vars.screenSpriteWidth / 2);

    float textureU = startU * vars.texUStep;
    if (screenU < 0)
    {
        textureU -= screenU * vars.texUStep;
        screenU = 0;
    }

    int screenUEnd = endU + (billboard.positionOnScreen - vars.screenSpriteWidth / 2);
    
    for (; screenU < screenUEnd && screenU < g_windowWidth; ++screenU )
    {
        if (billboard.distance < rays.const_at( screenU ).length)
        {
            int screenV = (vars.floorHeight) < 0
                ? 0 
                : vars.floorHeight;

            float textureV = vars.textureVstart;

            for (; screenV < vars.screenVEnd && screenV < g_windowHeight; ++screenV)
            {

                int uvPos[2]{};

                uvPos[0] = int(textureU);
                uvPos[1] = int(textureV);

                if (spriteTex.m_texturePixels[(uvPos[1] * spriteTex.width() + uvPos[0]) * 4 + 3] == 0xFF)
                {
                    int viewPixel = (screenV * g_windowWidth + screenU) * 4;
                    int texturePixel = (uvPos[1] * spriteTex.width() + uvPos[0]) * 4;

                    copy_pixels(view.m_pixels, spriteTex.m_texturePixels, viewPixel, texturePixel, 0xFF);
                }
                else
                {
                    //TODO semi transparency
                }

                textureV += vars.texVStep;
            }
        }
        textureU += vars.texUStep;
    }
}

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

void GameGraphics::draw_minimap_background(const GameMap& gameMap, const EntityTransform& transform, const GraphicsVars& graphVars)
{
    int tileDim = m_minimapInfo.minimapScale;
    int xoffset = m_minimapInfo.minimapCenterY;
    int yoffset = m_minimapInfo.minimapCenterX;

    //limit minimap to a square with sides length equal to double of the render distance 
    int startX = transform.coordinates.x - graphVars.maxSightDepth;

    //avoid drawing out of bounds cells
    if (startX < 0)
        startX = 0;

    int endX = transform.coordinates.x + graphVars.maxSightDepth;
    if (endX > gameMap.width)
        endX = gameMap.width;

    int startY = transform.coordinates.y - graphVars.maxSightDepth;
    if (startY < 0)
        startY = 0;

    int endY = transform.coordinates.y + graphVars.maxSightDepth;
    if (endY > gameMap.height)
        endY = gameMap.height;

    sf::RectangleShape wallRect({ (float)tileDim, (float)tileDim });

    for (int y = startY; y < endY; ++y)
    {
        for (int x = startX; x < endX; ++x)
        {
            char currentCell = gameMap.cells->at(y * gameMap.width + x);

            if (currentCell != ' ')
            {
                if (currentCell == 'b' || currentCell == 'w')
                    wallRect.setFillColor(sf::Color::White);

                else if (currentCell == 'g')
                    wallRect.setFillColor(sf::Color::Yellow);

                wallRect.setPosition({  tileDim * ((float)x - transform.coordinates.x) + xoffset,
                                        tileDim * ((float)y - transform.coordinates.y) + yoffset });
                m_window.draw(wallRect);
            }
        }
    }
}

//---------------IG-assets---

void MapSquareAsset::create(int mapWidth, int mapHeight)
{
    int xoverw = g_windowWidth / mapWidth;
    int yoverh = g_windowHeight / mapHeight;
    tileDim = std::min(xoverw, yoverh);
    xoffset = (g_windowWidth - mapWidth * tileDim) / 2;
    yoffset = (g_windowHeight - mapHeight * tileDim) / 2;
    wallRect.setSize({ (float)tileDim, (float)tileDim });
}