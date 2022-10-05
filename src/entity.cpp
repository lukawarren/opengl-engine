#include "entity.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <stdexcept>
#include <functional>

static Mesh* mesh_from_assimp(const aiMesh* assimp_mesh);

Entity::Entity(const std::string& filename)
{
    Assimp::Importer importer;

    const auto flags =
        aiProcess_Triangulate |
        aiProcess_OptimizeMeshes |
        aiProcess_OptimizeGraph |
        aiProcess_JoinIdenticalVertices |
        aiProcess_FlipUVs |
        aiProcess_CalcTangentSpace;

    // Load entire scene from disk
    const aiScene* scene = importer.ReadFile("../res/models/" + filename, flags);
    if (!scene || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) || !scene->mRootNode)
        throw std::runtime_error("Unable to load model " + filename);

    // Recursively process all nodes
    const std::function<void(const aiNode*)> load_node = [&](const aiNode* node)
    {
        for (unsigned int i = 0; i < node->mNumMeshes; ++i)
        {
            const auto meshIndex = node->mMeshes[i];
            const aiMesh* mesh = scene->mMeshes[meshIndex];
            meshes.push_back(
                mesh_from_assimp(mesh)
            );
        }

        // Process children
        for (unsigned int i = 0; i < node->mNumChildren; ++i)
            load_node(node->mChildren[i]);
    };

    load_node(scene->mRootNode);
}

static Mesh* mesh_from_assimp(const aiMesh* assimp_mesh)
{
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
        if (face.mNumIndices != 3) throw std::runtime_error("incorrect number of indices per face");

        for (unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }

    return new Mesh(vertices, indices, texture_coords);
}