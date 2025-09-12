#pragma once

#include "Krystal.Gfx.OpenGL/Hooks/gl.hpp"
#include "Krystal.Gfx.OpenGL/Shader.hpp"
#include "Krystal.Gfx.OpenGL/Texture.hpp"
#include "Krystal.Gfx.OpenGL/Utils.hpp"
#include "Krystal.IO/Path.hpp"
#include "Krystal.Maths/Vector.hpp"

#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

namespace Krys::Gfx::OpenGL
{
  enum class TextureType
  {
    Diffuse,
    Specular,
    Normal,
    Height
  };

  struct MeshTexture
  {
    Texture2D *Texture;
    TextureType Type;
  };

  struct ModelTexture
  {
    TextureType Type;
    string Path;
    Unique<Texture2D> Texture;
  };

  struct Vertex
  {
    Maths::Vec3 Position;
    Maths::Vec3 Normal;
    Maths::Vec2 TexCoords;
    Maths::Vec3 Tangent;
    Maths::Vec3 Bitangent;
  };

  class Mesh
  {
  public:
    List<Vertex> _vertices;
    List<uint> _indices;
    List<MeshTexture> _textures;
    GLuint VAO;

    Mesh(List<Vertex> vertices, List<uint> indices, List<MeshTexture> textures) noexcept
        : _vertices(vertices), _indices(indices), _textures(textures)
    {
      SetupMesh();
    }

    void Draw(Shader &shader) noexcept
    {
      Bind(shader);

      // draw mesh
      glDrawElements(GL_TRIANGLES, static_cast<uint>(_indices.size()), GL_UNSIGNED_INT, 0);
      
      glBindVertexArray(0);
      glActiveTexture(GL_TEXTURE0);
    }

    void DrawInstanced(Shader &shader, uint instanceCount) noexcept
    {
      Bind(shader);

      // draw mesh
      glDrawElementsInstanced(GL_TRIANGLES, static_cast<uint>(_indices.size()), GL_UNSIGNED_INT, 0, instanceCount);

      glBindVertexArray(0);
      glActiveTexture(GL_TEXTURE0);
    }

    GLuint GetVAO() const noexcept
    {
      return VAO;
    }

  private:
    // render data
    uint VBO, EBO;

    // initializes all the buffer objects/arrays
    void SetupMesh()
    {
      // create buffers/arrays
      glGenVertexArrays(1, &VAO);
      glGenBuffers(1, &VBO);
      glGenBuffers(1, &EBO);

      glBindVertexArray(VAO);

      // load data into vertex buffers
      glBindBuffer(GL_ARRAY_BUFFER, VBO);

      // A great thing about structs is that their memory layout is sequential for all its items.
      // The effect is that we can simply pass a pointer to the struct and it translates perfectly to a
      // Maths::Vec3/2 array which again translates to 3/2 floats which translates to a byte array.
      glBufferData(GL_ARRAY_BUFFER, _vertices.size() * sizeof(Vertex), &_vertices[0], GL_STATIC_DRAW);

      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, _indices.size() * sizeof(unsigned int), &_indices[0],
                   GL_STATIC_DRAW);

      // set the vertex attribute pointers

