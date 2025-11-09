#include "Krystal.Gfx/FontAtlasLoader.hpp"
#include "Krystal.Lib/DebugBreak.hpp"
#include "Krystal.Log/ILogger.hpp"
#include <ft2build.h>
#include <stb_rect_pack.h>
#include FT_FREETYPE_H

namespace
{
  using namespace Krys;
  using namespace Krys::Gfx;

  struct GlyphToPack
  {
    char Char {};
    Maths::Vec2u BitmapSize {}; // glyph bitmap size (tight)
    Maths::Vec2u PackedSize {}; // packed rect size incl. padding
    Maths::Vec2i Bearing {};    // slot->bitmap_left/top
    int32 Advance {};
    List<uint8> Pixels {}; // tight W*H grayscale
    stbrp_rect Rect {};
  };

  Map<char, Character> CreateBitmapCharacters(List<GlyphToPack> &glyphs, int padding,
                                              const Maths::Vec2u &atlasSize)
  {
    Map<char, Character> characters;
    characters.reserve(glyphs.size());

    for (const auto &glyph : glyphs)
    {
      const float u0 = float(glyph.Rect.x + padding) / float(atlasSize.x);
      const float v0 = float(glyph.Rect.y + padding) / float(atlasSize.y);
      const float u1 = float(glyph.Rect.x + padding + glyph.BitmapSize.x) / float(atlasSize.x);
      const float v1 = float(glyph.Rect.y + padding + glyph.BitmapSize.y) / float(atlasSize.y);

      Character ch = {
        .Size = glyph.BitmapSize,
        .Bearing = glyph.Bearing,
        .Advance = glyph.Advance,
        .UVMin = {u0, v0},
        .UVMax = {u1, v1},
      };
      characters[glyph.Char] = ch;
    }

    return characters;
  }

  List<uint8> CreateBitmapAtlasPixels(const List<GlyphToPack> &glyphs, const Maths::Vec2u &atlasSize)
  {
    List<uint8> pixels(size_t(atlasSize.x) * atlasSize.y);
    std::fill(pixels.begin(), pixels.end(), 0);

    for (const auto &glyph : glyphs)
    {
      for (uint row = 0; row < glyph.BitmapSize.y; row++)
      {
        uint8 *dst = pixels.data() + (glyph.Rect.y + row + 2) * atlasSize.x + (glyph.Rect.x + 2);
        const uint8 *src = glyph.Pixels.data() + row * glyph.BitmapSize.x;
        std::memcpy(dst, src, glyph.BitmapSize.x);
      }
    }
    return pixels;
  }

  bool TryPack(List<GlyphToPack> &glyphs, Maths::Vec2u &size)
  {
    bool success = false;
    while (!success)
    {
      List<stbrp_node> nodes(size.x);

      stbrp_context ctx {};
      stbrp_init_target(&ctx, size.x, size.y, nodes.data(), (int)nodes.size());

      List<stbrp_rect> rects;
      rects.reserve(glyphs.size());

      for (int i = 0; i < glyphs.size(); i++)
      {
        glyphs[i].Rect = {};
        glyphs[i].Rect.id = i;
        glyphs[i].Rect.w = glyphs[i].PackedSize.x;
        glyphs[i].Rect.h = glyphs[i].PackedSize.y;
        rects.push_back(glyphs[i].Rect);
      }

      stbrp_pack_rects(&ctx, rects.data(), (int)rects.size());

      bool allPacked = std::all_of(rects.begin(), rects.end(), [](const auto &r) { return r.was_packed; });
      if (allPacked)
      {
        // write back positions
        for (auto &r : rects)
        {
          glyphs[r.id].Rect = r;
        }

        success = true;
      }

      if (!success)
      {
        if (size.x <= size.y)
        {
          size.x *= 2;
        }
        else
        {
          size.y *= 2;
        }

        if (size.x > 4'096 || size.y > 4'096)
        {
          KRYS_ERROR("Font atlas exceeded 4096 size");
          KRYS_DEBUG_BREAK();
          return false;
        }
      }
    }

    return true;
  }

}

