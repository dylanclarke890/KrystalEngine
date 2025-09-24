#pragma once

#include "Krystal.Gfx/FontType.hpp"
#include "Krystal.Gfx/Handle.hpp"
#include "Krystal.IO/Path.hpp"

namespace Krys::Gfx
{
  class IFontSystem
  {
  public:
    virtual ~IFontSystem() = default;

    virtual FontHandle Load(const IO::Path &path, float size, FontType fontType) noexcept = 0;
  };
}