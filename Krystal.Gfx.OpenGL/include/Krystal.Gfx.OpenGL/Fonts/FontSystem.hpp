#pragma once

#include "Krystal.Gfx/Handle.hpp"
#include "Krystal.Gfx/IFontSystem.hpp"
#include "Krystal.IO/Path.hpp"
#include "Krystal.Lib/Macros.hpp"

namespace Krys::Gfx::OpenGL
{
  class FontSystem final : public IFontSystem
  {
    NO_COPY_MOVE(FontSystem)

  public:
    FontSystem() = default;

    ~FontSystem() noexcept override = default;
    
    NO_DISCARD FontHandle Load(const IO::Path &path, float size) noexcept override;
  };
}