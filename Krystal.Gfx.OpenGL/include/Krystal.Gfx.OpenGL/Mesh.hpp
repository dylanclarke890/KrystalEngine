#pragma once

#include "Krystal.Gfx.OpenGL/gl.hpp"
#include "Krystal.Gfx.OpenGL/Utils.hpp"
#include "Krystal.Gfx/VertexBufferLayout.hpp"
#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Lib/Span.hpp"
#include "Krystal.Lib/Types.hpp"

namespace Krys::Gfx::OpenGL
{
  class Mesh
  {
    NO_COPY(Mesh)

    GLuint _vao {0u};
    GLuint _vbo {0u};
    GLuint _ebo {0u};

    GLenum _primitiveType {GL_TRIANGLES};
    GLsizei _count {0};

  public:
    Mesh(const Span<const byte> &vertexBuffer, const VertexBufferLayout &layout,
         GLenum primitiveType = GL_TRIANGLES) noexcept
        : _primitiveType(primitiveType)
    {
      glCreateVertexArrays(1, &_vao);
      glBindVertexArray(_vao);

      glCreateBuffers(1, &_vbo);
      glNamedBufferData(_vbo, vertexBuffer.size_bytes(), vertexBuffer.data(), GL_STATIC_DRAW);
      glBindBuffer(GL_ARRAY_BUFFER, _vbo);

      auto vertexSize = Utils::ApplyVertexBufferLayout(layout);
      _count = static_cast<GLsizei>(vertexBuffer.size_bytes() / vertexSize);
    }

    Mesh(const Span<const byte> &vertexBuffer, const Span<const byte> &indexBuffer,
         const VertexBufferLayout &layout, GLenum primitiveType = GL_TRIANGLES) noexcept
        : _primitiveType(primitiveType)
    {
      glCreateVertexArrays(1, &_vao);
      glBindVertexArray(_vao);

      glCreateBuffers(1, &_vbo);
      glNamedBufferData(_vbo, vertexBuffer.size_bytes(), vertexBuffer.data(), GL_STATIC_DRAW);
      glBindBuffer(GL_ARRAY_BUFFER, _vbo);

      glCreateBuffers(1, &_ebo);
      glNamedBufferData(_ebo, indexBuffer.size_bytes(), indexBuffer.data(), GL_STATIC_DRAW);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo);

      Utils::ApplyVertexBufferLayout(layout);
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

    Mesh(Mesh &&other) noexcept
    {
      Swap(other);
    }

    Mesh &operator=(Mesh &&other) noexcept
    {
      Swap(other);
      return *this;
    }

    void Bind() const noexcept
    {
      glBindVertexArray(_vao);
    }

    void Draw() const noexcept
    {
      if (_ebo != 0u)
      {
        glDrawElements(_primitiveType, _count, GL_UNSIGNED_INT, nullptr);
      }
      else
      {
        glDrawArrays(_primitiveType, 0, _count);
      }
    }

    void DrawInstanced(GLsizei instanceCount) const noexcept
    {
      if (_ebo != 0u)
      {
        glDrawElementsInstanced(_primitiveType, _count, GL_UNSIGNED_INT, nullptr, instanceCount);
      }
      else
      {
        glDrawArraysInstanced(_primitiveType, 0, _count, instanceCount);
      }
    }

  private:
    void Swap(Mesh &other) noexcept
    {
      std::swap(_vao, other._vao);
      std::swap(_vbo, other._vbo);
      std::swap(_ebo, other._ebo);
      std::swap(_primitiveType, other._primitiveType);
      std::swap(_count, other._count);
    }
  };
}