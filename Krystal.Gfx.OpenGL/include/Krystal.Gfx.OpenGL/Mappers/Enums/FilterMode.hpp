#pragma once

#include "Krystal.Gfx.OpenGL/gl.hpp"
#include "Krystal.Gfx/Enums/FilterMode.hpp"
#include "Krystal.Lib/Attributes.hpp"

namespace Krys::Gfx::OpenGL
{
  NO_DISCARD constexpr inline GLenum MapFilterMode(FilterMode mode) noexcept
  {
    switch (mode)
    {
      case FilterMode::Nearest:              return GL_NEAREST;
      case FilterMode::Linear:               return GL_LINEAR;
      case FilterMode::NearestMipmapNearest: return GL_NEAREST_MIPMAP_NEAREST;
      case FilterMode::LinearMipmapNearest:  return GL_LINEAR_MIPMAP_NEAREST;
      case FilterMode::NearestMipmapLinear:  return GL_NEAREST_MIPMAP_LINEAR;
      case FilterMode::LinearMipmapLinear:   return GL_LINEAR_MIPMAP_LINEAR;
      default:                               assert(false && "Unknown enum value: FilterMode"); return GL_LINEAR;
    }
  }

  NO_DISCARD constexpr inline FilterMode MapFilterMode(GLenum mode) noexcept
  {
    switch (mode)
    {
      case GL_NEAREST:                FilterMode::Nearest;
      case GL_LINEAR:                 FilterMode::Linear;
      case GL_NEAREST_MIPMAP_NEAREST: FilterMode::NearestMipmapNearest;
      case GL_LINEAR_MIPMAP_NEAREST:  FilterMode::LinearMipmapNearest;
      case GL_NEAREST_MIPMAP_LINEAR:  FilterMode::NearestMipmapLinear;
      case GL_LINEAR_MIPMAP_LINEAR:   FilterMode::LinearMipmapLinear;
      default:                        assert(false && "Unknown enum value: FilterMode"); return FilterMode::Linear;
    }
  }
}