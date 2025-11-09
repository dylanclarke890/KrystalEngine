#pragma once

#include "Krystal.Gfx/Resources/Font.hpp"
#include "Krystal.IO/Path.hpp"
#include "Krystal.Lib/Attributes.hpp"
#include "Krystal.Lib/Expected.hpp"
#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Lib/Types.hpp"

namespace Krys::Gfx
{
  class FontAtlasLoader
  {
    NO_COPY_MOVE(FontAtlasLoader)

  public:
    FontAtlasLoader() noexcept = default;

    ~FontAtlasLoader() = default;

    NO_DISCARD Expected<FontAtlasData> LoadBitmap(const IO::Path &path, uint32 fontSizeInPixels,
                                                  uint8 paddingPerGlyph = 2u) noexcept;
  };
}