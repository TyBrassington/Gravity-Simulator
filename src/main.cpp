#include <SFML/Graphics.hpp>
#include <vector>
#include <limits>
#include <cmath>
#include <iostream>
#include <algorithm>

#include "math.hpp"
#include "mesh.hpp"
#include "obj_loader.hpp"

static inline float edge(const sf::Vector2f& a,
                         const sf::Vector2f& b,
                         const sf::Vector2f& c)
{
    return (c.x - a.x) * (b.y - a.y) - (c.y - a.y) * (b.x - a.x);
}

static inline uint8_t toByte(float x) {
    x = std::clamp(x, 0.0f, 255.0f);
    return static_cast<uint8_t>(x);
}

static inline Vec3D interpVec(
    const Vec3D& a, const Vec3D& b, const Vec3D& c,
    float w0, float w1, float w2)
{
    return {
        a.x * w0 + b.x * w1 + c.x * w2,
        a.y * w0 + b.y * w1 + c.y * w2,
        a.z * w0 + b.z * w1 + c.z * w2
    };
}

static inline Vec3D toViewDir(const Vec3D& n,
                             float ax, float ay, float az)
{
    return rotate_xy(
        rotate_xz(
            rotate_yz(n, ax), ay), az);
}


static inline sf::Color addColorClamped(const sf::Color& a, const sf::Color& b) {
    return sf::Color(
        toByte(float(a.r) + float(b.r)),
        toByte(float(a.g) + float(b.g)),
        toByte(float(a.b) + float(b.b))
    );
}

static inline sf::Color scaleColor(const sf::Color& c, float s) {
    return sf::Color(
        toByte(float(c.r) * s),
        toByte(float(c.g) * s),
        toByte(float(c.b) * s)
    );
}

static inline sf::Color shade(const Material& mat,
                              const Vec3D& n,
                              const Vec3D& pos,
                              const Vec3D& lightDir)
{
    Vec3D N = n;
    Vec3D L = normalize(lightDir);
    Vec3D V = normalize({ -pos.x, -pos.y, -pos.z });

    float lambert = std::max(0.0f, dot(N, L));

    sf::Color amb = scaleColor(mat.diffuse, 0.1f);
    sf::Color dif = scaleColor(mat.diffuse, lambert);

    sf::Color spc(0, 0, 0);
    if (lambert > 0.0f) {
        Vec3D H = normalize({ L.x + V.x, L.y + V.y, L.z + V.z });
        float shininess = std::clamp(mat.shininess * 0.1f, 1.0f, 64.0f);
        float spec = std::pow(std::max(0.0f, dot(N, H)), shininess);
        spc = scaleColor(mat.specular, spec * 4.0f);
    }

    return addColorClamped(addColorClamped(amb, dif), spc);
}



