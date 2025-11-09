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

  struct BitmapGlyph
  {
    uchar Char {};
    Maths::Vec2u ActualSize {}; // actual glyph bitmap size (tight)
    Maths::Vec2u PaddedSize {}; // size including padding
    Maths::Vec2i Bearing {};    // slot->bitmap_left/top
    int32 Advance {};
    List<uint8> Pixels {}; // tight W*H grayscale
    stbrp_rect Rect {};
  };

  class FreeTypeBitmapLoader
  {
    FT_Library _library = nullptr;
    FT_Face _face = nullptr;

  public:
    FreeTypeBitmapLoader() = default;

    ~FreeTypeBitmapLoader()
    {
      if (_face != nullptr)
      {
        FT_Done_Face(_face);
      }

      if (_library != nullptr)
      {
        FT_Done_FreeType(_library);
      }
    }

    bool Load(const IO::Path &path)
    {
      if (FT_Init_FreeType(&_library) != 0)
      {
        KRYS_ERROR("FREETYPE: Could not init FreeType Library");
        KRYS_DEBUG_BREAK();
        return false;
      }

      KRYS_INFO("FREETYPE: Loading bitmap font from '{}'", path.ToString());
      if (FT_New_Face(_library, path.ToString().c_str(), 0, &_face) != 0)
      {
        FT_Done_FreeType(_library);
        _library = nullptr;
        KRYS_ERROR("FREETYPE: Failed to load font '{}'", path.ToString());
        KRYS_DEBUG_BREAK();
        return false;
      }

      KRYS_INFO("FREETYPE: Loaded font.", path.ToString());
      KRYS_INFO("FREETYPE:   Family: {}", _face->family_name);
      KRYS_INFO("FREETYPE:   Style: {}", _face->style_name);
      KRYS_INFO("FREETYPE:   {} face(s) in font", _face->num_faces);

      return true;
    }

    bool SetPixelSize(uint32 height) const noexcept
    {
      KRYS_INFO("FREETYPE: Setting font size to {}px", height);
      if (FT_Set_Pixel_Sizes(_face, 0, height) != 0)
      {
        KRYS_ERROR("FREETYPE: Failed to set pixel size to {}px", height);
        KRYS_DEBUG_BREAK();
        return false;
      }

      return true;
    }

    List<BitmapGlyph> LoadGlyphs(uint8 paddingPerGlyph) const noexcept
    {
      List<BitmapGlyph> glyphs;
      glyphs.reserve(95);

      for (uchar c = 32; c < 127; c++)
      {
        if (FT_Load_Char(_face, c, FT_LOAD_RENDER))
        {
          KRYS_WARN("FREETYPE: Failed to load glyph {}", (int)c);
          continue;
        }

        if (_face->glyph->bitmap.pixel_mode != FT_PIXEL_MODE_GRAY)
        {
          KRYS_WARN("FREETYPE: Glyph {} not grayscale; skipping", (int)c);
          continue;
        }

        const FT_GlyphSlot slot = _face->glyph;
        const FT_Bitmap &bm = slot->bitmap;

        BitmapGlyph glyph {
          .Char = c,
          .ActualSize = {bm.width, bm.rows},
          .PaddedSize = glyph.ActualSize + (paddingPerGlyph * 2u),
          .Bearing = {slot->bitmap_left, slot->bitmap_top},
          .Advance = static_cast<int32>(slot->advance.x >> 6),
          .Pixels = {},
          .Rect = {},
        };

        // Copy to tight buffer (handle negative pitch)
        glyph.Pixels.resize(size_t(glyph.ActualSize.x) * glyph.ActualSize.y);
        for (uint row = 0; row < glyph.ActualSize.y; ++row)
        {
          const uint8 *src = (bm.pitch >= 0) ? (bm.buffer + row * bm.pitch)
                                             : (bm.buffer + (glyph.ActualSize.y - 1u - row) * (-bm.pitch));
          uint8 *dst = glyph.Pixels.data() + row * glyph.ActualSize.x;
          std::memcpy(dst, src, glyph.ActualSize.x);
        }

        glyphs.push_back(std::move(glyph));
      }
    }
  };

  Map<uchar, Character> ToCodepointsMap(List<BitmapGlyph> &glyphs, int padding, const Maths::Vec2u &atlasSize)
  {
    Map<uchar, Character> characters;
    characters.reserve(glyphs.size());

    for (const auto &glyph : glyphs)
    {
      const float u0 = float(glyph.Rect.x + padding) / float(atlasSize.x);
      const float v0 = float(glyph.Rect.y + padding) / float(atlasSize.y);
      const float u1 = float(glyph.Rect.x + padding + glyph.ActualSize.x) / float(atlasSize.x);
      const float v1 = float(glyph.Rect.y + padding + glyph.ActualSize.y) / float(atlasSize.y);

      Character ch = {
        .Size = glyph.ActualSize,
        .Bearing = glyph.Bearing,
        .Advance = glyph.Advance,
        .UVMin = {u0, v0},
        .UVMax = {u1, v1},
      };
      characters[glyph.Char] = ch;
    }

    return characters;
  }

  List<uint8> ToPixels(const List<BitmapGlyph> &glyphs, const Maths::Vec2u &atlasSize)
  {
    List<uint8> pixels(size_t(atlasSize.x) * atlasSize.y);
    std::fill(pixels.begin(), pixels.end(), 0);

    for (const auto &glyph : glyphs)
    {
      for (uint row = 0; row < glyph.ActualSize.y; row++)
      {
        uint8 *dst = pixels.data() + (glyph.Rect.y + row + 2) * atlasSize.x + (glyph.Rect.x + 2);
        const uint8 *src = glyph.Pixels.data() + row * glyph.ActualSize.x;
        std::memcpy(dst, src, glyph.ActualSize.x);
      }
    }
    return pixels;
  }

  bool TryPackGlyphs(List<BitmapGlyph> &glyphs, Maths::Vec2u &size)
  {
    while (true)
    {
      List<stbrp_rect> rects;
      rects.reserve(glyphs.size());

      for (size_t i = 0; i < glyphs.size(); i++)
      {
        glyphs[i].Rect.id = static_cast<int>(i);
        glyphs[i].Rect.w = glyphs[i].PaddedSize.x;
        glyphs[i].Rect.h = glyphs[i].PaddedSize.y;
        rects.push_back(glyphs[i].Rect);
      }

      stbrp_context ctx {};
      List<stbrp_node> nodes;
      nodes.resize(size.x);

      stbrp_init_target(&ctx, size.x, size.y, nodes.data(), (int)nodes.size());
      stbrp_pack_rects(&ctx, rects.data(), (int)rects.size());

      if (std::all_of(rects.begin(), rects.end(), [](const auto &r) { return r.was_packed; }))
      {
        for (const auto &r : rects) // write back positions
        {
          glyphs[r.id].Rect = r;
        }
        break;
      }

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
        KRYS_ERROR("Font atlas exceeded 4096x4096!");
        KRYS_DEBUG_BREAK();
        return false;
      }
    }

    return true;
  }
}

