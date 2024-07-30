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
    MinimapInfo(int scaleToScreen, float rayLenght) : minimapRelPos(scaleToScreen),
        minimapCenterX(screenStats::g_screenHeight / minimapRelPos),
        minimapCenterY(screenStats::g_screenWidth - minimapCenterX),
        minimapScale(minimapCenterX / rayLenght) {}
    int minimapRelPos;
    int minimapCenterX;
    int minimapCenterY;
    int minimapScale;
};

class GameGraphics
{
public:
    GameGraphics(std::unique_ptr<DataUtils::GameData>&, const std::string&);
    GameGraphics() = delete;
    GameGraphics& operator=(const GameGraphics&) = delete;
    void handle_events(sf::Window&, const GameCore::PlayerControler&);
    bool is_running() const { return m_window.isOpen(); }
    void start();
    void performGameCycle();
    bool goal_reached();

private:
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

    struct inGameMapAssets
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
    GameCore::PlayerControler m_playerController;
    MapData m_mapData;
    std::vector<std::pair<int, int>> m_pathToGoal;
    std::unique_ptr<PathFinder> m_pathFinder;
    const MinimapInfo m_minimapInfo;
    float m_halfWallHeight = 0.5f;

    GameAsset m_mainView;
    GameAsset m_mainBackground;
    sf::Text m_endGameText;
    sf::Font m_endGameFont;
    sf::CircleShape m_minimapFrame;
    inGameMapAssets m_igMapAssets;

    bool m_hadFocus = false;
    bool m_paused = false;
    bool m_findPathRequested = false;
    bool m_tabbed = false;

    void create_assets();
    void draw_camera_view();
    void draw_minimap_triangles();
    void draw_minimap_rays();
    void draw_minimap_background();
    void draw_map();
    void draw_end_screen();
    void draw_background();
    void draw_view();
    void draw_path_out();
    void create_minimap_frame();
    void load_end_screen();
    void generate_background();
    void handle_events();

    class RenderingThreadPool
    {
    public:
        RenderingThreadPool(GameGraphics&);
        ~RenderingThreadPool();
        void render_view();
        //bool is_buisy();

    private:
        int m_poolSize = 0;
        GameGraphics& m_gameGraphics;
        std::vector<std::thread> m_threads;
        std::vector<std::mutex> m_mutexVecStart;
        std::vector<std::mutex> m_mutexVecEnd;
        std::atomic_int jobs;
        int m_lastSectionSize = 0;
        bool m_isActive = true;

        float m_half_fov = m_gameGraphics.m_mapData.fov * .5f;
        float m_halfWallHeight = m_gameGraphics.m_halfWallHeight;
        float m_oneOverVerticalVisibleAngle = (screenStats::g_screenWidth / (screenStats::g_screenHeight * m_half_fov));

        void draw_section(int start, int end);
    };

    RenderingThreadPool m_renderingThreadPool;
};
#endif