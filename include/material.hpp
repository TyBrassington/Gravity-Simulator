#pragma once
#include <SFML/Graphics.hpp>

struct Material {
    sf::Color ambient  = sf::Color::White;
    sf::Color diffuse  = sf::Color::White;
    sf::Color specular = sf::Color::Black;
    float shininess    = 32.0f;
    float opacity      = 1.0f;
};

