#pragma once
#include <cmath>

struct Vec3D {
    float x, y, z;
};

struct Vec2D {
    float x, y;
};

constexpr float NEAR_PLANE_THRESHOLD = 0.01f;

Vec3D rotate_xz(const Vec3D& p, float angle);
Vec3D rotate_yz(const Vec3D& p, float angle);
Vec3D rotate_xy(const Vec3D& p, float angle);
Vec3D translate_z(const Vec3D& p, float dz);
Vec3D cross(const Vec3D& a, const Vec3D& b);
float dot(const Vec3D& a, const Vec3D& b);
Vec3D normalize(const Vec3D& v);
Vec3D toViewSpace(const Vec3D& v,
                  float ax, float ay, float az,
                  float dz);
bool clipLineToNearPlane(Vec3D& a, Vec3D& b, float nearZ);

Vec2D project(const Vec3D& p);
Vec2D screen(const Vec2D& p, int w, int h);
Vec2D transform(const Vec3D& v,
                float ax, float ay, float az,
                float dz, int w, int h);
