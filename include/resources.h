#pragma once
#include <string>
#include <vector>
#include "texture.h"
#include "mesh.h"

struct TexturedMesh
{
    Mesh* mesh;
    Texture* texture;
};

void init_resources();
void free_resources();

std::vector<TexturedMesh> load_assimp_scene(const std::string& filename);
Texture* texture(const std::string& filename);

extern Mesh* quad_mesh;
