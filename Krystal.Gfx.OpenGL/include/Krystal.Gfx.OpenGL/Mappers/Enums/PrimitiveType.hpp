#pragma once

#include "Krystal.Gfx.OpenGL/gl.hpp"
#include "Krystal.Gfx/Enums/PrimitiveType.hpp"
#include "Krystal.Lib/Attributes.hpp"

namespace Krys::Gfx::OpenGL
{
  NO_DISCARD constexpr inline GLenum MapPrimitiveType(PrimitiveType type) noexcept
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

  NO_DISCARD constexpr inline PrimitiveType MapPrimitiveType(GLenum type) noexcept
  {
    switch (type)
    {
      case GL_POINTS:         return PrimitiveType::Points;
      case GL_LINES:          return PrimitiveType::Lines;
      case GL_LINE_STRIP:     return PrimitiveType::LineStrip;
      case GL_TRIANGLES:      return PrimitiveType::Triangles;
      case GL_TRIANGLE_STRIP: return PrimitiveType::TriangleStrip;
      case GL_TRIANGLE_FAN:   return PrimitiveType::TriangleFan;
      default:                assert(false && "Unknown enum value: PrimitiveType"); return PrimitiveType::Triangles;
    }
  }
}