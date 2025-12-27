#pragma once

#include "Krystal.Gfx.OpenGL/gl.hpp"
#include "Krystal.Gfx/Enums/PixelFormat.hpp"
#include "Krystal.Lib/Core/Compiler.hpp"

namespace Krys::Gfx::OpenGL
{
  KRYS_NODISCARD constexpr inline GLenum MapPixelFormat(PixelFormat format) noexcept
  {
    switch (format)
    {
      case PixelFormat::R8:              return GL_R8;
      case PixelFormat::R8G8:            return GL_RG8;
      case PixelFormat::R8G8B8:          return GL_RGB8;
      case PixelFormat::R8G8B8A8:        return GL_RGBA8;
      case PixelFormat::SRGB8:           return GL_SRGB8;
      case PixelFormat::SRGB8A8:         return GL_SRGB8_ALPHA8;
      case PixelFormat::R16F:            return GL_R16F;
      case PixelFormat::R16G16F:         return GL_RG16F;
      case PixelFormat::R16G16B16F:      return GL_RGB16F;
      case PixelFormat::R16G16B16A16F:   return GL_RGBA16F;
      case PixelFormat::R32F:            return GL_R32F;
      case PixelFormat::R32G32F:         return GL_RG32F;
      case PixelFormat::R32G32B32F:      return GL_RGB32F;
      case PixelFormat::R32G32B32A32F:   return GL_RGBA32F;
      case PixelFormat::DEPTH24STENCIL8: return GL_DEPTH24_STENCIL8;
      case PixelFormat::DEPTH32F:        return GL_DEPTH_COMPONENT32F;
      default:                           assert(false && "Unknown enum value: PixelFormat"); return GL_INVALID_ENUM;
    }
  }

  KRYS_NODISCARD constexpr inline PixelFormat MapPixelFormat(GLenum internalFormat) noexcept
  {
    switch (internalFormat)
    {
      case GL_R8:                 return PixelFormat::R8;
      case GL_RG8:                return PixelFormat::R8G8;
      case GL_RGB8:               return PixelFormat::R8G8B8;
      case GL_RGBA8:              return PixelFormat::R8G8B8A8;
      case GL_SRGB8:              return PixelFormat::SRGB8;
      case GL_SRGB8_ALPHA8:       return PixelFormat::SRGB8A8;
      case GL_R16F:               return PixelFormat::R16F;
      case GL_RG16F:              return PixelFormat::R16G16F;
      case GL_RGB16F:             return PixelFormat::R16G16B16F;
      case GL_RGBA16F:            return PixelFormat::R16G16B16A16F;
      case GL_R32F:               return PixelFormat::R32F;
      case GL_RG32F:              return PixelFormat::R32G32F;
      case GL_RGB32F:             return PixelFormat::R32G32B32F;
      case GL_RGBA32F:            return PixelFormat::R32G32B32A32F;
      case GL_DEPTH24_STENCIL8:   return PixelFormat::DEPTH24STENCIL8;
      case GL_DEPTH_COMPONENT32F: return PixelFormat::DEPTH32F;
      default:                    assert(false && "Unknown enum value: PixelFormat"); return PixelFormat::R8G8B8A8;
    }
  }
}