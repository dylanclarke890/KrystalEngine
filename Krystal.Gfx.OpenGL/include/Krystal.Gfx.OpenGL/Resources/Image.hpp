#pragma once

#include "Krystal.Gfx.OpenGL/gl.hpp"
#include "Krystal.Gfx/Resources/Image.hpp"
#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Lib/Types.hpp"

namespace Krys::Gfx::OpenGL
{
  NO_DISCARD constexpr GLenum MapImageType(ImageType type) noexcept
  {
    switch (type)
    {
      case ImageType::Image1D:        return GL_TEXTURE_1D;
      case ImageType::Image2D:        return GL_TEXTURE_2D;
      case ImageType::Image3D:        return GL_TEXTURE_3D;
      case ImageType::ImageCube:      return GL_TEXTURE_CUBE_MAP;
      case ImageType::Image1DArray:   return GL_TEXTURE_1D_ARRAY;
      case ImageType::Image2DArray:   return GL_TEXTURE_2D_ARRAY;
      case ImageType::ImageCubeArray: return GL_TEXTURE_CUBE_MAP_ARRAY;
      default:                        assert(false && "Unknown enum value: ImageType"); return GL_INVALID_ENUM;
    }
  }

  NO_DISCARD constexpr ImageType MapImageType(GLenum target) noexcept
  {
    switch (target)
    {
      case GL_TEXTURE_1D:             return ImageType::Image1D;
      case GL_TEXTURE_2D:             return ImageType::Image2D;
      case GL_TEXTURE_3D:             return ImageType::Image3D;
      case GL_TEXTURE_CUBE_MAP:       return ImageType::ImageCube;
      case GL_TEXTURE_1D_ARRAY:       return ImageType::Image1DArray;
      case GL_TEXTURE_2D_ARRAY:       return ImageType::Image2DArray;
      case GL_TEXTURE_CUBE_MAP_ARRAY: return ImageType::ImageCubeArray;
      default:                        assert(false && "Unknown GLenum value for image type"); return ImageType::Image2D;
    }
  }

  NO_DISCARD constexpr GLenum MapImageFormat(ImageFormat format) noexcept
  {
    switch (format)
    {
      case ImageFormat::R8:              return GL_R8;
      case ImageFormat::R8G8:            return GL_RG8;
      case ImageFormat::R8G8B8:          return GL_RGB8;
      case ImageFormat::R8G8B8A8:        return GL_RGBA8;
      case ImageFormat::SRGB8:           return GL_SRGB8;
      case ImageFormat::SRGB8A8:         return GL_SRGB8_ALPHA8;
      case ImageFormat::R16F:            return GL_R16F;
      case ImageFormat::R16G16F:         return GL_RG16F;
      case ImageFormat::R16G16B16F:      return GL_RGB16F;
      case ImageFormat::R16G16B16A16F:   return GL_RGBA16F;
      case ImageFormat::R32F:            return GL_R32F;
      case ImageFormat::R32G32F:         return GL_RG32F;
      case ImageFormat::R32G32B32F:      return GL_RGB32F;
      case ImageFormat::R32G32B32A32F:   return GL_RGBA32F;
      case ImageFormat::DEPTH24STENCIL8: return GL_DEPTH24_STENCIL8;
      case ImageFormat::DEPTH32F:        return GL_DEPTH_COMPONENT32F;
      default:                           assert(false && "Unknown enum value: ImageFormat"); return GL_INVALID_ENUM;
    }
  }

  NO_DISCARD constexpr ImageFormat MapImageFormat(GLenum internalFormat) noexcept
  {
    switch (internalFormat)
    {
      case GL_R8:                 return ImageFormat::R8;
      case GL_RG8:                return ImageFormat::R8G8;
      case GL_RGB8:               return ImageFormat::R8G8B8;
      case GL_RGBA8:              return ImageFormat::R8G8B8A8;
      case GL_SRGB8:              return ImageFormat::SRGB8;
      case GL_SRGB8_ALPHA8:       return ImageFormat::SRGB8A8;
      case GL_R16F:               return ImageFormat::R16F;
      case GL_RG16F:              return ImageFormat::R16G16F;
      case GL_RGB16F:             return ImageFormat::R16G16B16F;
      case GL_RGBA16F:            return ImageFormat::R16G16B16A16F;
      case GL_R32F:               return ImageFormat::R32F;
      case GL_RG32F:              return ImageFormat::R32G32F;
      case GL_RGB32F:             return ImageFormat::R32G32B32F;
      case GL_RGBA32F:            return ImageFormat::R32G32B32A32F;
      case GL_DEPTH24_STENCIL8:   return ImageFormat::DEPTH24STENCIL8;
      case GL_DEPTH_COMPONENT32F: return ImageFormat::DEPTH32F;
      default:                    assert(false && "Unknown GLenum value for image format"); return ImageFormat::R8G8B8A8;
    }
  }

  class Image
  {
    NO_COPY(Image)

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

    NO_DISCARD GLuint Id() const noexcept
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