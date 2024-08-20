#ifndef GAMEGRAPHICS_HPP
#define GAMEGRAPHICS_HPP

#include <SFML/Graphics.hpp>
#include <thread>
#include "pathFinder.hpp"
#include "rendThreadPool.hpp"
#include "gameDataStructures.hpp"

struct MinimapInfo
{
    MinimapInfo(int scaleToScreen, float rayLength) : minimapRelPos(scaleToScreen),
        minimapCenterX(windowVars::g_screenHeight / minimapRelPos),
        minimapCenterY(windowVars::g_screenWidth - minimapCenterX),
        minimapScale(minimapCenterX / rayLength) 
    {}
    int minimapRelPos;
    int minimapCenterX;
    int minimapCenterY;
    int minimapScale;
};

struct GameView
{
    GameView() = default;
    GameView(int, int, bool);
    GameView(const GameView&) = delete;
    ~GameView() { if (m_hasPixelArray) delete[] m_pixels; }
    GameView& operator=(const GameView&) = delete;

    void create(int, int, bool);
    
    sf::Texture m_texture;
    sf::Sprite m_sprite;
    sf::Uint8* m_pixels = nullptr;
private:
    bool m_hasPixelArray = false;
};

struct Texture
{
public:
    Texture() = default;
    Texture(const std::string& filePath) { create(filePath); }
    void create(const std::string&);
    const sf::Uint8& get_pixel_at(int) const;
    int width() const { return m_image.getSize().x; }
    int height() const { return m_image.getSize().y; }
    const sf::Uint8* m_texturePixels = nullptr;
private:
    sf::Image m_image;
    int m_width = 0;
    int m_height = 0;
};

struct StaticTextures
{
    Texture wallTexture;
    Texture baundryTexture;
    Texture floorTexture;
    Texture ceilingTexture;
    Texture skyTexture;
};

//------------------THREAD-POOL---

struct BackgroundVars
{
    float skyPixPerCircle = 0.f;
    float skyUIncrement = 0.f;
    float skyVIncrement = 0.f;
};

struct SpriteRendVars
{
    float screenSpriteHeight = 0.f;
    float floorHeight = 0.f;
    float screenSpriteWidth = 0;
    sf::Uint8 shade = 0;
    float texVStep = 0;
    float texUStep = 0;
    float textureVstart = 0;
    int screenVEnd = 0;
};

//------main-view----
class ViewRendSectionFactory : public IRenderingSectionFactory
{
public:
    class ViewRendSection : public IRenderingSection
    {
    public:
        ViewRendSection(int, int, ViewRendSectionFactory*);
        void operator()() const override;
    protected:
        ViewRendSectionFactory* m_source = nullptr;
    };

    ViewRendSectionFactory(int taskNumber, int workers) : 
        IRenderingSectionFactory(taskNumber, workers) {}

    ViewRendSection create_section(int index);
    void set_target(const RayInfoArr* rays, GameView* view, const StaticTextures* tex, const GameStateVars* state, const GraphicsVars* graphVars, const EntityTransform* camTransform);

protected:
    const RayInfoArr* m_rays = nullptr;
    GameView* m_view = nullptr;
    const StaticTextures* m_staticTex = nullptr;
    const GameStateVars* m_state = nullptr;
    const GraphicsVars* m_graphVars = nullptr;
    const EntityTransform* m_camTransform = nullptr;
};

//----background----

class BackgroundRendSectionFactory : public IRenderingSectionFactory
{
public:
    class BackgroundRendSection : public IRenderingSection
    {
    public:
        BackgroundRendSection(int, int, BackgroundRendSectionFactory*);
        void operator()() const override;
    protected:
        BackgroundRendSectionFactory* m_source = nullptr;
    };

    BackgroundRendSectionFactory(int taskNumber, int workers) :
        IRenderingSectionFactory(taskNumber, workers) {}

    BackgroundRendSection create_section(int index);
    void set_target(GameView*, const StaticTextures*, const GameStateVars*, const GraphicsVars*, GameCameraView*);
    
protected:
    GameView* m_view = nullptr;
    const StaticTextures* m_staticTex = nullptr;
    GameCameraView* m_camera = nullptr;
    const GameStateVars* m_state = nullptr;
    const GraphicsVars* m_graphVars = nullptr;
    BackgroundVars m_backgroundVars{};
};

//-----sprites----

