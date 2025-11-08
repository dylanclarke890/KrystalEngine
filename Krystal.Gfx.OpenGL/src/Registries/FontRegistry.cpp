#pragma once

#include "Krystal.Gfx.OpenGL/Registries/FontRegistry.hpp"
#include "Krystal.Gfx.OpenGL/Context.hpp"
#include "Krystal.Gfx.OpenGL/Resources/Font.hpp"
#include "Krystal.Gfx/Handle.hpp"
#include "Krystal.Lib/DebugBreak.hpp"
#include "Krystal.Lib/Map.hpp"
#include "Krystal.Log/ILogger.hpp"
#include "Krystal.Platform/Platform.hpp"
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
          KRYS_ERROR("Font atlas exceeded 4096 size");
          KRYS_DEBUG_BREAK();
          return false;
        }
      }
    }

    return true;
  }

  BitmapLoadResult LoadBitmapAtlas(const IO::Path &path, double sizeInPixels)
  {
    const int padding = 2; // pixels of padding around each glyph

    BitmapLoadResult result {};
    result.Glyphs.reserve(95);
    result.Padding = padding;

    FT_Library fontLibraryHandle = nullptr;
    if (FT_Init_FreeType(&fontLibraryHandle))
    {
      KRYS_ERROR("FREETYPE: Could not init FreeType Library");
      KRYS_DEBUG_BREAK();
      return result;
    }

    KRYS_INFO("FREETYPE: Loading bitmap font from '{}'", path.ToString());
    FT_Face face {};
    if (FT_New_Face(fontLibraryHandle, path.ToString().c_str(), 0, &face))
    {
      FT_Done_FreeType(fontLibraryHandle);
      KRYS_ERROR("FREETYPE: Failed to load font '{}'", path.ToString());
      return result;
    }

    KRYS_INFO("FREETYPE: Loaded font.", path.ToString());
    KRYS_INFO("FREETYPE:   Family: {}", face->family_name);
    KRYS_INFO("FREETYPE:   Style: {}", face->style_name);
    KRYS_INFO("FREETYPE:   {} face(s) in font", face->num_faces);
    KRYS_INFO("FREETYPE: Setting font size to {}px", (uint32)sizeInPixels);
    if (FT_Set_Pixel_Sizes(face, 0, (uint32)sizeInPixels))
    {
      FT_Done_Face(face);
      FT_Done_FreeType(fontLibraryHandle);
      KRYS_ERROR("FREETYPE: Failed to set font size {}px", (uint32)sizeInPixels);
      KRYS_DEBUG_BREAK();
      return result;
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
      glyph.Advance = (uint32)(slot->advance.x >> 6);

      // Copy to tight buffer (handle negative pitch)
      glyph.Pixels.resize(size_t(glyph.BitmapSize.x) * glyph.BitmapSize.y);
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
      KRYS_ERROR("FREETYPE: Failed to pack glyphs for font '{}'", path.ToString());
      KRYS_DEBUG_BREAK();
      return {};
    }

    return result;
  }

  Map<char, Character> CreateBitmapCharacters(List<GlyphToPack> &glyphs, int padding,
                                              const Maths::Vec2u &atlasSize)
  {
    // Create character map
    Map<char, Character> characters;
    characters.reserve(95);
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

  List<uint8> CreateBitmapAtlasPixels(List<GlyphToPack> &glyphs, const Maths::Vec2u &atlasSize)
  {
    List<uint8> pixels(size_t(atlasSize.x) * atlasSize.y);
    std::fill(pixels.begin(), pixels.end(), 0);

    for (const auto &glyph : glyphs)
    {
      for (uint row = 0; row < glyph.BitmapSize.y; ++row)
      {
        uint8 *dst = pixels.data() + (glyph.Rect.y + row + 2) * atlasSize.x + (glyph.Rect.x + 2);
        const uint8 *src = glyph.Pixels.data() + row * glyph.BitmapSize.x;
        std::memcpy(dst, src, glyph.BitmapSize.x);
      }
    }
    return pixels;
  }

  MTSDFResult LoadMTSDFAtlas(const IO::Path &path, Gfx::FontType fontType, const SDFParams &params)
  {
    MTSDFResult result {};

    using namespace msdf_atlas;

    if (msdfgen::FreetypeHandle *ft = msdfgen::initializeFreetype())
    {
      if (msdfgen::FontHandle *font = msdfgen::loadFont(ft, path.ToString().c_str()))
      {
        // FontGeometry is a helper class that loads a set of glyphs from a single font.
        // It can also be used to get additional font metrics, kerning information, etc.
        List<GlyphGeometry> glyphs;
        FontGeometry fontGeometry(&glyphs);
        fontGeometry.loadCharset(font, 1.0, Charset::ASCII);

        const double maxCornerAngle = 3.0; // Apply MSDF edge coloring. edge-coloring.h for other strategies.
        for (GlyphGeometry &glyph : glyphs)
          glyph.edgeColoring(&msdfgen::edgeColoringInkTrap, maxCornerAngle, 0);

        int width = 0, height = 0;
        TightAtlasPacker packer;
        packer.setDimensionsConstraint(DimensionsConstraint::SQUARE);
        packer.setScale(params.EMSizeInPixels);
        packer.setPixelRange(params.PixelRange);
        packer.setMiterLimit(params.MiterLimit);
        packer.pack(glyphs.data(), (int)glyphs.size());
        packer.getDimensions(width, height); // final atlas dimensions

        GeneratorAttributes attributes {};
        attributes.scanlinePass = true;

        GLuint texture;
        glCreateTextures(GL_TEXTURE_2D, 1, &texture);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        if (fontType == Gfx::FontType::SDF)
        {
          using SDFAtlasData = const msdfgen::BitmapConstRef<msdfgen::byte, 1> &;
          using SDFGenerator =
            ImmediateAtlasGenerator<float, 1, sdfGenerator, BitmapAtlasStorage<msdfgen::byte, 1>>;

          SDFGenerator generator(width, height);
          generator.setAttributes(attributes);
          generator.setThreadCount(4);
          generator.generate(glyphs.data(), (int)glyphs.size());
          SDFAtlasData atlas = generator.atlasStorage();

          glTextureStorage2D(texture, 1, GL_R8, atlas.width, atlas.height);
          glTextureSubImage2D(texture, 0, 0, 0, atlas.width, atlas.height, GL_RED, GL_UNSIGNED_BYTE,
                              atlas.pixels);

          result.Texture = texture;
          result.AtlasSize = {uint32(atlas.width), uint32(atlas.height)};
        }
        else if (fontType == Gfx::FontType::MSDF)
        {
          using MSDFAtlasData = const msdfgen::BitmapConstRef<msdfgen::byte, 3> &;
          using MSDFGenerator =
            ImmediateAtlasGenerator<float, 3, msdfGenerator, BitmapAtlasStorage<msdfgen::byte, 3>>;

          MSDFGenerator generator(width, height);
          generator.setAttributes(attributes);
          generator.setThreadCount(4);
          generator.generate(glyphs.data(), (int)glyphs.size());
          MSDFAtlasData atlas = generator.atlasStorage();

          glTextureStorage2D(texture, 1, GL_RGB8, atlas.width, atlas.height);
          glTextureSubImage2D(texture, 0, 0, 0, atlas.width, atlas.height, GL_RGB, GL_UNSIGNED_BYTE,
                              atlas.pixels);

          result.Texture = texture;
          result.AtlasSize = {uint32(atlas.width), uint32(atlas.height)};
        }
        else
        {
          using MTSDFAtlasData = const msdfgen::BitmapConstRef<msdfgen::byte, 4> &;
          using MTSDFGenerator =
            ImmediateAtlasGenerator<float, 4, mtsdfGenerator, BitmapAtlasStorage<msdfgen::byte, 4>>;

          MTSDFGenerator generator(width, height);
          generator.setAttributes(attributes);
          generator.setThreadCount(4);
          generator.generate(glyphs.data(), (int)glyphs.size());
          MTSDFAtlasData atlas = generator.atlasStorage();

          glTextureStorage2D(texture, 1, GL_RGBA8, atlas.width, atlas.height);
          glTextureSubImage2D(texture, 0, 0, 0, atlas.width, atlas.height, GL_RGBA, GL_UNSIGNED_BYTE,
                              atlas.pixels);

          result.Texture = texture;
          result.AtlasSize = {uint32(atlas.width), uint32(atlas.height)};
        }
        glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

        glTextureParameteri(result.Texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTextureParameteri(result.Texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTextureParameteri(result.Texture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTextureParameteri(result.Texture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
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
          ch.UVMin = {float(l / result.AtlasSize.x), float(t / result.AtlasSize.y)};
          ch.UVMax = {float(r / result.AtlasSize.x), float(b / result.AtlasSize.y)};
          result.Characters.emplace((char)glyph.getCodepoint(), ch);
        }

        msdfgen::destroyFont(font);
      }
      msdfgen::deinitializeFreetype(ft);
    }
    return result;
  }

  double PtSizeToPixels(float ptSize, int dpi)
  {
    return (ptSize * dpi) / 72.0;
  }
}

namespace Krys::Gfx::OpenGL
{
  FontRegistry::FontRegistry(IContext &context) noexcept
      : _context(static_cast<Context &>(context)),
        _dpi(Platform::GetDPIForWindow(Platform::GetActiveWindow()))
  {
  }

  FontRegistry::~FontRegistry() noexcept
  {
  }

  void FontRegistry::Startup()
  {
    IO::Path defaultFontPath("data/assets/fonts/Antonio-Bold.ttf");
    _defaultFontFamily = Register(_context.Strings().Intern("Antonio"), defaultFontPath);
    if (!_defaultFontFamily.IsValid())
    {
      KRYS_ERROR("Failed to register default font '{}'", defaultFontPath.ToString());
      KRYS_DEBUG_BREAK();
    }
  }

  void FontRegistry::Shutdown() noexcept
  {
    // TODO: Unload all fonts and font families
  }

  FontFamilyHandle FontRegistry::Register(StringRef name, const IO::Path &path) noexcept
  {
    FontFamily fontFamily {path};
    FontFamilyHandle handle = _fontFamilies.Add(std::move(fontFamily));
    return handle;
  }

  FontHandle FontRegistry::Get(const FontDesc &desc) noexcept
  {
    double sizeInPixels = PtSizeToPixels(desc.Size, _dpi);

    FontHandle handle;
    if (desc.Type == FontType::Bitmap)
    {
      FontDesc key = desc;
      if (auto cached = _cache.Get(key); cached.IsValid())
      {
        KRYS_DEBUG("Font cache hit.");
        return cached;
      }

      FontFamily &fontFamily = _fontFamilies.Get(desc.Family);
      BitmapLoadResult result = LoadBitmapAtlas(fontFamily.Path(), sizeInPixels);
      if (!result.Success)
      {
        KRYS_ERROR("Failed to load font '{}'", fontFamily.Path().ToString());
        return {};
      }

      List<uint8> atlas = CreateBitmapAtlasPixels(result.Glyphs, result.AtlasSize);
      auto characters = CreateBitmapCharacters(result.Glyphs, result.Padding, result.AtlasSize);

      Font font {desc.Type, desc.Size, fontFamily.Path(), GL_RED};
      font.SetAtlasData(atlas, result.AtlasSize, characters);
      handle = _fonts.Add(std::move(font));
      _cache.Add(key, handle);
    }
    else if (desc.Type == FontType::SDF || desc.Type == FontType::MSDF || desc.Type == FontType::MTSDF)
    {
      FontDesc key = {desc.Family, desc.Type, 0.f}; // size-independent cache key
      if (auto cached = _cache.Get(key); cached.IsValid())
      {
        KRYS_DEBUG("Font cache hit.");
        return cached;
      }

      FontFamily &fontFamily = _fontFamilies.Get(desc.Family);
      SDFParams params {};
      MTSDFResult result = LoadMTSDFAtlas(fontFamily.Path(), desc.Type, params);

      if (!result.Success)
      {
        KRYS_ERROR("Failed to load font '{}'", fontFamily.Path().ToString());
        return {};
      }

      Font font {desc.Type,
                 desc.Size,
                 fontFamily.Path(),
                 {result.Texture, result.Characters, result.AtlasSize},
                 params};
      handle = _fonts.Add(std::move(font));
      _cache.Add(key, handle);
    }

    return handle;
  }

  bool FontRegistry::Unload(FontHandle handle) noexcept
  {
    if (_cache.Remove(handle))
    {
      return _fonts.Remove(handle);
    }

    return false;
  }

  bool FontRegistry::Unload(FontFamilyHandle handle) noexcept
  {
    // TODO: Unload all fonts associated with this family
    return _fontFamilies.Remove(handle);
  }

  Font &FontRegistry::Get(FontHandle handle)
  {
    return _fonts.Get(handle);
  }

  Font *FontRegistry::TryGet(FontHandle handle) noexcept
  {
    return _fonts.TryGet(handle);
  }

  void FontRegistry::DPIChanged(int dpi) noexcept
  {
    if (dpi == _dpi)
    {
      return; // no change
    }

    _dpi = dpi;

    // We want to preserve the validity of existing FontHandles, so we need to reload
    // all bitmap fonts at the new DPI in-place.
    for (auto &[key, resource] : _cache)
    {
      FontHandle handle = resource.Handle;
      Font &font = _fonts.Get(handle);

      if (font.Type() != FontType::Bitmap)
      {
        continue; // the rest of the font types are resolution-independent
      }

      auto sizeInPixels = PtSizeToPixels(font.PtSize(), _dpi);
      BitmapLoadResult result = LoadBitmapAtlas(font.Path(), sizeInPixels);
      if (!result.Success)
      {
        KRYS_ERROR("Failed to load font '{}'", font.Path().ToString());
        KRYS_DEBUG_BREAK();
        continue;
      }

      List<uint8> atlas = CreateBitmapAtlasPixels(result.Glyphs, result.AtlasSize);
      auto characters = CreateBitmapCharacters(result.Glyphs, result.Padding, result.AtlasSize);
      font.SetAtlasData(atlas, result.AtlasSize, characters);
    }
  }

  FontFamilyHandle FontRegistry::GetDefaultFontFamily() noexcept
  {
    return _defaultFontFamily;
  }
}