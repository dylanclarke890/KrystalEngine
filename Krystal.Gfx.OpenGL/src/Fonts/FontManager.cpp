#pragma once

#include "Krystal.Gfx.OpenGL/Fonts/Font.hpp"
#include "Krystal.Gfx.OpenGL/Fonts/FontSystem.hpp"
#include "Krystal.Gfx/Handle.hpp"
#include "Krystal.Lib/DebugBreak.hpp"
#include "Krystal.Lib/Map.hpp"
#include "Krystal.Log/ILogger.hpp"
#include <ft2build.h>
#include <msdfgen.h>
#include <msdfgen-ext.h>
#include <stb_rect_pack.h>
#include FT_FREETYPE_H
#include <algorithm>

namespace
{
  using namespace Krys;
  using namespace Krys::Maths;
  using namespace Krys::Gfx::OpenGL;

  FT_Library FontLibraryHandle = nullptr;

  struct PackedGlyph
  {
    uchar ch {};
    int bmpW {}, bmpH {};         // glyph bitmap size (tight)
    int bearingX {}, bearingY {}; // slot->bitmap_left/top
    uint32 advance {};            // pixels (>>6)
    List<uint8> pixels;           // tight W*H grayscale
    int w {}, h {};               // packed rect size incl. padding
    stbrp_rect rect {};
  };

  static bool TryPack(List<PackedGlyph> &glyphs, int atlasW, int atlasH)
  {
    List<stbrp_node> nodes(atlasW);

    stbrp_context ctx {};
    stbrp_init_target(&ctx, atlasW, atlasH, nodes.data(), (int)nodes.size());

    List<stbrp_rect> rects;
    rects.reserve(glyphs.size());

    for (int i = 0; i < glyphs.size(); i++)
    {
      glyphs[i].rect = {};
      glyphs[i].rect.id = i;
      glyphs[i].rect.w = glyphs[i].w;
      glyphs[i].rect.h = glyphs[i].h;
      rects.push_back(glyphs[i].rect);
    }

    stbrp_pack_rects(&ctx, rects.data(), (int)rects.size());

    bool allPacked = std::any_of(rects.begin(), rects.end(), [](const auto &r) { return r.was_packed; });
    if (!allPacked)
    {
      return false;
    }

    // write back positions
    for (auto &r : rects)
    {
      glyphs[r.id].rect = r;
    }
    return true;
  }
}

namespace Krys::Gfx::OpenGL
{
  FontSystem::FontSystem() noexcept
  {
    if (FT_Init_FreeType(&FontLibraryHandle))
    {
      auto *logger = Log::GetGlobalLogger();
      if (logger)
      {
        logger->Error("FREETYPE: Could not init FreeType Library");
        KRYS_DEBUG_BREAK();
      }
    }
  }

  FontSystem::~FontSystem() noexcept
  {
    if (FontLibraryHandle)
    {
      FT_Done_FreeType(FontLibraryHandle);
      FontLibraryHandle = nullptr;
    }
  }

