#pragma once

#include "Krystal.Gfx.OpenGL/Buffer.hpp"
#include "Krystal.Gfx.OpenGL/Hooks/gl.hpp"
#include "Krystal.Gfx.OpenGL/Utils.hpp"
#include "Krystal.Gfx/VertexBufferLayout.hpp"

namespace Krys::Gfx::OpenGL
{
  class VertexArray
  {
    GLuint _handle;
    size_t _attributeCount;
    List<VertexBuffer *> _vertexBuffers;
    IndexBuffer *_indexBuffer;

  public:
    VertexArray() noexcept : _handle(0u), _attributeCount(0), _indexBuffer(nullptr)
    {
      glCreateVertexArrays(1, &_handle);
    }

    ~VertexArray() noexcept
    {
      glDeleteVertexArrays(1, &_handle);
    }

    void Bind() const noexcept
    {
      glBindVertexArray(_handle);
    }

    void Unbind() const noexcept
    {
      glBindVertexArray(0);
    }

    void AddVertexBuffer(VertexBuffer *buffer, const VertexBufferLayout &layout) noexcept
    {
      _vertexBuffers.push_back(buffer);

      Bind();
      buffer->Bind();

      Utils::ApplyVertexBufferLayout(layout);
      _attributeCount += static_cast<uint32>(layout.size());
    }

    void SetIndexBuffer(IndexBuffer *buffer) noexcept
    {
      _indexBuffer = buffer;

      Bind();
      buffer->Bind();
    }
  };
}