#include "pch.h"

#include "assimp.h"

#include "FlexMath/vector3.h"
#include "AssetManager/assetmanager.h"

namespace
{
  // Internal variables

  // The current working directory of AssimpWrapper.
  // This is used to resolve relative paths of textures when creating the asset keys
  // for the textures.
  static FlexEngine::Path internal_current_working_directory = FlexEngine::Path::current();
  #define CURRENT_WORKING_DIRECTORY internal_current_working_directory

  // Internal functions

  FlexEngine::Matrix4x4 Internal_ConvertMatrix(const aiMatrix4x4& matrix)
  {
    FlexEngine::Matrix4x4 out = {
      matrix.a1, matrix.a2, matrix.a3, matrix.a4,
      matrix.b1, matrix.b2, matrix.b3, matrix.b4,
      matrix.c1, matrix.c2, matrix.c3, matrix.c4,
      matrix.d1, matrix.d2, matrix.d3, matrix.d4
    };

    return out.Transpose();
  }
}

namespace FlexEngine
{

  // static member initialization
  Assimp::Importer AssimpWrapper::importer;
  unsigned int AssimpWrapper::import_flags = aiProcess_ValidateDataStructure | aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace;

  Asset::Model AssimpWrapper::LoadModel(const Path& path)
  {
    // load the model
    const aiScene* scene = importer.ReadFile(path.string().c_str(), import_flags);
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
      Log::Error(std::string("Assimp import error: ") + importer.GetErrorString());
      return Asset::Model();
    }

    // set the current working directory
    CURRENT_WORKING_DIRECTORY = path.parent_path();

    // flip the model's y and z axis
    Matrix4x4 root_transform = Matrix4x4::Identity;
    root_transform(1, 1) = -1.0f;
    root_transform(2, 2) = -1.0f;

