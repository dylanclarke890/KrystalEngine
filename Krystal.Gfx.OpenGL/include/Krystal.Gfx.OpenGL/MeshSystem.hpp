#pragma once

#include "Krystal.Gfx.OpenGL/Mesh.hpp"
#include "Krystal.Gfx/IMeshSystem.hpp"
#include "Krystal.Gfx/ResourceManager.hpp"
#include "Krystal.Lib/List.hpp"
#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Maths/Vector.hpp"

namespace Krys::Gfx::OpenGL
{
  class MeshSystem final : public IMeshSystem
  {
    NO_COPY_MOVE(MeshSystem)

    using MeshManager = ResourceManager<Mesh, MeshHandle>;

  private:
    MeshManager _meshes;

  public:
    MeshSystem() = default;

    ~MeshSystem() noexcept override = default;

    virtual MeshHandle Create(const Span<const byte> &vertexBuffer, const VertexBufferLayout &layout,
                              PrimitiveType type) noexcept override
    {
      GLenum primitiveType = MapPrimitiveType(type);
      Mesh mesh {vertexBuffer, layout, primitiveType};
      return AddMesh(std::move(mesh));
    }

    virtual MeshHandle Create(const Span<const byte> &vertexBuffer, const Span<const byte> &indexBuffer,
                              const VertexBufferLayout &layout, PrimitiveType type) noexcept override
    {
      GLenum primitiveType = MapPrimitiveType(type);
      Mesh mesh {vertexBuffer, indexBuffer, layout, primitiveType};
      return AddMesh(std::move(mesh));
    }

    MeshHandle CreateCube() noexcept
    {
      List<float> vertices = {
        // back face
        -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
        1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,   // top-right
        1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f,  // bottom-right
        1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,   // top-right
        -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
        -1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f,  // top-left
        // front face
        -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom-left
        1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,  // bottom-right
        1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,   // top-right
        1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,   // top-right
        -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,  // top-left
        -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom-left
        // left face
        -1.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,   // top-right
        -1.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f,  // top-left
        -1.0f, -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-left
        -1.0f, -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-left
        -1.0f, -1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // bottom-right
        -1.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,   // top-right
                                                            // right face
        1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,     // top-left
        1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,   // bottom-right
        1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,    // top-right
        1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,   // bottom-right
        1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,     // top-left
        1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,    // bottom-left
        // bottom face
        -1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, // top-right
        1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f,  // top-left
        1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,   // bottom-left
        1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,   // bottom-left
        -1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f,  // bottom-right
        -1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, // top-right
        // top face
        -1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, // top-left
        1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,   // bottom-right
        1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,  // top-right
        1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,   // bottom-right
        -1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, // top-left
        -1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f   // bottom-left
      };

      VertexBufferLayout layout = {
        {VertexAttributeType::Float, 3}, // position
        {VertexAttributeType::Float, 3}, // normal
        {VertexAttributeType::Float, 2}  // texcoord
      };

      return Create(ByteUtils::AsBytesView(vertices), layout, PrimitiveType::Triangles);
    }

    MeshHandle CreateSphere() noexcept
    {
      List<Maths::Vec3> positions;
      List<Maths::Vec2> uv;
      List<Maths::Vec3> normals;
      List<uint> indices;

      const uint X_SEGMENTS = 64;
      const uint Y_SEGMENTS = 64;
      const float PI = 3.14159265359f;
      for (uint x = 0; x <= X_SEGMENTS; ++x)
      {
        for (uint y = 0; y <= Y_SEGMENTS; ++y)
        {
          float xSegment = (float)x / (float)X_SEGMENTS;
          float ySegment = (float)y / (float)Y_SEGMENTS;
          float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
          float yPos = std::cos(ySegment * PI);
          float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);

          positions.push_back({xPos, yPos, zPos});
          uv.push_back({xSegment, ySegment});
          normals.push_back({xPos, yPos, zPos});
        }
      }

      bool oddRow = false;
      for (uint y = 0; y < Y_SEGMENTS; ++y)
      {
        if (!oddRow) // even rows: y == 0, y == 2; and so on
        {
          for (uint x = 0; x <= X_SEGMENTS; ++x)
          {
            indices.push_back(y * (X_SEGMENTS + 1) + x);
            indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
          }
        }
        else
        {
          for (int x = X_SEGMENTS; x >= 0; --x)
          {
            indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
            indices.push_back(y * (X_SEGMENTS + 1) + x);
          }
        }
        oddRow = !oddRow;
      }

      List<float> vertices;
      for (uint i = 0; i < positions.size(); ++i)
      {
        vertices.push_back(positions[i].x);
        vertices.push_back(positions[i].y);
        vertices.push_back(positions[i].z);
        if (normals.size() > 0)
        {
          vertices.push_back(normals[i].x);
          vertices.push_back(normals[i].y);
          vertices.push_back(normals[i].z);
        }
        if (uv.size() > 0)
        {
          vertices.push_back(uv[i].x);
          vertices.push_back(uv[i].y);
        }
      }

      VertexBufferLayout layout = {
        {VertexAttributeType::Float, 3}, // position
        {VertexAttributeType::Float, 3}, // normal
        {VertexAttributeType::Float, 2}  // texcoord
      };

      return Create(ByteUtils::AsBytesView(vertices), ByteUtils::AsBytesView(indices), layout,
                    PrimitiveType::TriangleStrip);
    }

    MeshHandle CreateScreenQuad() noexcept
    {
      List<float> vertices = {
        // positions   // texCoords
        -1.0f, 1.0f, 0.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 0.0f,
      };

      VertexBufferLayout layout = {
        {VertexAttributeType::Float, 2}, // position
        {VertexAttributeType::Float, 2}  // texcoord
      };

      return Create(ByteUtils::AsBytesView(vertices), layout, PrimitiveType::TriangleStrip);
    }

    void Unload(MeshHandle handle) noexcept override
    {
      assert(handle.IsValid() && "Invalid handle.");
      _meshes.Remove(handle);
    }

    Mesh &Get(MeshHandle handle) noexcept
    {
      return _meshes.Get(handle);
    }

  private:
    static GLenum MapPrimitiveType(PrimitiveType type) noexcept
    {
      switch (type)
      {
        case PrimitiveType::Points:        return GL_POINTS;
        case PrimitiveType::Lines:         return GL_LINES;
        case PrimitiveType::LineStrip:     return GL_LINE_STRIP;
        case PrimitiveType::Triangles:     return GL_TRIANGLES;
        case PrimitiveType::TriangleStrip: return GL_TRIANGLE_STRIP;
        case PrimitiveType::TriangleFan:   return GL_TRIANGLE_FAN;
        default:                           return GL_TRIANGLES;
      }
    }

    NO_DISCARD MeshHandle AddMesh(Mesh &&mesh) noexcept
    {
      auto handle = _meshes.Add(std::move(mesh));
      if (!handle.IsValid())
      {
        return {};
      }
      return handle;
    }
  };
}