#pragma once

#include "Krystal.Gfx.OpenGL/gl.hpp"
#include "Krystal.Gfx/Enums/BufferUsage.hpp"

namespace Krys::Gfx::OpenGL
{
  NO_DISCARD constexpr inline GLenum MapBufferUsage(BufferUsage usage) noexcept
  {
    switch (usage)
    {
      case BufferUsage::Static:  return GL_STATIC_DRAW;
      case BufferUsage::Dynamic: return GL_DYNAMIC_DRAW;
      case BufferUsage::Stream:  return GL_STREAM_DRAW;
      default:                   assert(false && "Unknown enum value: BufferUsage"); return GL_INVALID_ENUM;
    }
  }

  NO_DISCARD constexpr inline BufferUsage MapBufferUsage(GLenum usage) noexcept
  {
    switch (usage)
    {
      case GL_STATIC_DRAW:  return BufferUsage::Static;
      case GL_DYNAMIC_DRAW: return BufferUsage::Dynamic;
      case GL_STREAM_DRAW:  return BufferUsage::Stream;
      default:              assert(false && "Unknown enum value: BufferUsage"); return BufferUsage::Static;
    }
  }
}