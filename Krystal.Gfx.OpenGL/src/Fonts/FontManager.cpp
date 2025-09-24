#pragma once

#include "Krystal.Gfx.OpenGL/Fonts/Font.hpp"
#include "Krystal.Gfx.OpenGL/Fonts/FontSystem.hpp"
#include "Krystal.Gfx/Handle.hpp"
#include "Krystal.Lib/DebugBreak.hpp"
#include "Krystal.Lib/Map.hpp"
#include "Krystal.Log/ILogger.hpp"
#include <ft2build.h>
#include <msdf-atlas-gen.h>
#include <msdfgen-ext.h>
#include <msdfgen.h>
#include <stb_rect_pack.h>
#include FT_FREETYPE_H
#include <algorithm>

namespace
{
  using namespace Krys;
  using namespace Krys::Maths;
  using namespace Krys::Gfx::OpenGL;

  struct GlyphToPack
  {
    char Char {};
    Vec2u BitmapSize {};   // glyph bitmap size (tight)
    Vec2u PackedSize {};   // packed rect size incl. padding
    Vec2i Bearing {};      // slot->bitmap_left/top
    uint32 Advance {};     // pixels (>>6)
    List<uint8> Pixels {}; // tight W*H grayscale
    stbrp_rect Rect {};
  };

  struct BitmapLoadResult
  {
    List<GlyphToPack> Glyphs;
    int Padding {0};
    Vec2u AtlasSize {0u};
    bool Success {false};
  };

  bool TryPack(List<GlyphToPack> &glyphs, Vec2u &size)
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
          if (auto *logger = Log::GetGlobalLogger(); logger != nullptr)
            logger->Error("Font atlas exceeded 4096 size");
          return false;
        }
      }
    }

    return true;
  }

  BitmapLoadResult LoadBitmapAtlas(const IO::Path &path, float size, Log::ILogger *logger)
  {
    const int padding = 2; // pixels of padding around each glyph

    BitmapLoadResult result {};
    result.Glyphs.reserve(95);
    result.Padding = padding;

    FT_Library fontLibraryHandle = nullptr;
    if (FT_Init_FreeType(&fontLibraryHandle))
    {
      if (logger)
      {
        logger->Error("FREETYPE: Could not init FreeType Library");
      }
      return result;
    }

    FT_Face face {};

    if (FT_New_Face(fontLibraryHandle, path.ToString().c_str(), 0, &face))
    {
      FT_Done_FreeType(fontLibraryHandle);
      if (logger)
      {
        logger->Error("FREETYPE: Failed to load font '{}'", path.ToString());
      }
      return result;
    }

    if (FT_Set_Pixel_Sizes(face, 0, (uint32)size))
    {
      FT_Done_Face(face);
      FT_Done_FreeType(fontLibraryHandle);
      if (logger)
      {
        logger->Error("FREETYPE: Failed to set font size {}px", (uint32)size);
      }
      return result;
    }

    for (uchar c = 32; c < 127; c++)
    {
      if (FT_Load_Char(face, c, FT_LOAD_RENDER))
      {
        if (logger)
          logger->Warn("FREETYPE: Failed to load glyph {}", (int)c);
        continue;
      }

      if (face->glyph->bitmap.pixel_mode != FT_PIXEL_MODE_GRAY)
      {
        if (logger)
          logger->Warn("FREETYPE: Glyph {} not grayscale; skipping", (int)c);
        continue;
      }

      const FT_GlyphSlot slot = face->glyph;
      const FT_Bitmap &bm = slot->bitmap;

      GlyphToPack glyph {};
      glyph.Char = c;
      glyph.BitmapSize = {bm.width, bm.rows};
      glyph.Bearing = {slot->bitmap_left, slot->bitmap_top};
      glyph.Advance = (uint32)(slot->advance.x >> 6);

      // Copy to tight buffer (handle negative pitch)
      glyph.Pixels.resize(glyph.BitmapSize.x * glyph.BitmapSize.y);
      for (uint row = 0; row < glyph.BitmapSize.y; ++row)
      {
        const uint8 *src = (bm.pitch >= 0) ? (bm.buffer + row * bm.pitch)
                                           : (bm.buffer + (glyph.BitmapSize.y - 1 - row) * (-bm.pitch));
        uint8 *dst = glyph.Pixels.data() + row * glyph.BitmapSize.x;
        std::memcpy(dst, src, glyph.BitmapSize.x);
      }

      glyph.PackedSize = {glyph.BitmapSize.x + padding * 2, glyph.BitmapSize.y + padding * 2};
      result.Glyphs.push_back(std::move(glyph));
    }

    FT_Done_Face(face);
    FT_Done_FreeType(fontLibraryHandle);

    result.Success = true;

    result.AtlasSize = {512, 512};
    if (!TryPack(result.Glyphs, result.AtlasSize))
    {
      if (logger)
      {
        logger->Error("FREETYPE: Failed to pack glyphs for font '{}'", path.ToString());
      }
      KRYS_DEBUG_BREAK();
      return {};
    }

    return result;
  }
}

