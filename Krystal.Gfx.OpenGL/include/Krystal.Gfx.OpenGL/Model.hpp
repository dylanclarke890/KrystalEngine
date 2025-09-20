#pragma once

#include "Krystal.Gfx.OpenGL/Hooks/gl.hpp"
#include "Krystal.Gfx.OpenGL/Shader.hpp"
#include "Krystal.Gfx.OpenGL/Texture.hpp"
#include "Krystal.Gfx.OpenGL/Utils.hpp"
#include "Krystal.Gfx.OpenGL/VertexArray.hpp"
#include "Krystal.IO/Path.hpp"
#include "Krystal.Lib/List.hpp"
#include "Krystal.Lib/String.hpp"
#include "Krystal.Maths/Vector.hpp"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <fstream>
#include <iostream>

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
    static VertexBufferLayout Layout() noexcept
    {
      return {
        {VertexAttributeType::Float, 3}, // position
        {VertexAttributeType::Float, 3}, // normal
        {VertexAttributeType::Float, 2}, // texcoords
        {VertexAttributeType::Float, 3}, // tangent
        {VertexAttributeType::Float, 3}  // bitangent
      };
    }

    Maths::Vec3 Position;
    Maths::Vec3 Normal;
    Maths::Vec2 TexCoords;
    Maths::Vec3 Tangent;
    Maths::Vec3 Bitangent;
  };

  class Mesh
  {
    List<Vertex> _vertices;
    List<uint> _indices;
    List<MeshTexture> _textures;
    GLuint VAO;
    GLuint VBO, EBO;

  public:
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
      glDrawElementsInstanced(GL_TRIANGLES, static_cast<uint>(_indices.size()), GL_UNSIGNED_INT, 0,
                              instanceCount);

      glBindVertexArray(0);
      glActiveTexture(GL_TEXTURE0);
    }

    GLuint GetVAO() const noexcept
    {
      return VAO;
    }

  private:
    void SetupMesh()
    {
      glCreateVertexArrays(1, &VAO);
      glCreateBuffers(1, &VBO);
      glCreateBuffers(1, &EBO);

      glBindVertexArray(VAO);

      glBindBuffer(GL_ARRAY_BUFFER, VBO);
      glBufferData(GL_ARRAY_BUFFER, _vertices.size() * sizeof(Vertex), &_vertices[0], GL_STATIC_DRAW);

      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, _indices.size() * sizeof(unsigned int), &_indices[0],
                   GL_STATIC_DRAW);

      Utils::ApplyVertexBufferLayout(Vertex::Layout());

      glBindVertexArray(0);
    }

    void Bind(Shader &shader) noexcept
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

  public:
    Model(const IO::Path &path) noexcept : _directory("")
    {
      LoadModel(path);
    }

    void Draw(Shader &shader)
    {
      for (auto &mesh : _meshes)
      {
        mesh.Draw(shader);
      }
    }

    void DrawInstanced(Shader &shader, uint instanceCount)
    {
      for (auto &mesh : _meshes)
      {
        mesh.DrawInstanced(shader, instanceCount);
      }
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

    Maths::Vec2 AssimpVec2DToVec2(const aiVector3D &vec) const noexcept
    {
      return Maths::Vec2(vec.x, vec.y);
    }

    Maths::Vec3 AssimpVec3DToVec3(const aiVector3D &vec) const noexcept
    {
      return Maths::Vec3(vec.x, vec.y, vec.z);
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

        vertex.Position = AssimpVec3DToVec3(mesh->mVertices[i]);

        if (mesh->HasNormals())
        {
          vertex.Normal = AssimpVec3DToVec3(mesh->mNormals[i]);
        }

        // A model can have up to 8 sets of texture coordinates but for now we only care about the first.
        if (mesh->HasTextureCoords(0u))
        {
          vertex.TexCoords = AssimpVec2DToVec2(mesh->mTextureCoords[0][i]);
        }

        if (mesh->HasTangentsAndBitangents())
        {
          vertex.Tangent = AssimpVec3DToVec3(mesh->mTangents[i]);
          vertex.Bitangent = AssimpVec3DToVec3(mesh->mBitangents[i]);
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