#pragma once

#include "Krystal.IO/Path.hpp"
#include "Krystal.Gfx/Handle.hpp"

namespace Krys::Gfx
{
  class IFontSystem
  {
  public:
    virtual ~IFontSystem() = default;

    virtual FontHandle Load(const IO::Path &path, float size) noexcept = 0;
  };
}