namespace Krys::Gfx
{
  Expected<FontAtlasData> FontAtlasLoader::LoadBitmap(const IO::Path &path, uint32 fontSizeInPixels,
                                                        uint8 paddingPerGlyph) noexcept
  {
    FreeTypeBitmapLoader ft {};
    if (!ft.Load(path))
    {
      return Unexpected("Failed to load font");
    }

    if (!ft.SetPixelSize(fontSizeInPixels))
    {
      return Unexpected("Failed to set font size");
    }

    List<BitmapGlyph> glyphs = ft.LoadGlyphs(paddingPerGlyph);
    if (glyphs.empty())
    {
      KRYS_ERROR("FREETYPE: No glyphs loaded for font '{}'", path.ToString());
      KRYS_DEBUG_BREAK();
      return Unexpected("No glyphs loaded");
    }

    auto atlasSize = Maths::Vec2u {512u, 512u}; // Will be resized as needed.
    if (!TryPackGlyphs(glyphs, atlasSize))
    {
      KRYS_ERROR("FREETYPE: Failed to pack glyphs for font '{}'", path.ToString());
      KRYS_DEBUG_BREAK();
      return Unexpected("Unable to pack glyphs");
    }

    FontAtlasData result {};
    result.Size = atlasSize;
    result.Pixels = ToPixels(glyphs, result.Size);
    result.Characters = ToCodepointsMap(glyphs, paddingPerGlyph, result.Size);
    return result;
  }
}