    // process the scene
    return Asset::Model( Internal_ProcessNode(scene->mRootNode, scene, root_transform) );
  }

  std::vector<Asset::Mesh> AssimpWrapper::Internal_ProcessNode(aiNode* node, const aiScene* scene, Matrix4x4 parent_transform)
  {
    Log::Flow("Processing node: " + std::string(node->mName.C_Str()));
    //Log::Debug("Node has " + std::to_string(node->mNumMeshes) + " meshes and " + std::to_string(node->mNumChildren) + " children.");

    std::vector<Asset::Mesh> meshes;

    Matrix4x4 transform = Matrix4x4::Identity;
    transform = parent_transform * Internal_ConvertMatrix(node->mTransformation);

    // process all the meshes in the node
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
      aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
      meshes.push_back( Internal_ProcessMesh(mesh, scene, transform) );
    }

    // process all the children of the node
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
      std::vector<Asset::Mesh> child( Internal_ProcessNode(node->mChildren[i], scene, transform) );
      meshes.insert(meshes.end(), child.begin(), child.end());
    }

    return meshes;
  }

  Asset::Mesh AssimpWrapper::Internal_ProcessMesh(aiMesh* mesh, const aiScene* scene, Matrix4x4 mesh_transform)
  {
    #pragma region Vertex Processing

    std::vector<Vertex> vertices;
    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
      Vertex vertex;
      vertex.position = Vector3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);

      if (mesh->HasNormals())
      {
        vertex.normal = Vector3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
      }
      else
      {
        vertex.normal = Vector3::Zero;
      }

      // this implementation only supports one set of texture coordinates
      if (mesh->HasTextureCoords(0))
      {
        vertex.tex_coords = Vector2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
      }
      else
      {
        vertex.tex_coords = Vector2::Zero;
      }

      vertices.push_back(vertex);
    }

    #pragma endregion

    #pragma region Index Processing

    std::vector<unsigned int> indices;
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
      aiFace face = mesh->mFaces[i];
      for (unsigned int j = 0; j < face.mNumIndices; j++)
      {
        indices.push_back(face.mIndices[j]);
      }
    }

    #pragma endregion

    #pragma region Material Processing

    aiMaterial* ai_material = scene->mMaterials[mesh->mMaterialIndex];

    Asset::Material material(
      Internal_ProcessMaterialTextures(ai_material, aiTextureType_DIFFUSE , scene),
      Internal_ProcessMaterialTextures(ai_material, aiTextureType_SPECULAR, scene)
    );

    #pragma endregion

    // create the mesh
    return Asset::Mesh(vertices, indices, material, mesh_transform);
  }

  // Only supports diffuse and specular textures, one of each
  Asset::Material::TextureVariant AssimpWrapper::Internal_ProcessMaterialTextures(aiMaterial* material, aiTextureType type, const aiScene* scene)
  {
    Asset::Material::TextureVariant textures = Asset::Texture::Default;

    // process the material
    if (material->GetTextureCount(type) > 0)
    {
      aiString texture_path;
      material->GetTexture(type, 0, &texture_path);

      const aiTexture* texture = scene->GetEmbeddedTexture(texture_path.C_Str());
      if (texture)
      {
        // guard: texture is compressed
        // use stb_image to decompress the texture
        if (texture->mHeight == 0)
        {
          Log::Error("AssimpWrapper: Embedded textures are compressed, decompression is not supported.");
          return textures;
        }

        // create a buffer for the texture data
        std::size_t size = texture->mWidth * texture->mHeight * 4;
        unsigned char* data = new unsigned char[size];

        // ARGB -> RGBA
        for (std::size_t j = 0; j < size; j += 4)
        {
          data[j + 0] = texture->pcData[j].r;
          data[j + 1] = texture->pcData[j].g;
          data[j + 2] = texture->pcData[j].b;
          data[j + 3] = texture->pcData[j].a;
        }

        // embed the texture
        Asset::Texture embedded_texture(data, texture->mWidth, texture->mHeight);

        // cleanup
        delete[] data;

        // add the texture to the list of textures
        textures = embedded_texture;
      }
      else
      {
        // guard: texture is not a file
        if (texture_path.length == 0)
        {
          Log::Error("AssimpWrapper: Texture path is empty.");
          return textures;
        }
        
        // create an asset key for the texture
        // TODO: add checks for the texture path
        // currently, the texture path is assumed to be relative to the model path
        Path full_path;
        try
        {
          full_path = CURRENT_WORKING_DIRECTORY.append(texture_path.C_Str());
        }
        catch (std::exception e)
        {
          Log::Error(e.what());
          return textures;
        }

        // remove the default directory from the asset key
        AssetKey assetkey = full_path.string().substr(AssetManager::DefaultDirectory().string().length());
        
        // add the asset key to the list of textures
        textures = assetkey;
      }
      
    }
    
    return textures;
  }

  //Asset::Material::TextureVariant AssimpWrapper::Internal_ProcessMaterialTextures(aiMaterial* material, aiTextureType type, const aiScene* scene)
  //{
  //  Asset::Material::TextureVariant textures;
  //
  //  // process all the textures of the material
  //  for (unsigned int i = 0; i < material->GetTextureCount(type); i++)
  //  {
  //    aiString texture_path;
  //    material->GetTexture(type, i, &texture_path);
  //
  //    const aiTexture* texture = scene->GetEmbeddedTexture(texture_path.C_Str());
  //    if (texture)
  //    {
  //      // guard: texture is compressed, idk what to do with this
  //      if (texture->mHeight == 0) continue;
  //
  //      // create a buffer for the texture data
  //      std::size_t size = texture->mWidth * texture->mHeight * 4;
  //      unsigned char* data = new unsigned char[size];
  //
  //      // ARGB -> RGBA
  //      for (std::size_t j = 0; j < size; j += 4)
  //      {
  //        data[j + 0] = texture->pcData[j].r;
  //        data[j + 1] = texture->pcData[j].g;
  //        data[j + 2] = texture->pcData[j].b;
  //        data[j + 3] = texture->pcData[j].a;
  //      }
  //
  //      // embed the texture
  //      Asset::Texture embedded_texture(data, texture->mWidth, texture->mHeight);
  //
  //      // cleanup
  //      delete[] data;
  //
  //      // add the texture to the list of textures
  //      textures.push_back(embedded_texture);
  //    }
  //    else
  //    {
  //      // guard: texture is not a file
  //      if (texture_path.length == 0) continue;
  //
  //      // create an asset key for the texture
  //      // TODO: add checks for the texture path
  //      // currently, the texture path is assumed to be relative to the model path
  //      Path full_path = CURRENT_WORKING_DIRECTORY.append(texture_path.C_Str());
  //
  //      // add the asset key to the list of textures
  //      textures.push_back( AssetKey(full_path.string()) );
  //    }
  //
  //  }
  //
  //  return textures;
  //}

}