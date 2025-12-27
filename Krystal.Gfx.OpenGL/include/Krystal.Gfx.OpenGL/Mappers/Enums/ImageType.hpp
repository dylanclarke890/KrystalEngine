#pragma once

#include "Krystal.Gfx.OpenGL/gl.hpp"
#include "Krystal.Gfx/Enums/ImageType.hpp"
#include "Krystal.Lib/Core/Compiler.hpp"

namespace Krys::Gfx::OpenGL
{
  KRYS_NODISCARD constexpr inline GLenum MapImageType(ImageType type) noexcept
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

  KRYS_NODISCARD constexpr inline ImageType MapImageType(GLenum target) noexcept
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
      default:                        assert(false && "Unknown enum value: ImageType"); return ImageType::Image2D;
    }
  }

}