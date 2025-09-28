#pragma once

#ifndef KRYS_GL_OVERRIDE_FUNCTIONS
  #define KRYS_GL_OVERRIDE_FUNCTIONS 0
#endif
#include "Krystal.Gfx.OpenGL/Hooks/gl.hpp"
#include "Krystal.Gfx/Common.hpp"
#include <cassert>

namespace Krys::Gfx::OpenGL
{
  NO_DISCARD constexpr GLenum MapPrimitiveType(PrimitiveType type) noexcept
  {
    switch (type)
    {
      case PrimitiveType::Points:        return GL_POINTS;
      case PrimitiveType::Lines:         return GL_LINES;
      case PrimitiveType::LineStrip:     return GL_LINE_STRIP;
      case PrimitiveType::Triangles:     return GL_TRIANGLES;
      case PrimitiveType::TriangleStrip: return GL_TRIANGLE_STRIP;
      case PrimitiveType::TriangleFan:   return GL_TRIANGLE_FAN;
      default:                           assert(false && "Unknown enum value: PrimitiveType"); return GL_INVALID_ENUM;
    }
  }

  NO_DISCARD constexpr GLenum MapFilterMode(FilterMode mode) noexcept
  {
    switch (mode)
    {
      case FilterMode::Nearest:              return GL_NEAREST;
      case FilterMode::Linear:               return GL_LINEAR;
      case FilterMode::NearestMipmapNearest: return GL_NEAREST_MIPMAP_NEAREST;
      case FilterMode::LinearMipmapNearest:  return GL_LINEAR_MIPMAP_NEAREST;
      case FilterMode::NearestMipmapLinear:  return GL_NEAREST_MIPMAP_LINEAR;
      case FilterMode::LinearMipmapLinear:   return GL_LINEAR_MIPMAP_LINEAR;
      default:                               assert(false && "Unknown enum value: FilterMode"); return GL_INVALID_ENUM;
    }
  }

  NO_DISCARD constexpr GLenum MapWrapMode(WrapMode mode) noexcept
  {
    switch (mode)
    {
      case WrapMode::Repeat:        return GL_REPEAT;
      case WrapMode::ClampToEdge:   return GL_CLAMP_TO_EDGE;
      case WrapMode::ClampToBorder: return GL_CLAMP_TO_BORDER;
      default:                      assert(false && "Unknown enum value: WrapMode"); return GL_INVALID_ENUM;
    }
  }

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
}