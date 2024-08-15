#ifndef GAMEGRAPHICS_HPP
#define GAMEGRAPHICS_HPP
 

#include<SFML/Graphics.hpp>
#include<thread>
#include<condition_variable> 
#include<mutex> 
#include<atomic>
#include"pathFinder.hpp"
#include"gameCore.hpp"

#define GENERATION_TIME_STEP_MS 5

struct MinimapInfo
{
    MinimapInfo(int scaleToScreen, float rayLength) : minimapRelPos(scaleToScreen),
        minimapCenterX(screenStats::g_screenHeight / minimapRelPos),
        minimapCenterY(screenStats::g_screenWidth - minimapCenterX),
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

class GameGraphics
{
public:
    GameGraphics(sf::RenderWindow&, const GraphicsVars&, int);
    GameGraphics() = delete;
    GameGraphics& operator=(const GameGraphics&) = delete;

    void create_assets(const GameAssets&, const GameMap&);
    bool is_running() const { return m_window.isOpen(); }
    bool goal_reached(const EntityTransform& pos, const GameMap& map);

    void draw_map_gen();
    void draw_minimap_triangles();
    void draw_minimap_rays();
    void draw_minimap_background();
    void draw_map();
    void draw_end_screen();
    void draw_path_out();
    void draw_view();
    void draw_sprites();
    void calculate_shortest_path(const EntityTransform&);
private:
    struct MapSquareAsset
    {
        void create(int, int);
        int tileDim = 0;
        //float tileThick;
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
    Texture m_wallTexture;
    Texture m_baundryTexture;
    Texture m_floorTexture;
    Texture m_ceilingTexture;
    Texture m_skyTexture;
    std::vector<Sprite> m_spritesToLoad;
    std::vector<std::unique_ptr<Texture>> m_spriteTexturesDict;

    void draw_camera_view();
    void draw_sprite_on_view(float, float, const Texture&);
    inline void load_end_screen();
    inline void load_sprites();
    inline void load_textures(const GameAssets&);

    class RenderingThreadPool
    {
    public:
        RenderingThreadPool(GameGraphics&);
        ~RenderingThreadPool();
        void render_view();
        void refresh_variables();

    private:
        int m_poolSize = 0;
        GameGraphics& m_gameGraphics;
        std::vector<std::thread> m_threads;
        std::vector<std::mutex> m_mutexVecStart;
        std::vector<std::mutex> m_mutexVecMid;
        std::vector<std::mutex> m_mutexVecEnd;
        std::atomic_int jobs;
        int m_lastSectionView = 0;
        int m_lastSectionBackgroud = 0;
        bool m_isActive = true;

        float m_verticalVisibleAngle = ((screenStats::g_screenHeight * m_gameGraphics.m_stateData.fov)/screenStats::g_screenWidth);
        float m_skyPixPerCircle = 0;
        float m_skyUIncrement = 0;
        float m_skyVIncrement = 0;

        void draw_veiw_section(int start, int end, const bool& linear);
        void draw_background_section(float startY, float endY, bool drawSky);
    };

    RenderingThreadPool m_renderingThreadPool;
};

inline void copy_pixels( sf::Uint8 *, const sf::Uint8 *, int, int, sf::Uint8 );

#endif