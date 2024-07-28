#ifndef GAMEGRAPHICS_HPP
#define GAMEGRAPHICS_HPP
 
#include"gameCore.hpp"
#include<SFML/Graphics.hpp>

#define GENERATION_TIME_STEP_MS 5

namespace screenStats
{
    constexpr int g_screenHeight = 720;
    constexpr int g_screenWidth = g_screenHeight* 16/9;
    constexpr int minimap_scale = 6;
}

struct MinimapInfo
{
    MinimapInfo(int scaleToScreen, float rayLenght) : minimapRelPos(scaleToScreen),
        minimapH(screenStats::g_screenHeight / minimapRelPos),
        minimapW(screenStats::g_screenWidth - minimapH),
        minimapScale(minimapH / rayLenght) {}
    int minimapRelPos;
    int minimapH;
    int minimapW;
    int minimapScale;
};

class GameGraphics
{
public:
    GameGraphics() = delete;
    GameGraphics(GameCore& gameCore, const std::string&);
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

    GameCore& m_gameCore;
    sf::RenderWindow m_window;
    const RayInfoArr& m_raysInfoVec;
    GameAsset m_mainView;
    GameAsset m_mainBackground;
    sf::Text m_endGameText;
    sf::Font m_endGameFont;
    GameCore::PlayerControler m_playerController;
    MapData m_mapData;
    
    const MinimapInfo m_minimapInfo{ screenStats::minimap_scale, m_mapData.maxRenderDist };
    sf::CircleShape m_minimapFrame;
    bool m_hadFocus = false;
    bool m_paused = false;

    void draw_camera_view();
    void draw_minimap_triangles();
    void draw_minimap_rays();
    void draw_minimap_background();
    void draw_map();
    void draw_end_screen();
    void draw_background();
    void create_minimap_frame();
    void load_end_screen();
    void generate_background();
    void handle_events();
};
#endif