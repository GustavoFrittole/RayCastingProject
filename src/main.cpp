#include <SFML/Graphics.hpp>
#include <cstdlib>
#include <iostream>
#include <chrono>

#include <glm/glm.hpp> // vec2, vec3, mat4, radians

using namespace std::chrono_literals;

constexpr int g_width = 1280;
constexpr int g_height = 720;

int main()
{
    sf::RenderWindow window(sf::VideoMode(g_width, g_height), "My window", sf::Style::Close );
    window.setFramerateLimit(60);

    std::string map;
    map += {'w','w','w','w','w','w','w','w'};
    map += {'w',' ',' ',' ',' ',' ',' ','w'};
    map += {'w',' ',' ',' ',' ',' ',' ','w'};
    map += {'w',' ',' ',' ',' ',' ',' ','w'};
    map += {'w',' ',' ',' ',' ',' ',' ','w'};
    map += {'w','w','w','w','w','w','w','w'};

    constexpr int mapX = 8;
    constexpr int mapY = 6;

    std::chrono::nanoseconds dt(0ns);
    int frameCounter = 0;
    auto tStart = std::chrono::high_resolution_clock::now();

    sf::CircleShape shape(5.f);

    while (window.isOpen())
    {
        auto deltaTime = std::chrono::high_resolution_clock::now() - tStart;
        tStart = std::chrono::high_resolution_clock::now();
        dt += deltaTime;
        frameCounter++; 
        
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

        // clear the window with black color
        window.clear(sf::Color::Black);

        // draw everything here...

        

        sf::Color sCol = shape.getFillColor();
        for (int i = 0; i < 100; i++)
        {
            shape.setPosition(10*i, 0);
            shape.setFillColor({ static_cast<sf::Uint8>(dt.count()), static_cast<sf::Uint8>(dt.count() * 1), static_cast<sf::Uint8>(dt.count() * 2)});
            window.draw(shape);
        }
        for (int i = 0; i < 100; i++)
        {
            shape.setPosition(10 * i, 10);
            shape.setFillColor({ static_cast<sf::Uint8>(dt.count()), static_cast<sf::Uint8>(dt.count() * 1), static_cast<sf::Uint8>(dt.count() * 2)});
            window.draw(shape);
        }

        auto tEnd = std::chrono::steady_clock::now();
        deltaTime = tEnd - tStart;

        
        
        if (dt > 1s)
        {
            std::cout << frameCounter << std::endl;
            dt = 0ms;
            frameCounter = 0;
        }

        // end the current frame
        window.display();
    }

    return 0;
}