#ifndef GAMEGRAPHICS_HPP
#define GAMEGRAPHICS_HPP

#include"gameCore.hpp"
#include<SFML/Graphics.hpp>


namespace ScreenStats
{
    constexpr int g_screenWidth = 1280;
    constexpr int g_screenHeight = 720;
}

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
    std::vector<RayInfo> m_raysInfoVec;
    sf::Texture m_viewTexture;
    sf::Sprite m_viewSprite;
    GameCore::PlayerControler m_playerController;
    MapData m_mapData;
    sf::Uint8* m_screenPixels;
    const int m_minimapY = ScreenStats::g_screenHeight / 8;
    const int m_minimapX = (ScreenStats::g_screenWidth - ScreenStats::g_screenHeight/8);

    void draw_camera_view();
    void draw_minimap() ;
};
#endif