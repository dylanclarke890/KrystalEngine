#pragma once

#include "Krystal.Gfx/ResourceManager.hpp"
#include "Krystal.Gfx.OpenGL/Debug.hpp"
#include "Krystal.Gfx.OpenGL/Mappers/Enums/PrimitiveType.hpp"
#include "Krystal.Gfx.OpenGL/Resources/Mesh.hpp"
#include "Krystal.Gfx/Registries/IMeshRegistry.hpp"
#include "Krystal.Gfx/Utils/MeshDataUtils.hpp"
#include "Krystal.Lib/ByteUtils.hpp"
#include "Krystal.Lib/Types/List.hpp"
#include "Krystal.Lib/Core/Macros.hpp"
#include "Krystal.Maths/Vector.hpp"

namespace Krys::Gfx::OpenGL
{
  class MeshRegistry final : public IMeshRegistry
  {
    NO_COPY_MOVE(MeshRegistry)

    using MeshManager = ResourceManager<Mesh, MeshHandle>;

  private:
    MeshManager _meshes;
    MeshHandle _fullScreenQuadHandle;

  public:
    MeshRegistry() = default;

    ~MeshRegistry() noexcept override = default;

    void Startup() override
    {
      using namespace Maths;

      MeshData data {};
      MeshDataUtils::GenerateQuad(data, Vec2 {-1.f}, Vec2 {2.f}, Colours::Transparent);

      _fullScreenQuadHandle = Create({
        .Vertices = data.Vertices,
        .Indices = data.Indices,
        .Layout = Vertex::Position2D_ColourbPremultiplied_UV::Layout(),
        .Primitive = PrimitiveType::Triangles,
        .Type = MeshType::Static,
      });

      Debug::SetName(_meshes.Get(_fullScreenQuadHandle), "FullScreenQuad");
    }

    void Shutdown() noexcept override
    {
      _meshes.Remove(_fullScreenQuadHandle);
    }

    KRYS_NODISCARD MeshHandle Create(const MeshDesc &desc) noexcept override
    {
      GLenum primitiveType = MapPrimitiveType(desc.Primitive);

      if (desc.Indices.size_bytes() > 0u)
      {
        return AddMesh(Mesh {desc.Vertices, desc.Indices, desc.Layout, primitiveType, desc.Type});
      }

      return AddMesh(Mesh {desc.Vertices, desc.Layout, primitiveType, desc.Type});
    }

    KRYS_NODISCARD MeshHandle GetFullScreenQuad() const noexcept override
    {
      return _fullScreenQuadHandle;
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

      return Create({
        .Vertices = ByteUtils::AsBytesView(vertices),
        .Layout = Vertex::Position3D_Normal_UV::Layout(),
        .Primitive = PrimitiveType::Triangles,
        .Type = MeshType::Static,
      });
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

      return Create({
        .Vertices = ByteUtils::AsBytesView(vertices),
        .Indices = ByteUtils::AsBytesView(indices),
        .Layout = Vertex::Position3D_Normal_UV::Layout(),
        .Primitive = PrimitiveType::TriangleStrip,
        .Type = MeshType::Static,
      });
    }

    MeshHandle CreateScreenQuad() noexcept
    {
      List<float> vertices = {
        // positions   // texCoords
        -1.0f, 1.0f, 0.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 0.0f,
      };

      VertexBufferLayout layout = Vertex::Position2D_UV::Layout();

      return Create({
        .Vertices = ByteUtils::AsBytesView(vertices),
        .Layout = layout,
        .Primitive = PrimitiveType::TriangleStrip,
        .Type = MeshType::Static,
      });
    }

    MeshHandle CreateQuad() noexcept
    {
      using QuadVertex = Vertex::Position2D_UV;
      Array<QuadVertex, 4> vertices = {
        QuadVertex {{0.0f, 0.0f}, {0.0f, 0.0f}},
        QuadVertex {{1.0f, 0.0f}, {1.0f, 0.0f}},
        QuadVertex {{1.0f, 1.0f}, {1.0f, 1.0f}},
        QuadVertex {{0.0f, 1.0f}, {0.0f, 1.0f}},
      };

      Array<uint32, 6> indices {0, 1, 2, 2, 3, 0};

      return Create({
        .Vertices = ByteUtils::AsBytesView(vertices),
        .Indices = ByteUtils::AsBytesView(indices),
        .Layout = QuadVertex::Layout(),
        .Primitive = PrimitiveType::Triangles,
        .Type = MeshType::Static,
      });
    }

    bool Destroy(MeshHandle handle) noexcept override
    {
      if (!handle.IsValid())
      {
        return false;
      }
      return _meshes.Remove(handle);
    }

    Mesh &Get(MeshHandle handle) noexcept
    {
      return _meshes.Get(handle);
    }

  private:
    KRYS_NODISCARD MeshHandle AddMesh(Mesh &&mesh) noexcept
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