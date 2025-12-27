#pragma once

#include "Krystal.Gfx.OpenGL/gl.hpp"
#include "Krystal.Gfx.OpenGL/Utils.hpp"
#include "Krystal.Gfx/Resources/Mesh.hpp"
#include "Krystal.Gfx/Vertex.hpp"
#include "Krystal.Lib/Core/Macros.hpp"
#include "Krystal.Lib/Types/Span.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"

namespace Krys::Gfx::OpenGL
{
  class Mesh
  {
    NO_COPY(Mesh)

    GLuint _vao {0u};
    GLuint _vbo {0u};
    GLuint _ebo {0u};

    VertexBufferLayout _layout {};
    GLenum _primitiveType {GL_TRIANGLES};
    MeshType _type {MeshType::Static};
    GLsizei _count {0};

  public:
    MOVE_SWAP(Mesh)

    Mesh(const Span<const byte> &vertexBuffer, const VertexBufferLayout &layout, GLenum primitiveType,
         MeshType meshType) noexcept
        : _layout(layout), _primitiveType(primitiveType), _type(meshType)
    {
      glCreateVertexArrays(1, &_vao);
      glBindVertexArray(_vao);

      glCreateBuffers(1, &_vbo);
      glNamedBufferData(_vbo, vertexBuffer.size_bytes(), vertexBuffer.data(),
                        _type == MeshType::Static ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW);
      glBindBuffer(GL_ARRAY_BUFFER, _vbo);

      auto vertexSize = Utils::ApplyVertexBufferLayout(_layout);
      _count = static_cast<GLsizei>(vertexBuffer.size_bytes() / vertexSize);
    }

    Mesh(const Span<const byte> &vertexBuffer, const Span<const byte> &indexBuffer,
         const VertexBufferLayout &layout, GLenum primitiveType, MeshType meshType) noexcept
        : _layout(layout), _primitiveType(primitiveType), _type(meshType)
    {
      glCreateVertexArrays(1, &_vao);
      glBindVertexArray(_vao);

      glCreateBuffers(1, &_vbo);
      glNamedBufferData(_vbo, vertexBuffer.size_bytes(), vertexBuffer.data(),
                        _type == MeshType::Static ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW);
      glBindBuffer(GL_ARRAY_BUFFER, _vbo);

      glCreateBuffers(1, &_ebo);
      glNamedBufferData(_ebo, indexBuffer.size_bytes(), indexBuffer.data(),
                        _type == MeshType::Static ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo);

      Utils::ApplyVertexBufferLayout(_layout);
      _count = static_cast<GLsizei>(indexBuffer.size_bytes() / sizeof(uint32));
    }

    ~Mesh() noexcept
    {
      if (_ebo != 0u)
      {
        glDeleteBuffers(1, &_ebo);
      }
      if (_vbo != 0u)
      {
        glDeleteBuffers(1, &_vbo);
      }
      if (_vao != 0u)
      {
        glDeleteVertexArrays(1, &_vao);
      }
    }

    void Bind() const noexcept
    {
      glBindVertexArray(_vao);
    }

    void Draw(GLsizei instanceCount = 1u) const noexcept
    {
      if (_ebo != 0u)
      {
        if (instanceCount > 1u)
        {
          glDrawElementsInstanced(_primitiveType, _count, GL_UNSIGNED_INT, nullptr, instanceCount);
        }
        else
        {
          glDrawElements(_primitiveType, _count, GL_UNSIGNED_INT, nullptr);
        }
      }
      else
      {
        if (instanceCount > 1u)
        {
          glDrawArraysInstanced(_primitiveType, 0, _count, instanceCount);
        }
        else
        {
          glDrawArrays(_primitiveType, 0, _count);
        }
      }
    }

    void ApplyInstanceDataLayout(Buffer &instanceDataBuffer, const VertexBufferLayout &instanceLayout,
                                 size_t attributeIndexOffset = 0u) noexcept
    {
      Bind();
      instanceDataBuffer.Bind();
      const size_t totalAttributeOffset = _layout.size() + attributeIndexOffset;
      Utils::ApplyVertexBufferLayout(instanceLayout, totalAttributeOffset);
    }

    KRYS_NODISCARD GLuint Id() const noexcept
    {
      return _vao;
    }

  private:
    void Swap(Mesh &other) noexcept
    {
      std::swap(_vao, other._vao);
      std::swap(_vbo, other._vbo);
      std::swap(_ebo, other._ebo);
      std::swap(_layout, other._layout);
      std::swap(_primitiveType, other._primitiveType);
      std::swap(_type, other._type);
      std::swap(_count, other._count);
    }
  };
}