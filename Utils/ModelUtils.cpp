#include "ModelUtils.h"
#include "Utils/MacroUtils.h"

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

MORISA_NAMESPACE_BEGIN

static MMesh* TransformMesh(const aiScene* scene, const aiMesh* mesh)
{
    MMesh* mMesh = MORISA_NEW(MMesh);
    
    const uint32_t numVertices = mesh->mNumVertices;
    const bool hasNormal = mesh->HasNormals();
    const bool hasColor = mesh->HasVertexColors(0);
    const bool hasUV = mesh->HasTextureCoords(0);
    mMesh->vertices.resize(numVertices);

    for (uint32_t i = 0; i < numVertices; ++i)
    {
        MMeshVertexData& vertex = mMesh->vertices[i];
        vertex.position.x = mesh->mVertices[i].x;
        vertex.position.y = mesh->mVertices[i].y;
        vertex.position.z = mesh->mVertices[i].z;

        if (hasNormal)
        {
            vertex.normal.x = mesh->mNormals[i].x;
            vertex.normal.y = mesh->mNormals[i].y;
            vertex.normal.z = mesh->mNormals[i].z;
        }
        else
        {
            vertex.normal.x = 0.0f;
            vertex.normal.y = 0.0f;
            vertex.normal.z = 0.0f;
        }

        if (hasColor)
        {
            // TODO Alpha
            vertex.color.x = mesh->mColors[0][i].r;
            vertex.color.y = mesh->mColors[0][i].g;
            vertex.color.z = mesh->mColors[0][i].b;
        }
        else
        {
            vertex.color.x = 1.0f;
            vertex.color.y = 1.0f;
            vertex.color.z = 1.0f;
        }

        if (hasUV)
        {
            // TODO tangent & bitangent
            vertex.uv.x = mesh->mTextureCoords[0][i].x;
            vertex.uv.y = mesh->mTextureCoords[0][i].y;
        }
        else
        {
            vertex.uv.x = 0.0f;
            vertex.uv.y = 0.0f;
        }
    }

    for (uint32_t i = 0; i < mesh->mNumFaces; ++i)
    {
        aiFace face = mesh->mFaces[i];
        for (uint32_t j = 0; j < face.mNumIndices; ++j)
        {
            mMesh->indices.emplace_back(face.mIndices[j]);
        }
    }

    // TODO Materials

    return mMesh;
}

static void Traversal(const aiScene* scene, aiNode* node, std::vector<MMesh*>& meshs)
{
    for (uint32_t i = 0; i < node->mNumMeshes; ++i)
    {
        meshs.emplace_back(TransformMesh(scene, scene->mMeshes[node->mMeshes[i]]));
    }
    for (uint32_t i = 0; i < node->mNumChildren; ++i)
    {
        Traversal(scene, node->mChildren[i], meshs);
    }
}

MModel* LoadModel(const char* path)
{
    std::string realPath = RESOURCES_ROOT_PATH;
    realPath.append(path);
    MModel* model = MORISA_NEW(MModel);
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(
        realPath,
        aiProcess_Triangulate);

    Traversal(scene, scene->mRootNode, model->_meshs);
    
    return model;
}

MModel::MModel()
{
}

MModel::~MModel()
{
    for (MMesh* mesh : _meshs)
    {
        MORISA_DELETE(mesh);
    }
}


MORISA_NAMESPACE_END

