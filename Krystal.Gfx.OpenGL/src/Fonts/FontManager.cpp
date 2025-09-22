#pragma once

#include "Krystal.Gfx.OpenGL/Fonts/Font.hpp"
#include "Krystal.Gfx.OpenGL/Fonts/FontSystem.hpp"
#include "Krystal.Gfx.OpenGL/gl.hpp"
#include "Krystal.Gfx/Handle.hpp"
#include <ft2build.h>
#include FT_FREETYPE_H

namespace
{
  FT_Library FontLibraryHandle = nullptr;
}

namespace Krys::Gfx::OpenGL
{
  FontHandle FontSystem::Load(const IO::Path &path, float size) noexcept
  {
    return {};
  }
}