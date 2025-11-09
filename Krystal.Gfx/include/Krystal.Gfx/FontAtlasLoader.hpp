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
    List<uint8> AtlasPixels;
    Maths::Vec2u AtlasSize;
    Map<char, Character> Characters;
    uint32 PaddingPerGlyph {};
  };

  class FontAtlasLoader
  {
    NO_COPY_MOVE(FontAtlasLoader)

  public:
    FontAtlasLoader() noexcept = default;

    ~FontAtlasLoader() = default;

    NO_DISCARD Expected<FontAtlasResult> LoadBitmap(const IO::Path &path, uint32 fontSizeInPixels,
                                                    Nullable<uint8> padding = std::nullopt) noexcept;
  };
}