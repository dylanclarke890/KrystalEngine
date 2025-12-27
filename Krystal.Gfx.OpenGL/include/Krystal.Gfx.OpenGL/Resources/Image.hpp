#pragma once

#include "Krystal.Gfx.OpenGL/gl.hpp"
#include "Krystal.Gfx/Resources/Image.hpp"
#include "Krystal.Lib/Core/Macros.hpp"
#include "Krystal.Lib/Mixins/NonCopyable.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"

namespace Krys::Gfx::OpenGL
{
  class Image : NonCopyable<Image>
  {
  private:
    GLuint _id {0u};
    GLenum _target {GL_TEXTURE_2D};
    GLenum _internalFormat {GL_RGBA8};
    uint32 _width {0u};
    uint32 _height {0u};
    uint32 _depth {0u};
    uint32 _mipLevels {1u};
    uint32 _arrayLayers {1u};

  public:
    MOVE_SWAP(Image)

    Image(GLenum target, GLenum internalFormat, uint32 width, uint32 height, uint32 depth, uint32 mipLevels,
          uint32 arrayLayers) noexcept
        : _target(target), _internalFormat(internalFormat), _width(width), _height(height), _depth(depth),
          _mipLevels(mipLevels), _arrayLayers(arrayLayers)
    {
      glCreateTextures(_target, 1, &_id);
      switch (_target)
      {
        case GL_TEXTURE_1D: glTextureStorage1D(_id, _mipLevels, _internalFormat, _width); break;
        case GL_TEXTURE_2D: glTextureStorage2D(_id, _mipLevels, _internalFormat, _width, _height); break;
        case GL_TEXTURE_3D:
          glTextureStorage3D(_id, _mipLevels, _internalFormat, _width, _height, _depth);
          break;
        case GL_TEXTURE_CUBE_MAP:
          glTextureStorage2D(_id, _mipLevels, _internalFormat, _width, _height);
          break;
        default: assert(false && "Unsupported image type for this constructor"); break;
      }
    }

    ~Image() noexcept
    {
      if (_id != 0u)
      {
        glDeleteTextures(1, &_id);
      }
    }

    KRYS_NODISCARD GLuint Id() const noexcept
    {
      return _id;
    }

    void UpdateData(const Span<const byte> &data, GLenum format, GLenum dataType) const noexcept
    {
      glTextureSubImage2D(_id, 0, 0, 0, _width, _height, format, dataType, data.data());
    }

    void UpdateData(GLenum face, const Span<const byte> &data, GLenum format, GLenum dataType) const noexcept
    {
      glTextureSubImage3D(_id, 0, 0, 0, face - GL_TEXTURE_CUBE_MAP_POSITIVE_X, _width, _height, 1, format,
                          dataType, data.data());
    }

    void GenerateMipmaps() const noexcept
    {
      if (_mipLevels > 1u)
      {
        glGenerateTextureMipmap(_id);
      }
    }

  private:
    void Swap(Image &other) noexcept
    {
      std::swap(_id, other._id);
      std::swap(_target, other._target);
      std::swap(_internalFormat, other._internalFormat);
      std::swap(_width, other._width);
      std::swap(_height, other._height);
      std::swap(_depth, other._depth);
      std::swap(_mipLevels, other._mipLevels);
      std::swap(_arrayLayers, other._arrayLayers);
    }
  };
}