#pragma once

#include "Krystal.Gfx.OpenGL/gl.hpp"
#include "Krystal.Gfx/Enums/VertexAttributeType.hpp"

namespace Krys::Gfx::OpenGL
{
  KRYS_NODISCARD constexpr inline GLenum MapVertexAttributeType(VertexAttributeType type) noexcept
  {
    switch (type)
    {
      case VertexAttributeType::Int32:        return GL_INT;
      case VertexAttributeType::UInt32:       return GL_UNSIGNED_INT;
      case VertexAttributeType::Float:        return GL_FLOAT;
      case VertexAttributeType::Double:       return GL_DOUBLE;
      case VertexAttributeType::UnsignedByte: return GL_UNSIGNED_BYTE;
      default:                                assert(false && "Unknown enum value: VertexAttributeType"); return GL_INT;
    }
  }

  KRYS_NODISCARD constexpr inline VertexAttributeType MapVertexAttributeType(GLenum type) noexcept
  {
    switch (type)
    {
      case GL_INT:           return VertexAttributeType::Int32;
      case GL_UNSIGNED_INT:  return VertexAttributeType::UInt32;
      case GL_FLOAT:         return VertexAttributeType::Float;
      case GL_DOUBLE:        return VertexAttributeType::Double;
      case GL_UNSIGNED_BYTE: return VertexAttributeType::UnsignedByte;
      default:               assert(false && "Unknown enum value: VertexAttributeType"); return VertexAttributeType::Int32;
    }
  }
}