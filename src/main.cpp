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

inline void handle_events(sf::Window&);

void draw_on_screen(sf::Uint8*);
void draw_on_screen(sf::RenderWindow&, const std::vector<RayInfo>& ,const sf::Vector2f& );

int main()
{
    //load ass(start)


    vecMath::rotation_mat2x2(10.f);
    

    //set win
    sf::RenderWindow window(sf::VideoMode(g_width, g_height), "My window", sf::Style::Close);
    window.setFramerateLimit(0);

    //create canvas
    //sf::Uint8* pixels = new sf::Uint8[g_width * g_height * 4];
    //sf::Texture viewTexture;
    //viewTexture.create(g_width, g_height);
    //sf::Sprite view(viewTexture);


    

    GameCore gameCore{ {{g_width, g_height}, 0.25f * (glm::pi<float>()), 30.f, 0.02f} };
    //sf::Vector2f plPos{ 15.f, 10.f };
    sf::Vector2f plPos{ 4.5f, 2.f };
    std::vector<RayInfo> rayInfoVec;

    if (!gameCore.load_map("map.txt"))
    {
        std::cerr << "Missing \"map\" file\n";
        return -1;
    }

    while (window.isOpen())
    {

        handle_events(window);
        window.clear(sf::Color::Black);

        debug::GameTimer gt;

        gt.reset_timer();
        
        rayInfoVec = gameCore.view_by_ray_casting();

        std::cout << "casting" << '\t' << gt.reset_timer() << std::endl;

        draw_on_screen(window, rayInfoVec, plPos);

        std::cout << "drawing" << '\t' << gt.reset_timer() << std::endl;
        //viewTexture.update(pixels);
        //window.draw(view);
        window.display();

        gt.add_frame();
        //std::cout << gt.get_frame_rate() << std::endl;
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

void draw_on_screen(sf::RenderWindow& window, const std::vector<RayInfo>& points, const sf::Vector2f& start)
{
    
    sf::VertexArray lines(sf::Lines, points.size() * 2);
    float multip = 100;
    for (int i = 0; i < points.size(); ++i)
    {
        lines[i * 2] = sf::Vector2f(start.x * multip, start.y * multip) ;
        lines[i*2 + 1] = sf::Vector2f((points[i].hitPos[0])* multip, (points[i].hitPos[1])* multip);
        lines[i * 2 + 1].color = sf::Color( 256, 0xFF, 0xFF);
        //std::cout << static_cast<char>(r.entityHit) << " " << r.hitPos[0] << " " << r.hitPos[1] << std::endl;

    }
    window.draw(lines);
}

void draw_on_screen(sf::Uint8* pixels) 
{
    for (int i = 0; i < g_width * g_height * 4; i += 4) 
    {
        pixels[i] = 0xff;
        pixels[i + 1] = i%120;
        pixels[i + 2] = i%200;
        pixels[i + 3] = i%256;
    }
}