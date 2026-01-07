#include "obj_loader.hpp"
#include "math.hpp"

#include <fstream>
#include <sstream>
#include <algorithm>

static std::string dirOf(const std::string& path) {
    size_t slash = path.find_last_of("/\\");
    if (slash == std::string::npos) return "";
    return path.substr(0, slash + 1);
}

bool loadOBJ(const std::string& path, Mesh& mesh) {

    if (mesh.materials.empty()) {
        Material def;
        def.diffuse = sf::Color(200, 200, 200);
        mesh.matIndex["__default__"] = 0;
        mesh.materials.push_back(def);
    }
    std::ifstream file(path);
    if (!file) return false;
    std::string baseDir = dirOf(path);
    std::string line;
    int currentMaterial = 0;

    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string type;
        ss >> type;

        if (type == "v") {
            Vec3D v;
            ss >> v.x >> v.y >> v.z;
            mesh.vertices.push_back(v);
        }
        else if (type == "vn") {
            Vec3D n;
            ss >> n.x >> n.y >> n.z;
            mesh.normals.push_back(normalize(n));
        }
        else if (type == "f") {
            Face f;
            f.material = currentMaterial;

            std::string token;
            while (ss >> token) {
                int v = -1, vt = -1, vn = -1;

                size_t p1 = token.find('/');
                size_t p2 = token.find('/', p1 + 1);

                v = std::stoi(token.substr(0, p1)) - 1;

                if (p2 != std::string::npos) {
                    vn = std::stoi(token.substr(p2 + 1)) - 1;
                }

                f.indices.push_back(v);
                f.normal_indices.push_back(vn);
            }

            for (size_t i = 1; i + 1 < f.indices.size(); i++) {
                Face tri;
                tri.indices = {
                    f.indices[0],
                    f.indices[i],
                    f.indices[i + 1]
                };
                tri.normal_indices = {
                    f.normal_indices[0],
                    f.normal_indices[i],
                    f.normal_indices[i + 1]
                };
                tri.material = f.material;
                mesh.faces.push_back(tri);
            }
        }

        else if (type == "mtllib") {
            std::string mtlFile;
            ss >> mtlFile;
            loadMTL(baseDir + mtlFile, mesh.matIndex, mesh.materials);
        }
        else if (type == "usemtl") {
            std::string name;
            ss >> name;

            auto it = mesh.matIndex.find(name);
            currentMaterial = (it != mesh.matIndex.end()) ? it->second : 0;
        }

    }

    if (mesh.materials.size() == 2) {
        // materials[0] = default
        // materials[1] = MTL material
        for (auto& face : mesh.faces) {
            if (face.material == 0) {
                face.material = 1;
            }
        }
    }

    return true;
}

bool loadMTL(const std::string& path,
             std::unordered_map<std::string, int>& matIndex,
             std::vector<Material>& materials)
{
    if (materials.empty()) {
        Material def;
        def.diffuse = sf::Color(200, 200, 200);
        matIndex["__default__"] = 0;
        materials.push_back(def);
    }

    std::ifstream file(path);
    if (!file) return false;

    std::string line;
    std::string current;

    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string tag;
        ss >> tag;

        if (tag == "newmtl") {
            ss >> current;
            matIndex[current] = materials.size();
            materials.push_back(Material{});
        }
        else if (tag == "Ka") {
            float r, g, b;
            ss >> r >> g >> b;
            materials.back().ambient = sf::Color(
                uint8_t(r * 255),
                uint8_t(g * 255),
                uint8_t(b * 255)
            );
        }
        else if (tag == "Kd") {
            float r, g, b;
            ss >> r >> g >> b;
            materials.back().diffuse = sf::Color(
                uint8_t(r * 255),
                uint8_t(g * 255),
                uint8_t(b * 255)
            );
        }
        else if (tag == "Ks") {
            float r, g, b;
            ss >> r >> g >> b;
            materials.back().specular = sf::Color(
                uint8_t(r * 255),
                uint8_t(g * 255),
                uint8_t(b * 255)
            );
        }
        else if (tag == "Ns") {
            float ns;
            ss >> ns;

            materials.back().shininess = std::clamp(ns / 10.0f, 4.0f, 64.0f);
            
        }

        else if (tag == "d") {
            ss >> materials.back().opacity;
        }
    }


    return true;
}

