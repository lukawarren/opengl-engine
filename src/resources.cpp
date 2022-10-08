#include "resources.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <unordered_map>
#include <functional>
#include <iostream>

// We use T*'s so that:
// a) we can return persistent pointers
// b) copy constructor woes are avoided
static std::unordered_map<std::string, Mesh*> meshes;
static std::unordered_map<std::string, Texture*> textures;

static Mesh* mesh_from_assimp(const aiMesh* assimp_mesh, const std::string& id);
static Texture* texture_from_assimp(const aiMaterial* material, const std::string& path);

Mesh* quad_mesh;

void init_resources()
{
    quad_mesh = Mesh::quad();
}

std::vector<TexturedMesh> load_assimp_scene(const std::string& filename)
{
    Assimp::Importer importer;
    std::vector<TexturedMesh> textured_meshes;

    const auto flags = aiProcess_Triangulate | aiProcess_FlipUVs;

    // Load entire scene from disk
    const aiScene* scene = importer.ReadFile("../res/assets/" + filename, flags);
    if (!scene || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) || !scene->mRootNode)
        throw std::runtime_error("Unable to load model " + filename);

    // Recursively process all nodes - identify each mesh when cached by (filename, nth_mesh_in_scene)
    size_t nth_mesh = 0;
    const std::function<void(const aiNode*)> load_node = [&](const aiNode* node)
    {
        for (unsigned int i = 0; i < node->mNumMeshes; ++i)
        {
            // Get mesh data itself...
            const auto mesh_index = node->mMeshes[i];
            const aiMesh* mesh = scene->mMeshes[mesh_index];

            // ...and the material
            const auto material_index = mesh->mMaterialIndex;
            const aiMaterial* material = scene->mMaterials[material_index];

            textured_meshes.push_back({
                mesh_from_assimp(mesh, filename + std::to_string(nth_mesh++)),
                texture_from_assimp(material, filename)
            });
        }

        // Process children
        for (unsigned int i = 0; i < node->mNumChildren; ++i)
            load_node(node->mChildren[i]);
    };

    load_node(scene->mRootNode);
    return textured_meshes;
}

static Mesh* mesh_from_assimp(const aiMesh* assimp_mesh, const std::string& id)
{
    // Use cached version if available
    if (meshes.contains(id)) return meshes[id];

    std::vector<float>          vertices;
    std::vector<float>          normals;
    std::vector<float>          texture_coords;
    std::vector<unsigned int>   indices;

    // Reserve space in advance (faster)
    vertices       .reserve(assimp_mesh->mNumVertices * 3);
    normals        .reserve(assimp_mesh->mNumVertices * 3);
    texture_coords .reserve(assimp_mesh->mNumVertices * 2);
    indices        .reserve(assimp_mesh->mNumFaces * 3);

    // Load vertices and texture coords
    for (unsigned int i = 0; i < assimp_mesh->mNumVertices; ++i)
    {
        vertices.push_back(assimp_mesh->mVertices[i].x);
        vertices.push_back(assimp_mesh->mVertices[i].y);
        vertices.push_back(assimp_mesh->mVertices[i].z);

        normals.push_back(assimp_mesh->mNormals[i].x);
        normals.push_back(assimp_mesh->mNormals[i].y);
        normals.push_back(assimp_mesh->mNormals[i].z);

        texture_coords.push_back(assimp_mesh->mTextureCoords[0][i].x);
        texture_coords.push_back(assimp_mesh->mTextureCoords[0][i].y);
    }

    // Load indices
    for (unsigned int i = 0; i < assimp_mesh->mNumFaces; ++i)
    {
        const aiFace face = assimp_mesh->mFaces[i];
        if (face.mNumIndices != 3)
            throw std::runtime_error("incorrect number of indices per face");

        for (unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }

    // Cache for later then return
    Mesh* mesh = new Mesh(vertices, indices, texture_coords);
    meshes.emplace(id, mesh);
    return mesh;
}

static Texture* texture_from_assimp(const aiMaterial* material, const std::string& path)
{
    // Use missing texture if none exists
    if (material->GetTextureCount(aiTextureType_DIFFUSE) == 0)
        return get_texture("missing_texture.png");

    // Remove filename from path (so we're just left with directory)
    const std::string directory = path.substr(0, path.find_last_of('/')) + std::string("/");

    aiString filename;
    material->GetTexture(aiTextureType_DIFFUSE, 0, &filename);
    return get_texture(directory + std::string(filename.C_Str()));
}

Texture* get_texture(const std::string& filename)
{
    if (textures.contains(filename)) return textures[filename];
    auto iterator = textures.emplace(filename, new Texture(filename)).first;
    return iterator->second;
}

void free_resources()
{
    for(auto& mesh : meshes) delete mesh.second;
    for(auto& texture : textures) delete texture.second;
    delete quad_mesh;
}
