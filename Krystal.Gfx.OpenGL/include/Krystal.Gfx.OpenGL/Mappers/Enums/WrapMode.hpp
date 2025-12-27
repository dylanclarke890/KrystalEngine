#pragma once

#include "Krystal.Gfx.OpenGL/gl.hpp"
#include "Krystal.Gfx/Enums/WrapMode.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"

namespace Krys::Gfx::OpenGL
{
  NO_DISCARD constexpr inline GLenum MapWrapMode(WrapMode mode) noexcept
  {
    switch (mode)
    {
      case WrapMode::Repeat:        return GL_REPEAT;
      case WrapMode::ClampToEdge:   return GL_CLAMP_TO_EDGE;
      case WrapMode::ClampToBorder: return GL_CLAMP_TO_BORDER;
      default:                      assert(false && "Unknown enum value: WrapMode"); return GL_REPEAT;
    }
  }

  NO_DISCARD constexpr inline WrapMode MapWrapMode(GLenum mode) noexcept
  {
    switch (mode)
    {
      case GL_REPEAT:          return WrapMode::Repeat;
      case GL_CLAMP_TO_EDGE:   return WrapMode::ClampToEdge;
      case GL_CLAMP_TO_BORDER: return WrapMode::ClampToBorder;
      default:                 assert(false && "Unknown enum value: WrapMode"); return WrapMode::Repeat;
    }
  }
}