  FontHandle FontSystem::Load(const IO::Path &path, float size) noexcept
  {
    string key = std::format("{0}-{1}", path.ToString(), size);
    if (auto cached = _cache.Get(key); cached.IsValid())
      return cached;

    auto *logger = Log::GetGlobalLogger();

    FT_Face face {};
    if (FT_New_Face(FontLibraryHandle, path.ToString().c_str(), 0, &face))
    {
      if (logger)
        logger->Error("FREETYPE: Failed to load font '{}'", path.ToString());
      KRYS_DEBUG_BREAK();
      return {};
    }

    if (FT_Set_Pixel_Sizes(face, 0, (uint32)size))
    {
      FT_Done_Face(face);
      if (logger)
        logger->Error("FREETYPE: Failed to set font size {}px", (uint32)size);
      KRYS_DEBUG_BREAK();
      return {};
    }

    const int padding = 2; // pixels of padding around each glyph
    List<PackedGlyph> packList;
    packList.reserve(95);

    for (uchar c = 32; c < 127; ++c)
    {
      if (FT_Load_Char(face, c, FT_LOAD_RENDER))
      {
        if (logger)
          logger->Warn("FREETYPE: Failed to load glyph {}", (int)c);
        continue;
      }
      const FT_GlyphSlot slot = face->glyph;
      const FT_Bitmap &bm = slot->bitmap;

      if (bm.pixel_mode != FT_PIXEL_MODE_GRAY)
      {
        if (logger)
          logger->Warn("FREETYPE: Glyph {} not grayscale; skipping", (int)c);
        continue;
      }

      PackedGlyph pg {};
      pg.ch = c;
      pg.bmpW = (int)bm.width;
      pg.bmpH = (int)bm.rows;
      pg.bearingX = slot->bitmap_left;
      pg.bearingY = slot->bitmap_top;
      pg.advance = (uint32)(slot->advance.x >> 6);

      // Copy to tight buffer (handle negative pitch)
      pg.pixels.resize(pg.bmpW * pg.bmpH);
      for (int row = 0; row < pg.bmpH; ++row)
      {
        const uint8 *src =
          (bm.pitch >= 0) ? (bm.buffer + row * bm.pitch) : (bm.buffer + (pg.bmpH - 1 - row) * (-bm.pitch));
        uint8 *dst = pg.pixels.data() + row * pg.bmpW;
        std::memcpy(dst, src, pg.bmpW);
      }

      // Packed rect size with padding
      pg.w = pg.bmpW + padding * 2;
      pg.h = pg.bmpH + padding * 2;

      packList.push_back(std::move(pg));
    }

    uint atlasW = 512, atlasH = 512;
    while (!TryPack(packList, atlasW, atlasH))
    {
      if (atlasW <= atlasH)
      {
        atlasW *= 2;
      }
      else
      {
        atlasH *= 2;
      }

      if (atlasW > 4'096 || atlasH > 4'096)
      {
        if (logger)
          logger->Error("Font atlas exceeded 4096 size for '{}'", path.ToString());
        FT_Done_Face(face);
        KRYS_DEBUG_BREAK();
        return {};
      }
    }

    List<uint8> atlas(atlasW * atlasH, 0);

    auto blitGlyph = [&](const PackedGlyph &g)
    {
      const int dstX = g.rect.x + padding;
      const int dstY = g.rect.y + padding;
      for (int row = 0; row < g.bmpH; ++row)
      {
        const uint8 *src = g.pixels.data() + row * g.bmpW;
        uint8 *dst = atlas.data() + (dstY + row) * atlasW + dstX;
        std::memcpy(dst, src, g.bmpW);
      }
    };

    for (const auto &g : packList)
      blitGlyph(g);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    GLuint atlasTex = 0;
    glCreateTextures(GL_TEXTURE_2D, 1, &atlasTex);
    glTextureStorage2D(atlasTex, 1, GL_R8, atlasW, atlasH);
    glTextureSubImage2D(atlasTex, 0, 0, 0, atlasW, atlasH, GL_RED, GL_UNSIGNED_BYTE, atlas.data());
    glTextureParameteri(atlasTex, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(atlasTex, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(atlasTex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(atlasTex, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

    Map<char, Character> characters;
    characters.reserve(95);

    for (const auto &g : packList)
    {
      const float u0 = float(g.rect.x + padding) / float(atlasW);
      const float v0 = float(g.rect.y + padding) / float(atlasH);
      const float u1 = float(g.rect.x + padding + g.bmpW) / float(atlasW);
      const float v1 = float(g.rect.y + padding + g.bmpH) / float(atlasH);

      Character ch = {
        .Size = Vec2u {(uint)g.bmpW, (uint)g.bmpH},
        .Bearing = Vec2i {g.bearingX, g.bearingY},
        .Advance = g.advance,
        .UVMin = {u0, v0},
        .UVMax = {u1, v1},
      };
      characters[(char)g.ch] = ch;
    }

    Font font {{atlasTex, characters, {atlasW, atlasH}}};
    FT_Done_Face(face);

    FontHandle handle = _fonts.Add(std::move(font));
    _cache.Add(key, handle);

    return handle;
  }

  Font &FontSystem::Get(FontHandle handle)
  {
    return _fonts.Get(handle);
  }
}