#pragma once

#include "Krystal.Gfx.OpenGL/gl.hpp"
#include "Krystal.Gfx/Resources/Buffer.hpp"
#include "Krystal.Lib/Attributes.hpp"
#include "Krystal.Lib/ByteUtils.hpp"
#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Lib/Types.hpp"

namespace Krys::Gfx::OpenGL
{
  NO_DISCARD constexpr GLenum MapBufferType(BufferType type) noexcept
  {
    switch (type)
    {
      case BufferType::Index:   return GL_ELEMENT_ARRAY_BUFFER;
      case BufferType::Vertex:  return GL_ARRAY_BUFFER;
      case BufferType::Uniform: return GL_UNIFORM_BUFFER;
      case BufferType::Storage: return GL_SHADER_STORAGE_BUFFER;
      default:                  assert(false && "Unknown enum value: BufferType"); return GL_INVALID_ENUM;
    }
  }

  NO_DISCARD constexpr BufferType MapBufferType(GLenum type) noexcept
  {
    switch (type)
    {
      case GL_ELEMENT_ARRAY_BUFFER:  return BufferType::Index;
      case GL_ARRAY_BUFFER:          return BufferType::Vertex;
      case GL_UNIFORM_BUFFER:        return BufferType::Uniform;
      case GL_SHADER_STORAGE_BUFFER: return BufferType::Storage;
      default:                       assert(false && "Unknown enum value: BufferType"); return BufferType::Index;
    }
  }

  NO_DISCARD constexpr GLenum MapBufferUsage(BufferUsage usage) noexcept
  {
    switch (usage)
    {
      case BufferUsage::Static:  return GL_STATIC_DRAW;
      case BufferUsage::Dynamic: return GL_DYNAMIC_DRAW;
      case BufferUsage::Stream:  return GL_STREAM_DRAW;
      default:                   assert(false && "Unknown enum value: BufferUsage"); return GL_INVALID_ENUM;
    }
  }

  NO_DISCARD constexpr BufferUsage MapBufferUsage(GLenum usage) noexcept
  {
    switch (usage)
    {
      case GL_STATIC_DRAW:  return BufferUsage::Static;
      case GL_DYNAMIC_DRAW: return BufferUsage::Dynamic;
      case GL_STREAM_DRAW:  return BufferUsage::Stream;
      default:              assert(false && "Unknown enum value: BufferUsage"); return BufferUsage::Static;
    }
  }

  class Buffer
  {
    NO_COPY(Buffer)

  private:
    GLuint _handle;
    GLenum _type;
    GLenum _usage;
    uint32 _size;

  public:
    MOVE_SWAP(Buffer)

    Buffer(GLenum type, GLenum usage, uint32 size, Span<const byte> initialData)
        : _handle(0u), _type(type), _usage(usage), _size(size)
    {
      glCreateBuffers(1, &_handle);
      glNamedBufferData(_handle, _size, nullptr, _usage);

      if (!initialData.empty())
      {
        glNamedBufferSubData(_handle, 0, initialData.size(), initialData.data());
      }
    }

    ~Buffer() noexcept
    {
      glDeleteBuffers(1, &_handle);
    }

    void Bind() const noexcept
    {
      glBindBuffer(_type, _handle);
    }

    void Bind(uint32 index) const noexcept
    {
      assert((_type == GL_UNIFORM_BUFFER || _type == GL_SHADER_STORAGE_BUFFER) && "Invalid buffer type.");
      glBindBufferBase(_type, index, _handle);
    }

    void Unbind() const noexcept
    {
      glBindBuffer(_type, 0);
    }

    void Update(const Span<const byte> &data, size_t offset = 0u) const noexcept
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
    void Swap(Buffer &other)
    {
      std::swap(_handle, other._handle);
      std::swap(_type, other._type);
      std::swap(_usage, other._usage);
      std::swap(_size, other._size);
    }
  };
}