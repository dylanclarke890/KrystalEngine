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

  struct MTSDFResult
  {
    GLuint Texture {0u};
    Map<char, Character> Characters {};
    Vec2u AtlasSize {};
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

  MTSDFResult LoadMTSDFAtlas(const IO::Path &path, float size, Log::ILogger *logger, Gfx::FontType fontType)
  {
    MTSDFResult result {};

    using namespace msdf_atlas;

    // Initialize instance of FreeType library
    if (msdfgen::FreetypeHandle *ft = msdfgen::initializeFreetype())
    {
      // Load font file
      if (msdfgen::FontHandle *font = msdfgen::loadFont(ft, path.ToString().c_str()))
      {
        // Storage for glyph geometry and their coordinates in the atlas
        std::vector<GlyphGeometry> glyphs;

        // FontGeometry is a helper class that loads a set of glyphs from a single font.
        // It can also be used to get additional font metrics, kerning information, etc.
        FontGeometry fontGeometry(&glyphs);

        // Load a set of character glyphs:
        // The second argument can be ignored unless you mix different font sizes in one atlas.
        // In the last argument, you can specify a charset other than ASCII.
        // To load specific glyph indices, use loadGlyphs instead.
        fontGeometry.loadCharset(font, 1.0, Charset::ASCII);

        // Apply MSDF edge coloring. See edge-coloring.h for other coloring strategies.
        const double maxCornerAngle = 3.0;
        for (GlyphGeometry &glyph : glyphs)
          glyph.edgeColoring(&msdfgen::edgeColoringInkTrap, maxCornerAngle, 0);

        // TightAtlasPacker class computes the layout of the atlas.
        TightAtlasPacker packer;
        // Set atlas parameters:
        // setDimensions or setDimensionsConstraint to find the best value
        packer.setDimensionsConstraint(DimensionsConstraint::SQUARE);
        // setScale for a fixed size or setMinimumScale to use the largest that fits
        packer.setMinimumScale(size);
        // setPixelRange or setUnitRange
        packer.setPixelRange(4.0);
        packer.setMiterLimit(1.0);
        // Compute atlas layout - pack glyphs
        packer.pack(glyphs.data(), (int)glyphs.size());
        // Get final atlas dimensions
        int width = 0, height = 0;
        packer.getDimensions(width, height);

        // TODO: generate based on fontType (SDF, MSDF, MTSDF)
        // The ImmediateAtlasGenerator class facilitates the generation of the atlas bitmap.
        ImmediateAtlasGenerator<
          float,         // pixel type of buffer for individual glyphs depends on generator function
          3,             // number of atlas color channels
          msdfGenerator, // function to generate bitmaps for individual glyphs
          BitmapAtlasStorage<msdfgen::byte, 3> // class that stores the atlas bitmap
          // For example, a custom atlas storage class that stores it in VRAM can be used.
          >
          generator(width, height);
        // GeneratorAttributes can be modified to change the generator's default settings.
        GeneratorAttributes attributes;
        generator.setAttributes(attributes);
        generator.setThreadCount(4);
        // Generate atlas bitmap
        generator.generate(glyphs.data(), (int)glyphs.size());

        // The atlas bitmap can now be retrieved via atlasStorage as a BitmapConstRef.
        // The glyphs array (or fontGeometry) contains positioning data for typesetting text.
        // success = my_project::submitAtlasBitmapAndLayout(generator.atlasStorage(), glyphs);
        const auto &storage = generator.atlasStorage();
        const msdfgen::BitmapConstRef<msdfgen::byte, 3> &atlas = storage;

        // Upload to OpenGL texture
        GLuint tex;
        glCreateTextures(GL_TEXTURE_2D, 1, &tex);
        glTextureStorage2D(tex, 1, GL_RGB8, atlas.width, atlas.height);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTextureSubImage2D(tex, 0, 0, 0, atlas.width, atlas.height, GL_RGB, GL_UNSIGNED_BYTE, atlas.pixels);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
        glTextureParameteri(tex, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTextureParameteri(tex, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTextureParameteri(tex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTextureParameteri(tex, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        result.Texture = tex;
        result.AtlasSize = {uint32(atlas.width), uint32(atlas.height)};
        result.Success = true;

        double emScale = packer.getScale();
        for (const GlyphGeometry &glyph : glyphs)
        {
          Character ch {};
          double b, t, l, r;
          glyph.getQuadAtlasBounds(l, b, r, t);

          double pl, pb, pr, pt;
          glyph.getQuadPlaneBounds(pl, pb, pr, pt);

          ch.Size = {(uint32)std::round((pr - pl) * emScale), (uint32)std::round((pt - pb) * emScale)};
          ch.Bearing = {(int)std::round(pl * emScale), (int)std::round(pt * emScale)};
          ch.Advance = (uint32)std::round(glyph.getAdvance() * emScale);
          ch.UVMin = {float(l / atlas.width), float(t / atlas.height)};
          ch.UVMax = {float(r / atlas.width), float(b / atlas.height)};
          result.Characters.emplace((char)glyph.getCodepoint(), ch);
        }

        // Cleanup
        msdfgen::destroyFont(font);
      }
      msdfgen::deinitializeFreetype(ft);
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
    else
    {
      MTSDFResult result = LoadMTSDFAtlas(path, size, logger, fontType);

      if (!result.Success)
      {
        if (logger)
        {
          logger->Error("Failed to load font '{}'", path.ToString());
        }
        return {};
      }

      Font font {fontType, {result.Texture, result.Characters, result.AtlasSize}};
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