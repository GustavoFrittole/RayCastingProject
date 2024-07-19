#include <SFML/Graphics.hpp>
#include <cstdlib>

 // vec2, vec3, mat4, radians
#include "utils.hpp"
#include <iostream>
#include <thread>
#include<gameCore.hpp>

constexpr int g_width = 1280;
constexpr int g_height = 720;

inline void handle_events(sf::Window&);

void draw_on_screen(sf::Uint8*);

int main()
{
    //load ass(start)
    GameCore gameCore{ {{g_width, g_height }, 90.f, 10.f, 1.f} };
    if (gameCore.load_map("map.txt")) {
        std::unique_ptr<RayInfo[]> rayInfoVec = gameCore.view_by_ray_casting();
        std::cout << rayInfoVec[0].entityHit;
    }
    else return -1;
    

    //set win
    sf::RenderWindow window(sf::VideoMode(g_width, g_height), "My window", sf::Style::Close);
    window.setFramerateLimit(0);

    //create canvas
    sf::Uint8* pixels = new sf::Uint8[g_width * g_height * 4];
    sf::Texture viewTexture;
    viewTexture.create(g_width, g_height);
    sf::Sprite view(viewTexture);

    //load ass(end)

    debug::GameTimer gt;


    while (window.isOpen())
    {

        handle_events(window);
        window.clear(sf::Color::Black);

        draw_on_screen(pixels);
        viewTexture.update(pixels);
        //draw on screen
        window.draw(view);
        window.display();

        gt.add_frame();
        std::cout << gt.get_frame_rate() << std::endl;
    }



    return 0;
}    

inline void handle_events(sf::Window& window) 
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
}

void draw_on_screen(sf::Uint8* pixels) 
{
    for (int i = 0; i < g_width * g_height * 4; i += 4) 
    {
        pixels[i] = 0xFF;
        pixels[i + 1] = i%120;
        pixels[i + 2] = i%200;
        pixels[i + 3] = i%256;
    }
}