class SpriteRendSectionFactory : public IRenderingSectionFactory
{
public:
    class SpriteRendSection : public IRenderingSection
    {
    public:
        SpriteRendSection() : IRenderingSection() {}
        SpriteRendSection(int, int, SpriteRendSectionFactory*);
        void operator()() const override;
    protected:
        SpriteRendSectionFactory* m_source = nullptr;
    };

    SpriteRendSectionFactory(int taskNumber, int workers) :
        IRenderingSectionFactory(taskNumber, workers) {}

    SpriteRendSection create_section(int index);
    void set_environment(GameView*, const GraphicsVars*, const RayInfoArr*);
    void set_target(const Billboard&, const Texture&);

protected:
    const Billboard* m_billboard = nullptr;
    GameView* m_view = nullptr;
    const GraphicsVars* m_graphVars = nullptr;
    const RayInfoArr* m_rays = nullptr;
    const Texture* m_tex = nullptr;
    SpriteRendVars m_sVars{};
};


class GameGraphics
{
public:
    GameGraphics(sf::RenderWindow& window, const GraphicsVars& graphicsVars, int frameRate = 0);
    GameGraphics() = delete;
    GameGraphics& operator=(const GameGraphics&) = delete;

    void create_assets(const GameAssets&, const GameMap&, const GraphicsVars&, const RayInfoArr&, const GameStateVars&, GameCameraView&);
    void load_sprite(const std::string&);
    bool is_running() const { return m_window.isOpen(); }
    //no trasfer
    bool goal_reached(const EntityTransform& pos, const GameMap& map);

    void draw_map_gen(int mapWidth, int mapHeight, int posX, int posY, const std::string& cells);
    void draw_minimap_triangles(int winPixWidth, const RayInfoArr& rays, const GraphicsVars& graphVars);
    void draw_minimap_rays(int winPixWidth, const RayInfoArr& rays);
    void draw_minimap_background(const GameMap& gameMap, const EntityTransform& transform, const GraphicsVars& graphVars);
    void draw_map(int mapWidth, int mapHeight, int posX, int posY, const std::string& cells);
    void draw_end_screen();
    void draw_path_out();
    void draw_view(bool, const std::vector<Billboard>&);
    void calculate_shortest_path(const EntityTransform&);

    void static draw_view_section(int startY, int endY, bool linear, const RayInfoArr&, GameView&, const GraphicsVars&, const StaticTextures&, const EntityTransform&);
    void static draw_background_section(int startX, int endX, bool drawSky, GameView&, const BackgroundVars&, const GraphicsVars&, GameCameraView&, const StaticTextures&);
    void static draw_sprite_section(int startU, int endU, GameView&, const SpriteRendVars&, const Billboard&, const Texture&, const GraphicsVars&, const RayInfoArr&);

private:
    struct MapSquareAsset
    {
        void create(int, int);
        int tileDim = 0;
        int xoffset = 0;
        int yoffset = 0;
        sf::RectangleShape wallRect;
    };

    sf::RenderWindow& m_window;

    std::vector<std::pair<int, int>> m_pathToGoal;
    std::unique_ptr<PathFinder> m_pathFinder;
    const MinimapInfo m_minimapInfo;

    GameView m_mainView;
    sf::Text m_endGameText;
    sf::Font m_endGameFont;
    MapSquareAsset m_mapSquareAsset;
    StaticTextures m_staticTextures;
    std::vector<std::unique_ptr<Texture>> m_spriteTexturesDict;

    void draw_camera_view();

    inline void load_end_screen();
    inline void load_textures(const GameAssets&);

    RendThreadPool m_rendThreadPool;

    ViewRendSectionFactory m_viewSecFactory;
    std::vector<ViewRendSectionFactory::ViewRendSection> m_viewSectionsVec;
    void create_view_sections();

    BackgroundRendSectionFactory m_backgroundSecFactory;
    std::vector<BackgroundRendSectionFactory::BackgroundRendSection> m_backgroundSectionsVec;
    void create_background_sections();

    SpriteRendSectionFactory m_spriteSecFactory;
    std::vector<SpriteRendSectionFactory::SpriteRendSection> m_spriteSectionsVec;
    void create_sprite_sections();
    void update_sprite_sections();

    void render_view(bool);
    void render_sprites(const std::vector<Billboard>&);
    void render_sprite();
};

inline void copy_pixels( sf::Uint8 *, const sf::Uint8 *, int, int, sf::Uint8 );

#endif