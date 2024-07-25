#ifndef GAMEGRAPHICS_HPP
#define GAMEGRAPHICS_HPP

#define MINIMAP_SCALE 6
 
#include"gameCore.hpp"
#include<SFML/Graphics.hpp>


namespace screenStats
{
    constexpr int g_screenHeight = 720;
    constexpr int g_screenWidth = g_screenHeight* 16/9;
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
    ~GameGraphics() { delete[] m_screenPixels; }
private:
    GameCore& m_gameCore;
    sf::RenderWindow m_window;
    const RayInfoArr& m_raysInfoVec;
    sf::Uint8* m_screenPixels;
    sf::Texture m_viewTexture;
    sf::Sprite m_viewSprite;
    sf::Uint8* m_mapPixels;
    sf::Texture m_mapTexture;
    sf::Sprite m_mapSprite;
    GameCore::PlayerControler m_playerController;
    MapData m_mapData;
    
    const MinimapInfo m_minimapInfo{ MINIMAP_SCALE, m_mapData.maxRenderDist };
    sf::CircleShape m_minimapFrame;
    bool m_hadFocus = false;
    bool m_paused = false;

    void draw_camera_view();
    void draw_minimap();
    void draw_minimap_rays();
    void draw_map_back();
    void draw_map();
    void create_minimap_frame();
    void handle_events();
};
#endif