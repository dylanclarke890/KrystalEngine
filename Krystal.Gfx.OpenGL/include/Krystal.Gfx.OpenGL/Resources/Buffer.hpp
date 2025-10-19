#pragma once

#include "Krystal.Gfx.OpenGL/gl.hpp"
#include "Krystal.Lib/Attributes.hpp"
#include "Krystal.Lib/ByteUtils.hpp"
#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Lib/Types.hpp"
#include "Krystal.Maths/Matrix.hpp"

namespace Krys::Gfx::OpenGL
{
  using BufferData = Span<const byte>;

  class Buffer
  {
    NO_COPY(Buffer)

  private:
    GLuint _handle;
    GLenum _bufferType;

  public:
    Buffer(GLenum bufferType, const BufferData &data, GLenum bufferUsage = GL_STATIC_DRAW) noexcept
        : _handle(0u), _bufferType(bufferType)
    {
      glCreateBuffers(1, &_handle);
      glNamedBufferData(_handle, data.size(), data.data(), bufferUsage);
    }

    template <typename T>
    Buffer(GLenum bufferType, const List<T> &data, GLenum bufferUsage = GL_STATIC_DRAW) noexcept
        : Buffer(bufferType, ByteUtils::AsBytesView(data), bufferUsage)
    {
    }

    Buffer(GLenum bufferType, size_t size, GLenum bufferUsage = GL_DYNAMIC_DRAW) noexcept
        : _handle(0u), _bufferType(bufferType)
    {
      glCreateBuffers(1, &_handle);
      glNamedBufferData(_handle, size, nullptr, bufferUsage);
    }

    ~Buffer() noexcept
    {
      glDeleteBuffers(1, &_handle);
    }

    Buffer(Buffer &&other) noexcept
    {
      Swap(std::move(other));
    }

    Buffer &operator=(Buffer &&other) noexcept
    {
      if (this != &other)
      {
        Swap(std::move(other));
      }
      return *this;
    }

    void Bind() const noexcept
    {
      glBindBuffer(_bufferType, _handle);
    }

    void Bind(uint32 index) const noexcept
    {
      assert((_bufferType == GL_UNIFORM_BUFFER || _bufferType == GL_SHADER_STORAGE_BUFFER)
             && "Invalid buffer type.");
      glBindBufferBase(_bufferType, index, _handle);
    }

    void Unbind() const noexcept
    {
      glBindBuffer(_bufferType, 0);
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

    NO_DISCARD GLuint GetHandle() const noexcept
    {
      return _handle;
    }

  private:
    void Swap(Buffer &&other)
    {
      std::swap(_handle, other._handle);
      std::swap(_bufferType, other._bufferType);
    }
  };
}