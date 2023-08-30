#include "ModelUtils.h"
#include "Utils/MacroUtils.h"
#include "Utils/LogUtils.h"
#include "Core/MString.h"

#include <algorithm>
#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

MORISA_NAMESPACE_BEGIN

static void ProcessMaterial(MMesh* mMesh, MVector<MImageData>& imageDatas, aiMaterial* material)
{
    for (uint32_t i = 0; i <= aiTextureType_TRANSMISSION; ++i)
    {
        uint32_t count = material->GetTextureCount((aiTextureType)i);
        for (uint32_t j = 0; j < count; ++j)
        {
            C_STRUCT aiString path;
            material->GetTexture((aiTextureType)i, j, &path);
            MVector<MImageData>::const_iterator it = std::find_if(
                imageDatas.cbegin(), imageDatas.cend(), [&path](const MImageData& imageData)
                {
                    return imageData.path.compare(path.C_Str()) == 0;
                });
            uint32_t index = std::distance(imageDatas.cbegin(), it);
            if (it == imageDatas.cend())
            {
                imageDatas.emplace_back(MImageData{ path.C_Str(), nullptr });
            }
            mMesh->imageIndices.emplace_back(index);
        }
    }
}

static MMesh* TransformMesh(const aiScene* scene, const aiMesh* mesh, MVector<MImageData>& imageDatas)
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

    ProcessMaterial(mMesh, imageDatas, scene->mMaterials[mesh->mMaterialIndex]);

    return mMesh;
}

static void LoadImages(MVector<MImageData>& imageDatas, MString basePath)
{
    for (MImageData& imageData : imageDatas)
    {
        MString realPath = basePath;
        realPath.append(imageData.path);
        imageData.path = std::move(realPath);
        imageData.image = MORISA_NEW(MImage, imageData.path.c_str());
    }
}

static void Traversal(const aiScene* scene, aiNode* node, 
    MVector<MMesh*>& meshs, MVector<MImageData>& imageDatas)
{
    for (uint32_t i = 0; i < node->mNumMeshes; ++i)
    {
        meshs.emplace_back(TransformMesh(scene, scene->mMeshes[node->mMeshes[i]], imageDatas));
    }
    for (uint32_t i = 0; i < node->mNumChildren; ++i)
    {
        Traversal(scene, node->mChildren[i], meshs, imageDatas);
    }
}

