#pragma once

#include "Krystal.Gfx/Resources/Font.hpp"
#include "Krystal.IO/Path.hpp"
#include "Krystal.Lib/Attributes.hpp"
#include "Krystal.Lib/Expected.hpp"
#include "Krystal.Lib/List.hpp"
#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Lib/Map.hpp"
#include "Krystal.Lib/Nullable.hpp"
#include "Krystal.Lib/Types.hpp"
#include "Krystal.Maths/Vector.hpp"

namespace Krys::Gfx
{
  struct FontAtlasResult
  {
    Maths::Vec2u Size;
    List<uint8> Pixels;
    Map<uchar, Character> Characters;
  };

  class FontAtlasLoader
  {
    NO_COPY_MOVE(FontAtlasLoader)

  public:
    FontAtlasLoader() noexcept = default;

    ~FontAtlasLoader() = default;

    NO_DISCARD Expected<FontAtlasResult> LoadBitmap(const IO::Path &path, uint32 fontSizeInPixels,
                                                    uint8 paddingPerGlyph = 2u) noexcept;
  };
}