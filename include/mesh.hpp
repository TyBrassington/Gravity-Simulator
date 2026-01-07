#pragma once
#include <vector>
#include "material.hpp"

struct Vec3D;
struct Face {
    std::vector<int> indices;
    std::vector<int> normal_indices;
    int material;
};

struct Mesh {
    std::vector<Vec3D> vertices;
    std::vector<Vec3D> normals;
    std::vector<Face> faces;
    std::vector<Material> materials;
    std::unordered_map<std::string, int> matIndex;
};
