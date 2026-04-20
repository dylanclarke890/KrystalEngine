#pragma once

#include "Krystal.Gfx.OpenGL/gl.hpp"
#include "Krystal.Gfx/Resources/Buffer.hpp"
#include "Krystal.Lib/ByteUtils.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Mixins/NonCopyable.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"

namespace Krys::Gfx::OpenGL
{
  class Buffer : NonCopyable<Buffer>
  {
  private:
    GLuint _id;
    GLenum _type;
    GLenum _usage;
    uint32 _size;

  public:
    Buffer(GLenum type, GLenum usage, uint32 size, Span<const byte> initialData)
        : _id(0u), _type(type), _usage(usage), _size(size)
    {
      glCreateBuffers(1, &_id);
      glNamedBufferData(_id, _size, nullptr, _usage);

      if (!initialData.empty())
      {
        glNamedBufferSubData(_id, 0, initialData.size(), initialData.data());
      }
    }

    ~Buffer() noexcept
    {
      glDeleteBuffers(1u, &_id);
    }

    Buffer(Buffer &&other) noexcept
        : _id(std::exchange(other._id, 0u)), _type(std::exchange(other._type, 0u)),
          _usage(std::exchange(other._usage, 0u)), _size(std::exchange(other._size, 0u))
    {
    }

    Buffer &operator=(Buffer &&other) noexcept
    {
      if (this != &other)
      {
        glDeleteBuffers(1u, &_id);

        _id = std::exchange(other._id, 0u);
        _type = std::exchange(other._type, 0u);
        _usage = std::exchange(other._usage, 0u);
        _size = std::exchange(other._size, 0u);
      }
      return *this;
    }

    void Bind() const noexcept
    {
      glBindBuffer(_type, _id);
    }

    void Bind(uint32 index) const noexcept
    {
      assert((_type == GL_UNIFORM_BUFFER || _type == GL_SHADER_STORAGE_BUFFER) && "Invalid buffer type.");
      glBindBufferBase(_type, index, _id);
    }

    void Unbind() const noexcept
    {
      glBindBuffer(_type, 0u);
    }

    void Update(const Span<const byte> &data, size_t offset = 0u) const noexcept
    {
      glNamedBufferSubData(_id, offset, data.size(), data.data());
    }

    template <typename T>
    void Update(const T &data, size_t offset = 0u) const noexcept
    {
      Update(ByteUtils::AsBytesView(data), offset);
    }

    KRYS_NODISCARD GLuint Id() const noexcept
    {
      return _id;
    }
  };
}