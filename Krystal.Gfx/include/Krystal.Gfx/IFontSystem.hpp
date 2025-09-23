#pragma once

#include "Krystal.IO/Path.hpp"
#include "Krystal.Gfx/Handle.hpp"

namespace Krys::Gfx
{
  enum class FontType : uint8
  {
    Bitmap,
    SDF,
    MSDF
  };

  class IFontSystem
  {
  public:
    virtual ~IFontSystem() = default;

    virtual FontHandle Load(const IO::Path &path, float size, FontType fontType) noexcept = 0;
  };
}