#pragma once

#include "Krystal.Gfx.OpenGL/gl.hpp"
#include "Krystal.Lib/Attributes.hpp"
#include "Krystal.Gfx/Enums/WrapMode.hpp"

namespace Krys::Gfx::OpenGL
{
  NO_DISCARD constexpr static GLenum MapWrapMode(WrapMode mode) noexcept
  {
    switch (mode)
    {
      case WrapMode::Repeat:        return GL_REPEAT;
      case WrapMode::ClampToEdge:   return GL_CLAMP_TO_EDGE;
      case WrapMode::ClampToBorder: return GL_CLAMP_TO_BORDER;
      default:                      return GL_REPEAT;
    }
  }

  NO_DISCARD constexpr static WrapMode MapWrapMode(GLenum mode) noexcept
  {
    switch (mode)
    {
      case GL_REPEAT:          return WrapMode::Repeat;
      case GL_CLAMP_TO_EDGE:   return WrapMode::ClampToEdge;
      case GL_CLAMP_TO_BORDER: return WrapMode::ClampToBorder;
      default:                 return WrapMode::Repeat;
    }
  }
}