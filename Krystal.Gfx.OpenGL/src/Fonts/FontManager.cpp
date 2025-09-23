#pragma once

#include "Krystal.Gfx.OpenGL/Fonts/Font.hpp"
#include "Krystal.Gfx.OpenGL/Fonts/FontSystem.hpp"
#include "Krystal.Gfx/Handle.hpp"
#include "Krystal.Lib/DebugBreak.hpp"
#include "Krystal.Lib/Map.hpp"
#include "Krystal.Log/ILogger.hpp"
#include <ft2build.h>
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

  struct PackedGlyph
  {
    char Char {};
    Vec2u BitmapSize {};   // glyph bitmap size (tight)
    Vec2u PackedSize {};   // packed rect size incl. padding
    Vec2i Bearing {};      // slot->bitmap_left/top
    uint32 Advance {};     // pixels (>>6)
    List<uint8> Pixels {}; // tight W*H grayscale
    stbrp_rect Rect {};
  };

  struct GlyphPackResult
  {
    List<PackedGlyph> Glyphs;
    int Padding {0};
    bool Success {false};
  };

  static bool TryPack(List<PackedGlyph> &glyphs, Vec2u &size)
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

  GlyphPackResult LoadBitmapGlyphs(const IO::Path &path, float size, Log::ILogger *logger)
  {
    const int padding = 2; // pixels of padding around each glyph

    GlyphPackResult result {};
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

      PackedGlyph glyph {};
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

    result.Success = true;
    FT_Done_Face(face);
    FT_Done_FreeType(fontLibraryHandle);

    return result;
  }

  GlyphPackResult LoadMTSDFGlyphs(const IO::Path &path, float size, Log::ILogger *logger,
                                  Gfx::FontType fontType)
  {
    using namespace msdfgen;

    const int padding = 8;       // SDF needs more padding than bmp fonts so the distance field isn't clipped
    const double pxPerEm = size; // because we're going to load glyphs as EM normalized
    ulong seed = 0;              // for edgeColoringSimple randomness

    GlyphPackResult result {};
    result.Glyphs.reserve(95);
    result.Padding = padding;

    if (FreetypeHandle *ft = initializeFreetype())
    {
      if (FontHandle *font = loadFont(ft, path.ToString().c_str()))
      {
        for (uchar c = 32; c < 127; ++c)
        {
          Shape shape;
          if (!loadGlyph(shape, font, c, FONT_SCALING_EM_NORMALIZED))
          {
            if (logger)
              logger->Warn("MSDFGEN: Failed to load outline for '{}'", (int)c);
            continue;
          }

          shape.normalize();
          if (fontType == Gfx::FontType::SDF)
          {
            BitmapRef<float, 1> bitmap;
            generateSDF(bitmap, shape, SDFTransformation(), GeneratorConfig());
          }
          else if (fontType == Gfx::FontType::MSDF)
          {
            edgeColoringSimple(shape, 3.0, ++seed);
          }
        }
        destroyFont(font);
      }
      else if (logger)
      {
        logger->Error("FREETYPE: Could not init FreeType Library");
      }
      deinitializeFreetype(ft);
    }
    else if (logger)
    {
      logger->Error("FREETYPE: Could not init FreeType Library");
    }

    result.Success = true;
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

    GlyphPackResult packResult = [&]()
    {
      switch (fontType)
      {
        case FontType::Bitmap: return LoadBitmapGlyphs(path, size, logger);
        case FontType::SDF:
        case FontType::MSDF:
        default:
          if (logger)
            logger->Error("FREETYPE: Unsupported font type for font '{}'", path.ToString());
          KRYS_DEBUG_BREAK();
          return LoadBitmapGlyphs(path, size, logger);
      }
      return LoadBitmapGlyphs(path, size, logger);
    }();

    if (!packResult.Success)
    {
      if (logger)
      {
        logger->Error("FREETYPE: Failed to load glyphs for font '{}'", path.ToString());
      }
      KRYS_DEBUG_BREAK();
      return {};
    }

    Vec2u atlasSize {512, 512};
    if (!TryPack(packResult.Glyphs, atlasSize))
    {
      if (logger)
      {
        logger->Error("FREETYPE: Failed to pack glyphs for font '{}'", path.ToString());
      }
      KRYS_DEBUG_BREAK();
      return {};
    }

    List<uint8> atlas(atlasSize.x * atlasSize.y, 0);
    for (const auto &glyph : packResult.Glyphs)
    {
      const int dstX = glyph.Rect.x + packResult.Padding;
      const int dstY = glyph.Rect.y + packResult.Padding;
      for (uint row = 0; row < glyph.BitmapSize.y; ++row)
      {
        const uint8 *src = glyph.Pixels.data() + row * glyph.BitmapSize.x;
        uint8 *dst = atlas.data() + (dstY + row) * atlasSize.x + dstX;
        std::memcpy(dst, src, glyph.BitmapSize.x);
      }
    }

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    GLuint atlasTexture = 0;
    glCreateTextures(GL_TEXTURE_2D, 1, &atlasTexture);
    glTextureStorage2D(atlasTexture, 1, GL_R8, atlasSize.x, atlasSize.y);
    glTextureSubImage2D(atlasTexture, 0, 0, 0, atlasSize.x, atlasSize.y, GL_RED, GL_UNSIGNED_BYTE,
                        atlas.data());
    glTextureParameteri(atlasTexture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(atlasTexture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(atlasTexture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(atlasTexture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

    Map<char, Character> characters;
    characters.reserve(95);

    for (const auto &glyph : packResult.Glyphs)
    {
      const float u0 = float(glyph.Rect.x + packResult.Padding) / float(atlasSize.x);
      const float v0 = float(glyph.Rect.y + packResult.Padding) / float(atlasSize.y);
      const float u1 = float(glyph.Rect.x + packResult.Padding + glyph.BitmapSize.x) / float(atlasSize.x);
      const float v1 = float(glyph.Rect.y + packResult.Padding + glyph.BitmapSize.y) / float(atlasSize.y);

      Character ch = {
        .Size = glyph.BitmapSize,
        .Bearing = glyph.Bearing,
        .Advance = glyph.Advance,
        .UVMin = {u0, v0},
        .UVMax = {u1, v1},
      };
      characters[glyph.Char] = ch;
    }

    Font font {{atlasTexture, characters, atlasSize}};
    FontHandle handle = _fonts.Add(std::move(font));
    _cache.Add(key, handle);

    return handle;
  }

  Font &FontSystem::Get(FontHandle handle)
  {
    return _fonts.Get(handle);
  }
}