int main()
{
    Mesh mesh;
    if (!loadOBJ("assets/obj/mora.obj", mesh)) {
        std::cerr << "OBJ load failed\n";
        return -1;
    }

    const int W = 800;
    const int H = 800;

    sf::RenderWindow window(sf::VideoMode({ (unsigned)W, (unsigned)H }), "Software Z-Buffer");
    window.setFramerateLimit(60);

    sf::Image framebuffer({ (unsigned)W, (unsigned)H }, sf::Color(16,16,16));
    sf::Texture texture({ (unsigned)W, (unsigned)H });
    sf::Sprite sprite(texture);

    std::vector<float> zbuffer(W * H, std::numeric_limits<float>::infinity());

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

        // Clear framebuffer (SFML 3: easiest is re-init or set pixels in a loop; re-init is fine for now)
        framebuffer = sf::Image({ (unsigned)W, (unsigned)H }, sf::Color(16,16,16));
        std::fill(zbuffer.begin(), zbuffer.end(), std::numeric_limits<float>::infinity());

        Vec3D viewLight = normalize(
            rotate_xy(
                rotate_xz(
                    rotate_yz(lightDir, ax), ay), az));

        for (const auto& face : mesh.faces) {
            // Your loader already triangulates, so each face should be exactly 3 indices.
            if (face.indices.size() != 3) continue;

            // Safe material fetch (avoid out-of-range)
            int midx = face.material;
            if (midx < 0 || midx >= (int)mesh.materials.size()) midx = 0;
            const Material& mat = mesh.materials[midx];

            //debug
            Material debugMat = mat;
            debugMat.specular = sf::Color(120, 120, 120);
            debugMat.shininess = 64.0f;

            Vec3D v0 = toViewSpace(mesh.vertices[face.indices[0]], ax, ay, az, dz);
            Vec3D v1 = toViewSpace(mesh.vertices[face.indices[1]], ax, ay, az, dz);
            Vec3D v2 = toViewSpace(mesh.vertices[face.indices[2]], ax, ay, az, dz);

            Vec3D n0 = normalize(toViewDir(mesh.normals[face.normal_indices[0]], ax, ay, az));
            Vec3D n1 = normalize(toViewDir(mesh.normals[face.normal_indices[1]], ax, ay, az));
            Vec3D n2 = normalize(toViewDir(mesh.normals[face.normal_indices[2]], ax, ay, az));


            if (v0.z <= NEAR_PLANE_THRESHOLD ||
                v1.z <= NEAR_PLANE_THRESHOLD ||
                v2.z <= NEAR_PLANE_THRESHOLD)
                continue;

            Vec3D n = normalize(cross(
                {v1.x - v0.x, v1.y - v0.y, v1.z - v0.z},
                {v2.x - v0.x, v2.y - v0.y, v2.z - v0.z}
            ));

            // Make normal face the camera (camera at origin in view space)
            // If it points "away", flip it.
            if (dot(n, v0) > 0.0f) {
                n = { -n.x, -n.y, -n.z };
            }

            sf::Color col = shade(mat, n, v0, viewLight);

            Vec2D p0 = screen(project(v0), W, H);
            Vec2D p1 = screen(project(v1), W, H);
            Vec2D p2 = screen(project(v2), W, H);

            sf::Vector2f a(p0.x, p0.y);
            sf::Vector2f b(p1.x, p1.y);
            sf::Vector2f c(p2.x, p2.y);

            float area = edge(a, b, c);
            if (area == 0.0f) continue;

            int minX = std::max(0, (int)std::floor(std::min({a.x, b.x, c.x})));
            int maxX = std::min(W - 1, (int)std::ceil (std::max({a.x, b.x, c.x})));
            int minY = std::max(0, (int)std::floor(std::min({a.y, b.y, c.y})));
            int maxY = std::min(H - 1, (int)std::ceil (std::max({a.y, b.y, c.y})));

            float iz0 = 1.0f / v0.z;
            float iz1 = 1.0f / v1.z;
            float iz2 = 1.0f / v2.z;

            for (int y = minY; y <= maxY; y++) {
                for (int x = minX; x <= maxX; x++) {
                    sf::Vector2f p(x + 0.5f, y + 0.5f);

                    float w0 = edge(b, c, p);
                    float w1 = edge(c, a, p);
                    float w2 = edge(a, b, p);

                    if ((w0 >= 0) == (area >= 0) &&
                        (w1 >= 0) == (area >= 0) &&
                        (w2 >= 0) == (area >= 0))
                    {
                        w0 /= area;
                        w1 /= area;
                        w2 /= area;

                        float iz = w0 * iz0 + w1 * iz1 + w2 * iz2;
                        float z  = 1.0f / iz;

                        int idx = y * W + x;
                        if (z < zbuffer[idx]) {
                            zbuffer[idx] = z;

                            // Perspective-correct weights
                            float pw0 = w0 * iz0 / iz;
                            float pw1 = w1 * iz1 / iz;
                            float pw2 = w2 * iz2 / iz;

                            // Interpolate position + normal
                            Vec3D P = interpVec(v0, v1, v2, pw0, pw1, pw2);
                            Vec3D N = normalize(interpVec(n0, n1, n2, pw0, pw1, pw2));

                            sf::Color col = shade(debugMat, N, P, viewLight);
                            framebuffer.setPixel({ (unsigned)x, (unsigned)y }, col);
                        }
                    }
                }
            }

        }

        texture.update(framebuffer);
        window.clear();
        window.draw(sprite);
        window.display();
    }

    return 0;
}