      // vertex Positions
      glEnableVertexAttribArray(0);
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)0);

      // vertex normals
      glEnableVertexAttribArray(1);
      glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, Normal));

      // vertex texture coords
      glEnableVertexAttribArray(2);
      glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, TexCoords));

      // vertex tangent
      glEnableVertexAttribArray(3);
      glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, Tangent));

      // vertex bitangent
      glEnableVertexAttribArray(4);
      glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, Bitangent));

      glBindVertexArray(0);
    }
  
    void Bind(Shader& shader) noexcept
    {
      // bind appropriate textures
      uint diffuseNr = 1;
      uint specularNr = 1;
      uint normalNr = 1;
      uint heightNr = 1;
      for (uint i = 0; i < _textures.size(); i++)
      {
        _textures[i].Texture->Bind(i);

        // retrieve texture number (the N in diffuse_textureN)
        string number;
        string name;
        switch (_textures[i].Type)
        {
          case TextureType::Diffuse:
            number = std::to_string(diffuseNr++);
            name = "texture_diffuse";
            break;
          case TextureType::Specular:
            number = std::to_string(specularNr++);
            name = "texture_specular";
            break;
          case TextureType::Normal:
            number = std::to_string(normalNr++);
            name = "texture_normal";
            break;
          case TextureType::Height:
            number = std::to_string(heightNr++);
            name = "texture_height";
            break;
          default: break;
        }

        // now set the sampler to the correct texture unit
        shader.SetUniform(name + number, i);
        // and finally bind the texture
      }

      glBindVertexArray(VAO);
    }
  };

  class Model
  {
  private:
    Map<string, Unique<Texture2D>> _loadedTextures;
    List<Mesh> _meshes;
    IO::Path _directory;
    bool _gammaCorrection;

  public:
    Model(const IO::Path &path, bool gamma = false) : _gammaCorrection(gamma), _directory("")
    {
      LoadModel(path);
    }

    void Draw(Shader &shader)
    {
      for (uint i = 0; i < _meshes.size(); i++)
        _meshes[i].Draw(shader);
    }

    void DrawInstanced(Shader &shader, uint instanceCount)
    {
      for (uint i = 0; i < _meshes.size(); i++)
        _meshes[i].DrawInstanced(shader, instanceCount);
    }

    void ApplyVertexLayout(VertexBufferLayout layout, uint32 attributeIndexOffset = 0u) noexcept
    {
      const uint32 modelAttributeCount = 5; // position, normal, texcoords, tangent, bitangent
      for (uint i = 0; i < _meshes.size(); i++)
      {
        glBindVertexArray(_meshes[i].GetVAO());
        Utils::ApplyVertexBufferLayout(layout, attributeIndexOffset + modelAttributeCount);
      }
    }

  private:
    void LoadModel(const IO::Path &path)
    {
      // read file via ASSIMP
      Assimp::Importer importer {};
      const aiScene *scene =
        importer.ReadFile(path.ToString(), aiProcess_Triangulate | aiProcess_GenSmoothNormals
                                             | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
      // check for errors
      if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
      {
        std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
        return;
      }

      // retrieve the directory path of the filepath
      _directory = path.ParentPath();

      // process ASSIMP's root node recursively
      ProcessNode(scene->mRootNode, scene);
    }

    // processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats
    // this process on its children nodes (if any).
    void ProcessNode(aiNode *node, const aiScene *scene)
    {
      // process each mesh located at the current node
      for (uint i = 0; i < node->mNumMeshes; i++)
      {
        // the node object only contains indices to index the actual objects in the scene.
        // the scene contains all the data, node is just to keep stuff organized (like relations between
        // nodes).
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        _meshes.push_back(ProcessMesh(mesh, scene));
      }
      // after we've processed all of the meshes (if any) we then recursively process each of the children
      // nodes
      for (uint i = 0; i < node->mNumChildren; i++)
      {
        ProcessNode(node->mChildren[i], scene);
      }
    }

    Mesh ProcessMesh(aiMesh *mesh, const aiScene *scene)
    {
      List<Vertex> vertices;
      List<uint> indices;
      List<MeshTexture> textures;

      // walk through each of the mesh's vertices
      for (uint i = 0; i < mesh->mNumVertices; i++)
      {
        Vertex vertex;
        Maths::Vec3 vector; // we declare a placeholder vector since assimp uses its own vector class that
                            // doesn't directly convert to our vec3 class so we transfer the data to this
                            // placeholder Maths::Vec3 first.
        // positions
        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z;
        vertex.Position = vector;

        // normals
        if (mesh->HasNormals())
        {
          vector.x = mesh->mNormals[i].x;
          vector.y = mesh->mNormals[i].y;
          vector.z = mesh->mNormals[i].z;
          vertex.Normal = vector;
        }

        // texture coordinates
        if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
        {
          Maths::Vec2 vec;

          // a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we
          // won't use models where a vertex can have multiple texture coordinates so we always take the first
          // set (0).
          vec.x = mesh->mTextureCoords[0][i].x;
          vec.y = mesh->mTextureCoords[0][i].y;
          vertex.TexCoords = vec;

          // tangent
          vector.x = mesh->mTangents[i].x;
          vector.y = mesh->mTangents[i].y;
          vector.z = mesh->mTangents[i].z;
          vertex.Tangent = vector;

          // bitangent
          vector.x = mesh->mBitangents[i].x;
          vector.y = mesh->mBitangents[i].y;
          vector.z = mesh->mBitangents[i].z;
          vertex.Bitangent = vector;
        }
        else
        {
          vertex.TexCoords = Maths::Vec2(0.0f, 0.0f);
        }

        vertices.push_back(vertex);
      }

      // now walk through each of the mesh's faces (a face is a mesh its triangle) and retrieve the
      // corresponding vertex indices.
      for (uint i = 0; i < mesh->mNumFaces; i++)
      {
        aiFace face = mesh->mFaces[i];
        // retrieve all indices of the face and store them in the indices vector
        for (uint j = 0; j < face.mNumIndices; j++)
          indices.push_back(face.mIndices[j]);
      }

      // process materials
      aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
      // we assume a convention for sampler names in the shaders. Each diffuse texture should be named
      // as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER.
      // Same applies to other texture as the following list summarizes:
      // diffuse: texture_diffuseN
      // specular: texture_specularN
      // normal: texture_normalN

      // 1. diffuse maps
      List<MeshTexture> diffuseMaps =
        LoadMaterialTextures(material, aiTextureType_DIFFUSE, TextureType::Diffuse);
      textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

      // 2. specular maps
      List<MeshTexture> specularMaps =
        LoadMaterialTextures(material, aiTextureType_SPECULAR, TextureType::Specular);
      textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

      // 3. normal maps
      List<MeshTexture> normalMaps =
        LoadMaterialTextures(material, aiTextureType_NORMALS, TextureType::Normal);
      textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());

      // 4. height maps
      List<MeshTexture> heightMaps =
        LoadMaterialTextures(material, aiTextureType_HEIGHT, TextureType::Height);
      textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

      // return a mesh object created from the extracted mesh data
      return Mesh(vertices, indices, textures);
    }

    // checks all material textures of a given type and loads the textures if they're not loaded yet.
    // the required info is returned as a Texture struct.
    List<MeshTexture> LoadMaterialTextures(aiMaterial *mat, aiTextureType type, TextureType textureType)
    {
      List<MeshTexture> textures;

      for (uint i = 0; i < mat->GetTextureCount(type); i++)
      {
        aiString str;
        mat->GetTexture(type, i, &str);
        auto path = IO::Path(string(str.C_Str()));

        if (auto it = _loadedTextures.find(path.ToString()); it != _loadedTextures.end())
        {
          auto &modelTexture = it->second;
          textures.push_back({modelTexture.get(), textureType});
        }
        else
        {
          auto fullPath = _directory / path;
          _loadedTextures[path.ToString()] = CreateUnique<Texture2D>(fullPath);
          textures.push_back({_loadedTextures[path.ToString()].get(), textureType});
        }
      }
      return textures;
    }
  };
}