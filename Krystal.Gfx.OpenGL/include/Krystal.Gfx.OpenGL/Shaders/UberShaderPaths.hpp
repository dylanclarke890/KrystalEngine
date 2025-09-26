#pragma once

#include "Krystal.IO/Path.hpp"
#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Lib/Pair.hpp"

namespace Krys::Gfx::OpenGL
{
  struct UberShaders
  {
    NO_COPY_MOVE(UberShaders)

    inline static const Pair<IO::Path, IO::Path> Text = std::make_pair(
      IO::Path("data/shaders/opengl/text-shader.vert"), IO::Path("data/shaders/opengl/text-shader.vert"));
  };
}