#ifndef GAMEGRAPHICS_HPP
#define GAMEGRAPHICS_HPP

#include"gameCore.hpp"
#include<SFML/Graphics.hpp>


namespace screenStats
{
    constexpr int g_screenWidth = 1280;
    constexpr int g_screenHeight = 720;
}

struct MinimapInfo
{
    MinimapInfo(int scale, float rayLenght) : minimapRelPos(scale),
        minimapY(screenStats::g_screenHeight / minimapRelPos),
        minimapX(screenStats::g_screenWidth - minimapY),
        minimapScale(minimapY / (rayLenght)) {}
    int minimapRelPos;
    int minimapY;
    int minimapX;
    int minimapScale;
};

class GameGraphics
{
public:
    GameGraphics() = delete;
    GameGraphics(GameCore& gameCore, const std::string&);
    inline void GameGraphics::handle_events(sf::Window&, const GameCore::PlayerControler&);
    bool is_running() const { return m_window.isOpen(); }
    void start();
    void performGameCycle();
    void handle_events();
    ~GameGraphics() { delete[] m_screenPixels; }
private:
    GameCore& m_gameCore;
    sf::RenderWindow m_window;
    const RayInfoArr& m_raysInfoVec;
    sf::Texture m_viewTexture;
    sf::Sprite m_viewSprite;
    GameCore::PlayerControler m_playerController;
    MapData m_mapData;
    sf::Uint8* m_screenPixels;
    const MinimapInfo m_minimapInfo{ 6, m_mapData.maxRenderDist };
    sf::CircleShape m_minimapFrame;
    bool m_hadFocus = false;

    void draw_camera_view();
    void draw_minimap();
    void draw_minimap_rays();
    void create_minimap_frame();
};
#endif