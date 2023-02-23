#pragma once
#include <string>
#include <vector>
#include <optional>
#include "texture.h"
#include "mesh.h"

struct Material
{
    Texture* diffuse_texture;
    std::optional<Texture*> normal_map;
};

struct TexturedMesh
{
    Mesh* mesh;
    Material material;
};

void init_resources();
void free_resources();

std::vector<TexturedMesh> load_assimp_scene(const std::string& filename);
Texture* get_texture(const std::string& filename);

extern Mesh* quad_mesh;
extern Mesh* cube_mesh;