MModel* LoadModel(const char* path)
{
    MString realPath = globalConfig.resourcesRootPath;
    realPath.append(path);
    MModel* model = MORISA_NEW(MModel);
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(
        realPath,
        aiProcess_JoinIdenticalVertices
        |aiProcess_Triangulate
        |aiProcess_GenNormals
        |aiProcess_GenUVCoords);

    Traversal(scene, scene->mRootNode, model->_meshs, model->_imageDatas);

    MString basePath = MString(path);
    basePath = basePath.substr(0, basePath.rfind('/') + 1);
    LoadImages(model->_imageDatas, basePath);

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


#pragma region Default Mesh Data
MMesh mPlaneMesh
{
    {
        {{-1.0f, 0.0f,  1.0f},{0.0f, 1.0f, 0.0f},{1.0f, 1.0f, 1.0f},{0.0f, 0.0f}},
        {{ 1.0f, 0.0f,  1.0f},{0.0f, 1.0f, 0.0f},{1.0f, 1.0f, 1.0f},{1.0f, 0.0f}},
        {{ 1.0f, 0.0f, -1.0f},{0.0f, 1.0f, 0.0f},{1.0f, 1.0f, 1.0f},{1.0f, 1.0f}},
        {{-1.0f, 0.0f, -1.0f},{0.0f, 1.0f, 0.0f},{1.0f, 1.0f, 1.0f},{0.0f, 1.0f}},
    },
    {
        0,1,2,
        0,2,3,
    },
};

MMesh mQuadMesh
{
    {
        {{-1.0f, -1.0f, 0.0f},{0.0f, 0.0f, 1.0f},{1.0f, 1.0f, 1.0f},{0.0f, 0.0f}},
        {{ 1.0f, -1.0f, 0.0f},{0.0f, 0.0f, 1.0f},{1.0f, 1.0f, 1.0f},{1.0f, 0.0f}},
        {{ 1.0f,  1.0f, 0.0f},{0.0f, 0.0f, 1.0f},{1.0f, 1.0f, 1.0f},{1.0f, 1.0f}},
        {{-1.0f,  1.0f, 0.0f},{0.0f, 0.0f, 1.0f},{1.0f, 1.0f, 1.0f},{0.0f, 1.0f}},
    },
    {
        0,1,2,
        0,2,3,
    },
};

MMesh mCubeMesh
{
   {
        // +Z
        {{-1.0f, -1.0f, 1.0f},{0.0f, 0.0f, 1.0f},{1.0f, 1.0f, 1.0f},{0.0f, 0.0f}},
        {{ 1.0f, -1.0f, 1.0f},{0.0f, 0.0f, 1.0f},{1.0f, 1.0f, 1.0f},{1.0f, 0.0f}},
        {{ 1.0f,  1.0f, 1.0f},{0.0f, 0.0f, 1.0f},{1.0f, 1.0f, 1.0f},{1.0f, 1.0f}},
        {{-1.0f,  1.0f, 1.0f},{0.0f, 0.0f, 1.0f},{1.0f, 1.0f, 1.0f},{0.0f, 1.0f}},

        //  -Z
        {{-1.0f, -1.0f, -1.0f},{0.0f, 0.0f, -1.0f},{1.0f, 1.0f, 1.0f},{1.0f, 0.0f}},
        {{ 1.0f, -1.0f, -1.0f},{0.0f, 0.0f, -1.0f},{1.0f, 1.0f, 1.0f},{0.0f, 0.0f}},
        {{ 1.0f,  1.0f, -1.0f},{0.0f, 0.0f, -1.0f},{1.0f, 1.0f, 1.0f},{0.0f, 1.0f}},
        {{-1.0f,  1.0f, -1.0f},{0.0f, 0.0f, -1.0f},{1.0f, 1.0f, 1.0f},{1.0f, 1.0f}},

        // +X
        {{1.0f, -1.0f,  1.0f},{1.0f, 0.0f, 0.0f},{1.0f, 1.0f, 1.0f},{0.0f, 0.0f}},
        {{1.0f, -1.0f, -1.0f},{1.0f, 0.0f, 0.0f},{1.0f, 1.0f, 1.0f},{1.0f, 0.0f}},
        {{1.0f,  1.0f, -1.0f},{1.0f, 0.0f, 0.0f},{1.0f, 1.0f, 1.0f},{1.0f, 1.0f}},
        {{1.0f,  1.0f,  1.0f},{1.0f, 0.0f, 0.0f},{1.0f, 1.0f, 1.0f},{0.0f, 1.0f}},

        // -X
        {{-1.0f, -1.0f,  1.0f},{-1.0f, 0.0f, 0.0f},{1.0f, 1.0f, 1.0f},{1.0f, 0.0f}},
        {{-1.0f, -1.0f, -1.0f},{-1.0f, 0.0f, 0.0f},{1.0f, 1.0f, 1.0f},{0.0f, 0.0f}},
        {{-1.0f,  1.0f, -1.0f},{-1.0f, 0.0f, 0.0f},{1.0f, 1.0f, 1.0f},{0.0f, 1.0f}},
        {{-1.0f,  1.0f,  1.0f},{-1.0f, 0.0f, 0.0f},{1.0f, 1.0f, 1.0f},{1.0f, 1.0f}},

        // +Y
        {{-1.0f, 1.0f,  1.0f},{0.0f, 1.0f, 0.0f},{1.0f, 1.0f, 1.0f},{0.0f, 0.0f}},
        {{ 1.0f, 1.0f,  1.0f},{0.0f, 1.0f, 0.0f},{1.0f, 1.0f, 1.0f},{1.0f, 0.0f}},
        {{ 1.0f, 1.0f, -1.0f},{0.0f, 1.0f, 0.0f},{1.0f, 1.0f, 1.0f},{1.0f, 1.0f}},
        {{-1.0f, 1.0f, -1.0f},{0.0f, 1.0f, 0.0f},{1.0f, 1.0f, 1.0f},{0.0f, 1.0f}},

        // -Y
        {{-1.0f, -1.0f,  1.0f},{0.0f, -1.0f, 0.0f},{1.0f, 1.0f, 1.0f},{1.0f, 0.0f}},
        {{ 1.0f, -1.0f,  1.0f},{0.0f, -1.0f, 0.0f},{1.0f, 1.0f, 1.0f},{0.0f, 0.0f}},
        {{ 1.0f, -1.0f, -1.0f},{0.0f, -1.0f, 0.0f},{1.0f, 1.0f, 1.0f},{0.0f, 1.0f}},
        {{-1.0f, -1.0f, -1.0f},{0.0f, -1.0f, 0.0f},{1.0f, 1.0f, 1.0f},{1.0f, 1.0f}},
   },

    {
        0,1,2,0,2,3,
        4,6,5,4,7,6,
        8,9,10,8,10,11,
        12,14,13,12,15,14,
        16,17,18,16,18,19,
        20,22,21,20,23,22,
    },
};
#pragma endregion

MORISA_NAMESPACE_END

