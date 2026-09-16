#pragma once

#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/String.hpp"
#include "Krystal.Gfx.OpenGL/gl.hpp"
#include "Krystal.Gfx.OpenGL/Resources/Buffer.hpp"
#include "Krystal.Gfx.OpenGL/Resources/Mesh.hpp"
#include "Krystal.Gfx.OpenGL/Resources/Shader.hpp"

namespace krys::Gfx::OpenGL
{
  class Debug
  {
    Debug() = delete;
    ~Debug() = delete;

    KRYS_NON_COPY_MOVABLE(Debug);

  public:
    static void SetName(Shader &shader, stringview debugName) noexcept
    {
      SetName(GL_PROGRAM, shader.Id(), debugName);
    }

    static void SetName(Buffer &buffer, stringview debugName) noexcept
    {
      SetName(GL_BUFFER, buffer.Id(), debugName);
    }

    static void SetName(Mesh &mesh, stringview debugName) noexcept
    {
      SetName(GL_VERTEX_ARRAY, mesh.Id(), debugName);
    }

    static void SetName(GLenum identifier, GLuint name, stringview debugName) noexcept
    {
      glObjectLabel(identifier, name, static_cast<GLsizei>(debugName.size()), debugName.data());
    }
  };
}