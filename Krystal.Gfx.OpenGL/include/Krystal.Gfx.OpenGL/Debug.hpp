#pragma once

#include "Krystal.Gfx.OpenGL/gl.hpp"
#include "Krystal.Gfx.OpenGL/Resources/Buffer.hpp"
#include "Krystal.Gfx.OpenGL/Resources/Shader.hpp"
#include "Krystal.Gfx.OpenGL/Resources/Mesh.hpp"
#include "Krystal.Lib/Mixins/NonCopyMovable.hpp"
#include "Krystal.Lib/String/String.hpp"

namespace Krys::Gfx::OpenGL
{
  struct Debug : NonCopyMovable<Debug>
  {
    Debug() = delete;
    ~Debug() = delete;

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