namespace Krys::Gfx
{
  Expected<FontAtlasResult> FontAtlasLoader::LoadBitmap(const IO::Path &path, uint32 fontSizeInPixels,
                                                        Nullable<uint8> padding) noexcept
  {
    FontAtlasResult atlasResult {};
    atlasResult.PaddingPerGlyph = padding.has_value() ? padding.value() : 2u;

    List<GlyphToPack> glyphs;
    glyphs.reserve(95);

    FT_Library fontLibraryHandle = nullptr;
    if (FT_Init_FreeType(&fontLibraryHandle))
    {
      KRYS_ERROR("FREETYPE: Could not init FreeType Library");
      KRYS_DEBUG_BREAK();
      return Unexpected("Could not init FreeType Library");
    }

    KRYS_INFO("FREETYPE: Loading bitmap font from '{}'", path.ToString());
    FT_Face face {};
    if (FT_New_Face(fontLibraryHandle, path.ToString().c_str(), 0, &face))
    {
      FT_Done_FreeType(fontLibraryHandle);
      KRYS_ERROR("FREETYPE: Failed to load font '{}'", path.ToString());
      return Unexpected("Failed to load font");
    }

    KRYS_INFO("FREETYPE: Loaded font.", path.ToString());
    KRYS_INFO("FREETYPE:   Family: {}", face->family_name);
    KRYS_INFO("FREETYPE:   Style: {}", face->style_name);
    KRYS_INFO("FREETYPE:   {} face(s) in font", face->num_faces);
    KRYS_INFO("FREETYPE: Setting font size to {}px", fontSizeInPixels);
    if (FT_Set_Pixel_Sizes(face, 0, fontSizeInPixels))
    {
      FT_Done_Face(face);
      FT_Done_FreeType(fontLibraryHandle);
      KRYS_ERROR("FREETYPE: Failed to set font size {}px", fontSizeInPixels);
      KRYS_DEBUG_BREAK();
      return Unexpected("Failed to set font size");
    }

    for (uchar c = 32; c < 127; c++)
    {
      if (FT_Load_Char(face, c, FT_LOAD_RENDER))
      {
        KRYS_WARN("FREETYPE: Failed to load glyph {}", (int)c);
        continue;
      }

      if (face->glyph->bitmap.pixel_mode != FT_PIXEL_MODE_GRAY)
      {
        KRYS_WARN("FREETYPE: Glyph {} not grayscale; skipping", (int)c);
        continue;
      }

      const FT_GlyphSlot slot = face->glyph;
      const FT_Bitmap &bm = slot->bitmap;

      GlyphToPack glyph {};
      glyph.Char = c;
      glyph.BitmapSize = {bm.width, bm.rows};
      glyph.Bearing = {slot->bitmap_left, slot->bitmap_top};
      glyph.Advance = static_cast<int32>(slot->advance.x >> 6);

      // Copy to tight buffer (handle negative pitch)
      glyph.Pixels.resize(size_t(glyph.BitmapSize.x) * glyph.BitmapSize.y);
      for (uint row = 0; row < glyph.BitmapSize.y; ++row)
      {
        const uint8 *src = (bm.pitch >= 0) ? (bm.buffer + row * bm.pitch)
                                           : (bm.buffer + (glyph.BitmapSize.y - 1u - row) * (-bm.pitch));
        uint8 *dst = glyph.Pixels.data() + row * glyph.BitmapSize.x;
        std::memcpy(dst, src, glyph.BitmapSize.x);
      }

      glyph.PackedSize = {glyph.BitmapSize.x + atlasResult.PaddingPerGlyph * 2u,
                          glyph.BitmapSize.y + atlasResult.PaddingPerGlyph * 2u};
      glyphs.push_back(std::move(glyph));
    }

    FT_Done_Face(face);
    FT_Done_FreeType(fontLibraryHandle);

    atlasResult.AtlasSize = {512u, 512u};
    if (!TryPack(glyphs, atlasResult.AtlasSize))
    {
      KRYS_ERROR("FREETYPE: Failed to pack glyphs for font '{}'", path.ToString());
      KRYS_DEBUG_BREAK();
      return {};
    }

    atlasResult.AtlasPixels = CreateBitmapAtlasPixels(glyphs, atlasResult.AtlasSize);
    atlasResult.Characters =
      CreateBitmapCharacters(glyphs, atlasResult.PaddingPerGlyph, atlasResult.AtlasSize);

    return atlasResult;
  }
}