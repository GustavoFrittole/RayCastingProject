#include <SFML/Graphics.hpp>
#include <cstdlib>

 // vec2, vec3, mat4, radians
#include "utils.hpp"
#include <iostream>
#include <thread>
#include<gameCore.hpp>
#include<glm/ext/scalar_constants.hpp>
constexpr int g_width = 1280;
constexpr int g_height = 720;

inline void handle_events(sf::Window&, const GameCore::PlayerControler&);

void draw_on_screen(sf::RenderWindow& window, const std::vector<RayInfo>& points, sf::Uint8* pixels);
void draw_on_screen(sf::RenderWindow&, const std::vector<RayInfo>& ,const sf::Vector2f& );

int main()
{
    //set win
    sf::RenderWindow window(sf::VideoMode(g_width, g_height), "My window", sf::Style::Close);
    window.setFramerateLimit(0);

    //create canvas
    sf::Uint8* pixels = new sf::Uint8[g_width * g_height * 4];
    sf::Texture viewTexture;
    viewTexture.create(g_width, g_height);
    sf::Sprite view(viewTexture);

    GameCore gameCore{ {{g_width, g_height}, 0.25f * (glm::pi<float>()), 8.f*8, 0.02f}};

    if (!gameCore.load_map("map.txt"))
    {
        std::cerr << "Missing \"map\" file\n";
        return -1;
    }

    //sf::Vector2f plPos{ 15.f, 10.f };
    std::vector<RayInfo> rayInfoVec;

    GameCore::PlayerControler playerController(&gameCore);

    gameCore.start_internal_time();

    debug::GameTimer gt;

    while (window.isOpen())
    {

        handle_events(window, playerController);
        window.clear(sf::Color::Black);

        

        gt.reset_timer();
        
        gameCore.update_entities();
        rayInfoVec = gameCore.view_by_ray_casting();

        std::cout << "casting" << '\t' << gt.reset_timer() << std::endl;

        
        draw_on_screen(window, rayInfoVec, pixels);
        viewTexture.update(pixels);
        window.draw(view);

        draw_on_screen(window, rayInfoVec, { g_width - 4 * 32,4 * 32 });

        std::cout << "drawing" << '\t' << gt.reset_timer() << std::endl;
        window.display();

        gt.add_frame();
        //std::cout << gt.get_frame_rate() << std::endl;
    }



    return 0;
}    

inline void handle_events(sf::Window& window, const GameCore::PlayerControler& playerController) 
{
    sf::Event event;
    while (window.pollEvent(event))
    {
        switch (event.type)
        {
        case sf::Event::Closed:
            window.close();
            break;
        }
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
    {
        playerController.move_foreward(1.f);
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
    {
        playerController.move_strafe(1.f);
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
    {
        playerController.move_strafe(-1.f);
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
    {
        playerController.move_foreward(-1.f);
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
    {
        playerController.rotate(1.0f);
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
    {
        playerController.rotate(-1.0f);
    }

}

void draw_on_screen(sf::RenderWindow& window, const std::vector<RayInfo>& raysInfo, const sf::Vector2f& start)
{
    
    sf::VertexArray lines(sf::Lines, raysInfo.size() * 2);
    float multip = 32;
    for (int i = 0; i < raysInfo.size(); ++i)
    {
        lines[i * 2] = sf::Vector2f(start.x, start.y ) ;
        lines[i * 2 + 1] = { { start.x + raysInfo[i].hitPos[0] * multip, start.y + raysInfo[i].hitPos[1] * multip}, sf::Color(i%256, 0xFF, 0xFF) };
        //std::cout << static_cast<char>(r.entityHit) << " " << r.hitPos[0] << " " << r.hitPos[1] << std::endl;

    }
    window.draw(lines);
}

void draw_on_screen(sf::RenderWindow& window, const std::vector<RayInfo>& raysInfo, sf::Uint8* pixels)
{
    for (int i = 0; i < raysInfo.size(); ++i)
    {
 
        if (raysInfo[i].entityHit == EntityType::Wall) 
        {
            float distance = glm::length(raysInfo[i].hitPos) + 1;
            float distanceFromFloor = (g_height * (1 - 1/distance))*0.5f;

            sf::Color pixelColor;
            for (int y = 0; y < g_height * 4; y+=4)
            {
                int x = i * 4;
                if (y < distanceFromFloor*4 || y > (g_height - distanceFromFloor)*4)
                    pixelColor = { 0x32, 0, 0x32, 0xff };
                else 
                    pixelColor = { 0xff, 0xff, 0xff , 0xff };
                pixels[y * g_width + x] = pixelColor.r;
                pixels[y * g_width + x + 1] = pixelColor.g;
                pixels[y * g_width + x + 2] = pixelColor.b;
                pixels[y * g_width + x + 3] = pixelColor.a;
            }
        }
    }
}