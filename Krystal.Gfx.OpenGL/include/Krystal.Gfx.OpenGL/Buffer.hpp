#pragma once

#include "Krystal.Core/ByteUtils.hpp"
#include "Krystal.Core/Core.hpp"
#include "Krystal.Gfx.OpenGL/Hooks/gl.hpp"
#include "Krystal.Maths/Matrix.hpp"

namespace Krys::Gfx::OpenGL
{
  using BufferData = Span<const byte>;

  template <GLenum BufferType>
  class Buffer
  {
    GLuint _handle;

  public:
    Buffer(const BufferData &data, GLenum bufferUsage = GL_STATIC_DRAW) noexcept : _handle(0u)
    {
      glCreateBuffers(1, &_handle);
      glNamedBufferData(_handle, data.size(), data.data(), bufferUsage);
    }

    template <typename T>
    Buffer(const List<T> &data, GLenum bufferUsage = GL_STATIC_DRAW) noexcept
        : Buffer(ByteUtils::AsBytesView(data), bufferUsage)
    {
    }

    Buffer(size_t size, GLenum bufferUsage = GL_DYNAMIC_DRAW) noexcept : _handle(0u)
    {
      glCreateBuffers(1, &_handle);
      glNamedBufferData(_handle, size, nullptr, bufferUsage);
    }

    ~Buffer() noexcept
    {
      glDeleteBuffers(1, &_handle);
    }

    void Bind() const noexcept
    {
      glBindBuffer(BufferType, _handle);
    }

    void Bind(uint32 index) const noexcept
    requires(BufferType == GL_UNIFORM_BUFFER || BufferType == GL_SHADER_STORAGE_BUFFER)
    {
      glBindBufferBase(BufferType, index, _handle);
    }

    void Unbind() const noexcept
    {
      glBindBuffer(BufferType, 0);
    }

    void Update(const BufferData &data, size_t offset = 0u) const noexcept
    {
      glNamedBufferSubData(_handle, offset, data.size(), data.data());
    }

    template <typename T>
    void Update(const T &data, size_t offset = 0u) const noexcept
    {
      Update(ByteUtils::AsBytesView(data), offset);
    }

    GLuint GetHandle() const noexcept
    {
      return _handle;
    }
  };

  using VertexBuffer = Buffer<GL_ARRAY_BUFFER>;
  using IndexBuffer = Buffer<GL_ELEMENT_ARRAY_BUFFER>;
  using UniformBuffer = Buffer<GL_UNIFORM_BUFFER>;
  using ShaderStorageBuffer = Buffer<GL_SHADER_STORAGE_BUFFER>;
}