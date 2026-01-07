#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>
#include <iostream>

#include "math.hpp"


int main()
{


    const int W = 800;
    const int H = 800;

    sf::RenderWindow window(sf::VideoMode({ (unsigned)W, (unsigned)H }), "Gravity Simulator");
    window.setFramerateLimit(60);

    float angle = 0.f;
    float dz = 1.5f;

    Vec3D lightDir = normalize({0.3f, 0.8f, 0.5f});
    while (window.isOpen()) {
        while (auto ev = window.pollEvent()) {
            if (ev->is<sf::Event::Closed>())
                window.close();
        }

        angle += 0.01f;

        float ax = angle * 0.7f;
        float ay = angle;
        float az = angle * 0.4f;
        Vec3D lightDir = normalize({0.3f, 0.8f, 0.5f});

        Vec3D viewLight = normalize(
            rotate_xy(
                rotate_xz(
                    rotate_yz(lightDir, ax), ay), az));


        window.clear();
        window.display();
    }

    return 0;
}
