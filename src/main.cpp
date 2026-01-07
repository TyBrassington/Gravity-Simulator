#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>
#include <iostream>
#include <algorithm>

#include "math.hpp"


int main()
{


    const int W = 800;
    const int H = 800;

    sf::RenderWindow window(
        sf::VideoMode({ (unsigned)W, (unsigned)H }),
        "Gravity Simulator"
    );
    window.setFramerateLimit(60);

    float yaw   = 0.0f;
    float pitch = 0.0f;
    float roll  = 0.0f;
    float dz    = 6.0f;

    Vec3D camOffset{0, 0, 0};

    bool dragging = false;
    sf::Vector2i lastMousePos;

    sf::Clock clock;

    //test sphere

    Vec3D spherePos{0, 0, 0};
    float sphereRadius = 1.0f;

    Vec3D lightDirWorld = normalize({ 0.3f, 0.7f, 0.6f });


    while (window.isOpen()) {
        while (auto ev = window.pollEvent()) {
            if (ev->is<sf::Event::Closed>())
                window.close();

            if (const auto* mb = ev->getIf<sf::Event::MouseButtonPressed>()) {
                if (mb->button == sf::Mouse::Button::Left) {
                    dragging = true;
                    lastMousePos = mb->position;
                }
            }

            if (const auto* mb = ev->getIf<sf::Event::MouseButtonReleased>()) {
                if (mb->button == sf::Mouse::Button::Left) {
                    dragging = false;
                }
            }

            if (const auto* mm = ev->getIf<sf::Event::MouseMoved>()) {
                if (dragging) {
                    sf::Vector2i cur = mm->position;
                    sf::Vector2i delta = cur - lastMousePos;
                    lastMousePos = cur;

                    yaw   += delta.x * 0.005f;
                    pitch += delta.y * 0.005f;
                    pitch = std::clamp(pitch, -1.5f, 1.5f);
                }
            }

            if (const auto* mw = ev->getIf<sf::Event::MouseWheelScrolled>()) {
                dz -= mw->delta * 0.5f;
                dz = std::clamp(dz, 1.0f, 20.0f);
            }
        }

        float dt = clock.restart().asSeconds();

        Vec3D forward = {
            std::sin(yaw),
            0,
            std::cos(yaw)
        };
        Vec3D right = {
            std::cos(yaw),
            0,
            -std::sin(yaw)
        };

        float speed = 3.0f * dt;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) {
            camOffset.x += forward.x * speed;
            camOffset.z += forward.z * speed;
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) {
            camOffset.x -= forward.x * speed;
            camOffset.z -= forward.z * speed;
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) {
            camOffset.x -= right.x * speed;
            camOffset.z -= right.z * speed;
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) {
            camOffset.x += right.x * speed;
            camOffset.z += right.z * speed;
        }


        // apply gravity here
        
        window.clear(sf::Color(16, 16, 16));

        // draw bodies

        Vec3D worldPos = {
            spherePos.x - camOffset.x,
            spherePos.y - camOffset.y,
            spherePos.z - camOffset.z
        };

        Vec3D view = toViewSpace(worldPos, pitch, yaw, roll, dz);

        Vec3D lightDirView = normalize(
            rotate_xy(
                rotate_xz(
                    rotate_yz(lightDirWorld, pitch),
                    yaw
                ),
                roll
            )
        );

        if (view.z > NEAR_PLANE_THRESHOLD) {
            Vec2D p = screen(project(view), W, H);

            float r = sphereRadius / view.z * W;
            sf::Image sphereImg(
                sf::Vector2u((unsigned)(2*r), (unsigned)(2*r)),
                sf::Color::Transparent
            );

            for (int y = 0; y < (int)(2*r); y++) {
                for (int x = 0; x < (int)(2*r); x++) {
                    float nx = (x - r) / r;
                    float ny = (y - r) / r;

                    float d2 = nx*nx + ny*ny;
                    if (d2 > 1.0f)
                        continue;

                    float nz = std::sqrt(1.0f - d2);

                    Vec3D N = normalize({ nx, ny, nz });
                    float lambert = std::max(0.15f, dot(N, lightDirView));

                    uint8_t c = static_cast<uint8_t>(lambert * 255);
                    sphereImg.setPixel(
                        { (unsigned)x, (unsigned)y },
                        sf::Color(c, c, c)
                    );
                }
            }

            sf::Texture sphereTex;
            sphereTex.loadFromImage(sphereImg);

            sf::Sprite sphereSprite(sphereTex);
            sphereSprite.setPosition({ p.x - r, p.y - r });

            window.draw(sphereSprite);

        }

        window.display();
    }

    return 0;
}
