#pragma once

#include "Krystal.Gfx.OpenGL/gl.hpp"
#include "Krystal.Gfx/Enums/BufferBitFlags.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"

namespace Krys::Gfx::OpenGL
{
  NO_DISCARD constexpr inline GLenum MapBufferBitFlags(BufferBitFlags flags) noexcept
  {
    GLenum glFlags = 0;
    if (!!(flags & BufferBitFlags::Colour))
    {
      glFlags |= GL_COLOR_BUFFER_BIT;
    }
    if (!!(flags & BufferBitFlags::Depth))
    {
      glFlags |= GL_DEPTH_BUFFER_BIT;
    }
    if (!!(flags & BufferBitFlags::Stencil))
    {
      glFlags |= GL_STENCIL_BUFFER_BIT;
    }
    return glFlags;
  }

  NO_DISCARD constexpr inline BufferBitFlags MapBufferBitFlags(GLenum glFlags) noexcept
  {
    BufferBitFlags flags = BufferBitFlags::None;
    if (glFlags & GL_COLOR_BUFFER_BIT)
    {
      flags |= BufferBitFlags::Colour;
    }
    if (glFlags & GL_DEPTH_BUFFER_BIT)
    {
      flags |= BufferBitFlags::Depth;
    }
    if (glFlags & GL_STENCIL_BUFFER_BIT)
    {
      flags |= BufferBitFlags::Stencil;
    }
    return flags;
  }
}