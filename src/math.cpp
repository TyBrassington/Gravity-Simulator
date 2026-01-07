#include "math.hpp"

Vec2D project(const Vec3D& p) {
    if (p.z <= NEAR_PLANE_THRESHOLD)
        return { NAN, NAN };
    return Vec2D{
        p.x / (p.z),
        p.y / (p.z)
    };
}

Vec2D screen(const Vec2D& p, int w, int h) {
    return {
        (p.x + 1.0f) * 0.5f * w,
        (1.0f - (p.y + 1.0f) * 0.5f) * h
    };
}

Vec3D rotate_xz(const Vec3D& p, float angle) {
    float cos_a = std::cos(angle);
    float sin_a = std::sin(angle);
    return {
        p.x * cos_a - p.z * sin_a,
        p.y,
        p.x * sin_a + p.z * cos_a
    };
}

Vec3D rotate_yz(const Vec3D& p, float angle) {
    float cos_a = std::cos(angle);
    float sin_a = std::sin(angle);
    return {
        p.x,
        p.y * cos_a - p.z * sin_a,
        p.y * sin_a + p.z * cos_a
    };
}

Vec3D rotate_xy(const Vec3D& p, float angle) {
    float cos_a = std::cos(angle);
    float sin_a = std::sin(angle);
    return {
        p.x * cos_a - p.y * sin_a,
        p.x * sin_a + p.y * cos_a,
        p.z
    };
}

Vec3D translate_z(const Vec3D& p, float dz) {
    return {
        p.x,
        p.y,
        p.z + dz
    };
}

Vec3D cross(const Vec3D& a, const Vec3D& b) {
    return {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}

float dot(const Vec3D& a, const Vec3D& b) {
    return a.x*b.x + a.y*b.y + a.z*b.z;
}

Vec3D normalize(const Vec3D& v) {
    float len = std::sqrt(dot(v, v));
    return { v.x/len, v.y/len, v.z/len };
}

Vec3D toViewSpace(const Vec3D& v,
                  float ax, float ay, float az,
                  float dz)
{
    Vec3D p = v;
    p = rotate_yz(p, ax);
    p = rotate_xz(p, ay);
    p = rotate_xy(p, az);
    p = translate_z(p, dz);
    return p;
}

bool clipLineToNearPlane(Vec3D& a, Vec3D& b, float nearZ) {
    bool aIn = a.z > nearZ;
    bool bIn = b.z > nearZ;

    if (aIn && bIn) return true;
    if (!aIn && !bIn) return false;


    float t = (nearZ - a.z) / (b.z - a.z);
    Vec3D p = {
        a.x + t * (b.x - a.x),
        a.y + t * (b.y - a.y),
        nearZ
    };

    if (!aIn) a = p;
    else      b = p;

    return true;
}

Vec2D transform(const Vec3D& v,
                float ax, float ay, float az,
                float dz, int w, int h)
{
    Vec3D p = v;

    p = rotate_yz(p, ax); // rotate around X
    p = rotate_xz(p, ay); // rotate around Y
    p = rotate_xy(p, az); // rotate around Z

    p = translate_z(p, dz);

    Vec2D projected = project(p);
    return screen(projected, w, h);
}
