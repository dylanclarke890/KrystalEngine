#pragma once

#include "Krystal.Gfx.OpenGL/gl.hpp"
#include "Krystal.Gfx/Enums/BufferType.hpp"

namespace Krys::Gfx::OpenGL
{
  NO_DISCARD constexpr inline GLenum MapBufferType(BufferType type) noexcept
  {
    switch (type)
    {
      case BufferType::Index:   return GL_ELEMENT_ARRAY_BUFFER;
      case BufferType::Vertex:  return GL_ARRAY_BUFFER;
      case BufferType::Uniform: return GL_UNIFORM_BUFFER;
      case BufferType::Storage: return GL_SHADER_STORAGE_BUFFER;
      default:                  assert(false && "Unknown enum value: BufferType"); return GL_INVALID_ENUM;
    }
  }

  NO_DISCARD constexpr inline BufferType MapBufferType(GLenum type) noexcept
  {
    switch (type)
    {
      case GL_ELEMENT_ARRAY_BUFFER:  return BufferType::Index;
      case GL_ARRAY_BUFFER:          return BufferType::Vertex;
      case GL_UNIFORM_BUFFER:        return BufferType::Uniform;
      case GL_SHADER_STORAGE_BUFFER: return BufferType::Storage;
      default:                       assert(false && "Unknown enum value: BufferType"); return BufferType::Index;
    }
  }
}