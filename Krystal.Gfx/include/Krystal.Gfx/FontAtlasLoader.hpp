#pragma once

#include "Krystal.Gfx/Resources/Font.hpp"
#include "Krystal.IO/Path.hpp"
#include "Krystal.Lib/Expected.hpp"
#include "Krystal.Lib/List.hpp"
#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Lib/Map.hpp"
#include "Krystal.Lib/Nullable.hpp"
#include "Krystal.Lib/Types.hpp"
#include "Krystal.Maths/Vector.hpp"
#include <stb_rect_pack.h>

namespace Krys::Gfx
{
  struct FontAtlasResult
  {
    List<uint8> AtlasPixels;
    Maths::Vec2u AtlasSize;
    Map<char, Character> Characters;
    uint32 PaddingPerGlyph {};
  };

  struct GlyphToPack
  {
    char Char {};
    Maths::Vec2u BitmapSize {}; // glyph bitmap size (tight)
    Maths::Vec2u PackedSize {}; // packed rect size incl. padding
    Maths::Vec2i Bearing {};    // slot->bitmap_left/top
    uint32 Advance {};          // pixels (>>6)
    List<uint8> Pixels {};      // tight W*H grayscale
    stbrp_rect Rect {};
  };

  class FontAtlasLoader
  {
    NO_COPY_MOVE(FontAtlasLoader)

  public:
    FontAtlasLoader() noexcept = default;

    ~FontAtlasLoader() = default;

    Expected<FontAtlasResult> LoadBitmap(const IO::Path &path, double fontSizeInPixels,
                                         Nullable<uint8> padding = std::nullopt);

  private:
    Map<char, Character> CreateBitmapCharacters(List<GlyphToPack> &glyphs, int padding,
                                                const Maths::Vec2u &atlasSize);
    List<uint8> CreateBitmapAtlasPixels(List<GlyphToPack> &glyphs, const Maths::Vec2u &atlasSize);

    bool TryPack(List<GlyphToPack> &glyphs, Maths::Vec2u &size);
  };
}