namespace Krys::Gfx::OpenGL
{
  FontSystem::FontSystem() noexcept
  {
  }

  FontSystem::~FontSystem() noexcept
  {
  }

  FontHandle FontSystem::Load(const IO::Path &path, float size, FontType fontType) noexcept
  {
    string key = std::format("{0}-{1}-{2}", path.ToString(), size, (int)fontType);
    if (auto cached = _cache.Get(key); cached.IsValid())
      return cached;

    auto *logger = Log::GetGlobalLogger();

    FontHandle handle;
    if (fontType == FontType::Bitmap)
    {
      BitmapLoadResult result = LoadBitmapAtlas(path, size, logger);
      if (!result.Success)
      {
        if (logger)
          logger->Error("Failed to load font '{}'", path.ToString());
        return {};
      }

      List<uint8> atlas(result.AtlasSize.x * result.AtlasSize.y, 0);
      for (const auto &glyph : result.Glyphs)
      {
        const int dstX = glyph.Rect.x + result.Padding;
        const int dstY = glyph.Rect.y + result.Padding;
        for (uint row = 0; row < glyph.BitmapSize.y; ++row)
        {
          const uint8 *src = glyph.Pixels.data() + row * glyph.BitmapSize.x;
          uint8 *dst = atlas.data() + (dstY + row) * result.AtlasSize.x + dstX;
          std::memcpy(dst, src, glyph.BitmapSize.x);
        }
      }

      glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
      GLuint atlasTexture = 0;
      glCreateTextures(GL_TEXTURE_2D, 1, &atlasTexture);
      glTextureStorage2D(atlasTexture, 1, GL_R8, result.AtlasSize.x, result.AtlasSize.y);
      glTextureSubImage2D(atlasTexture, 0, 0, 0, result.AtlasSize.x, result.AtlasSize.y, GL_RED,
                          GL_UNSIGNED_BYTE, atlas.data());
      glTextureParameteri(atlasTexture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTextureParameteri(atlasTexture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTextureParameteri(atlasTexture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTextureParameteri(atlasTexture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

      Map<char, Character> characters;
      characters.reserve(95);

      for (const auto &glyph : result.Glyphs)
      {
        const float u0 = float(glyph.Rect.x + result.Padding) / float(result.AtlasSize.x);
        const float v0 = float(glyph.Rect.y + result.Padding) / float(result.AtlasSize.y);
        const float u1 =
          float(glyph.Rect.x + result.Padding + glyph.BitmapSize.x) / float(result.AtlasSize.x);
        const float v1 =
          float(glyph.Rect.y + result.Padding + glyph.BitmapSize.y) / float(result.AtlasSize.y);

        Character ch = {
          .Size = glyph.BitmapSize,
          .Bearing = glyph.Bearing,
          .Advance = glyph.Advance,
          .UVMin = {u0, v0},
          .UVMax = {u1, v1},
        };
        characters[glyph.Char] = ch;
      }

      Font font {fontType, {atlasTexture, characters, result.AtlasSize}};
      handle = _fonts.Add(std::move(font));
      _cache.Add(key, handle);
    }

    return handle;
  }

  Font &FontSystem::Get(FontHandle handle)
  {
    return _fonts.Get(handle);
  }
}