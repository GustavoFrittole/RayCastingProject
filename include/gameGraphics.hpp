#ifndef GAMEGRAPHICS_HPP
#define GAMEGRAPHICS_HPP
 
#include"gameCore.hpp"
#include<SFML/Graphics.hpp>
#include<thread>
#include<condition_variable> 
#include<mutex> 
#include<atomic>
#include"pathFinder.hpp"

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

struct GameAsset
{
    GameAsset() = default;
    GameAsset(int, int, bool);
    void create(int, int, bool);
    GameAsset& operator=(const GameAsset&) = delete;
    GameAsset(const GameAsset&) = delete;
    ~GameAsset() { if (m_hasPixelArray) delete[] m_pixels; }
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
    GameGraphics(std::unique_ptr<DataUtils::GameData>&, const std::string&);
    GameGraphics() = delete;
    GameGraphics& operator=(const GameGraphics&) = delete;
    void handle_events(sf::Window&, const GameCore::PlayerController&);
    bool is_running() const { return m_window.isOpen(); }
    void start();
    void performGameCycle();
    bool goal_reached();
private:
    struct MapSquareAsset
    {
        void create(const GameGraphics&);
        int tileDim = 0;
        //float tileThick;
        int xoffset = 0;
        int yoffset = 0;
        sf::RectangleShape wallRect;
    };

    GameCore m_gameCore;
    sf::RenderWindow m_window;
    const RayInfoArr& m_raysInfoVec;
    GameCore::PlayerController& m_playerController;
    GameStateData m_stateData;
    std::vector<std::pair<int, int>> m_pathToGoal;
    std::unique_ptr<PathFinder> m_pathFinder;
    const MinimapInfo m_minimapInfo;
    float m_halfWallHeight = 0.5f;
    Controls m_controlsMulti;

    GameAsset m_mainView;
    GameAsset m_mainBackground;
    sf::Text m_endGameText;
    sf::Font m_endGameFont;
    MapSquareAsset m_mapSquareAsset;
    GameAssets m_gameAssets;
    Texture m_wallTexture;
    Texture m_baundryTexture;
    Texture m_floorTexture;
    Texture m_ceilingTexture;
    Texture m_skyTexture;
    std::vector<Sprite> m_spritesToLoad;
    std::vector<std::unique_ptr<Texture>> m_spriteTexturesDict;

    bool m_hadFocus = false;
    bool m_paused = false;
    bool m_findPathRequested = false;
    bool m_tabbed = false;
    bool m_linear = true;

    void draw_camera_view();
    void draw_minimap_triangles();
    void draw_minimap_rays();
    void draw_minimap_background();
    void draw_map();
    void draw_end_screen();
    void draw_background();
    void draw_path_out();
    void draw_view();
    void draw_sprites();
    void draw_sprite_on_view(float, float, const Texture&);
    void load_end_screen();
    void generate_background();
    void handle_events();
    inline void create_assets();
    inline void load_sprites();

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
        void draw_background_section(float startY, float endY);
    };

    RenderingThreadPool m_renderingThreadPool;
};
#endif