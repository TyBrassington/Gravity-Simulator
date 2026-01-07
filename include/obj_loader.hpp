#pragma once
#include <string>
#include "mesh.hpp"

bool loadOBJ(const std::string& path, Mesh& mesh);
bool loadMTL(const std::string& path,
             std::unordered_map<std::string, int>& matIndex,
             std::vector<Material>& materials);
