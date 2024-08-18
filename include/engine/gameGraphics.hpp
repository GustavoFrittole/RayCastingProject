#ifndef GAMEGRAPHICS_HPP
#define GAMEGRAPHICS_HPP

#include <SFML/Graphics.hpp>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <atomic>
#include "pathFinder.hpp"
#include "gameCore.hpp"
#include "rendThreadPool.hpp"



struct MinimapInfo
{
    MinimapInfo(int scaleToScreen, float rayLength) : minimapRelPos(scaleToScreen),
        minimapCenterX(graphicsVars::g_screenHeight / minimapRelPos),
        minimapCenterY(graphicsVars::g_screenWidth - minimapCenterX),
        minimapScale(minimapCenterX / rayLength) {}
    int minimapRelPos;
    int minimapCenterX;
    int minimapCenterY;
    int minimapScale;
};

struct GameView
{
    GameView() = default;
    GameView(int, int, bool);
    void create(int, int, bool);
    GameView& operator=(const GameView&) = delete;
    GameView(const GameView&) = delete;
    ~GameView() { if (m_hasPixelArray) delete[] m_pixels; }
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

class GameGraphics
{
public:
    GameGraphics(sf::RenderWindow&, const GraphicsVars&, const RayInfoArr&, const GameStateVars&, const EntityTransform&, int = 0);
    GameGraphics() = delete;
    GameGraphics& operator=(const GameGraphics&) = delete;

    void create_assets(const GameAssets&, const GameMap&);
    void load_sprite(const std::string&);
    bool is_running() const { return m_window.isOpen(); }
    //no trasfer
    bool goal_reached(const EntityTransform& pos, const GameMap& map);

    void draw_map_gen(int mapWidth, int mapHeight, int posX, int posY, const std::string& cells);
    void draw_minimap_triangles(int winPixWidth, const RayInfoArr& rays, const GraphicsVars& graphVars);
    void draw_minimap_rays(int winPixWidth, const RayInfoArr& rays);
    void draw_minimap_background(const GameMap& gameMap, const EntityTransform& cameraTransform, const GraphicsVars& graphVars);
    void draw_map(int mapWidth, int mapHeight, int posX, int posY, const std::string& cells);
    void draw_end_screen();
    void draw_path_out();
    void draw_view();
    void draw_sprites();
    void calculate_shortest_path(const EntityTransform&);

    void static draw_view_section(int start, int end, bool linear, const RayInfoArr& rays, GameView& view, const GraphicsVars& graphVars, const StaticTextures& tex, const EntityTransform& camTransform);
    void static draw_background_section(float startY, float endY, bool drawSky);

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
    void draw_sprite_on_view(float, float, const Texture&);
    inline void load_end_screen();
    inline void load_textures(const GameAssets&);

    RendThreadPool m_rendThreadPool;

    ViewRendSectionFactory m_viewSecFactory;
    std::vector<ViewRendSectionFactory::ViewRendSection> m_viewSectionsVec;
    void crete_view_sections();

    inline void render_view();
    inline void GameGraphics::rend_view_secs();
    //inline void GameGraphics::queue_background_rend();
    //inline void GameGraphics::queue_sprite_rend();
};

inline void copy_pixels( sf::Uint8 *, const sf::Uint8 *, int, int, sf::Uint8 );

#endif