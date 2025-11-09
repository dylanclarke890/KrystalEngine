#pragma once

#include "Krystal.Gfx.OpenGL/Registries/FontRegistry.hpp"
#include "Krystal.Gfx.OpenGL/Context.hpp"
#include "Krystal.Gfx/Handle.hpp"
#include "Krystal.Gfx/Resources/Font.hpp"
#include "Krystal.Lib/DebugBreak.hpp"
#include "Krystal.Lib/Map.hpp"
#include "Krystal.Log/ILogger.hpp"
#include "Krystal.Platform/Platform.hpp"
#include <ft2build.h>
#include <msdf-atlas-gen.h>
#include <msdfgen-ext.h>
#include <msdfgen.h>
#include FT_FREETYPE_H
#include <algorithm>

namespace
{
  using namespace Krys;
  using namespace Krys::Maths;
  using namespace Krys::Gfx;
  using namespace Krys::Gfx::OpenGL;

  struct MTSDFResult
  {
    GLuint Texture {0u};
    Map<uchar, Character> Characters {};
    Vec2u AtlasSize {};
    bool Success {false};
  };

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

  uint32 PtSizeToPixels(float ptSize, int dpi)
  {
    auto pixelSize = (ptSize * dpi) / 72.0;
    return static_cast<uint32>(pixelSize);
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
    const IO::Path defaultFontPath("data/assets/fonts/Antonio-Bold.ttf");
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
    FontFamily fontFamily {name, path};
    FontFamilyHandle handle = _fontFamilies.Add(std::move(fontFamily));
    return handle;
  }

  FontHandle FontRegistry::Get(const FontDesc &desc) noexcept
  {
    assert(desc.Family.IsValid() && "Invalid font family handle.");
    FontFamily &fontFamily = _fontFamilies.Get(desc.Family);

    if (desc.Type == FontType::Bitmap)
    {
      if (FontHandle cached = _cache.Get(desc); cached.IsValid())
      {
        KRYS_DEBUG("Font cache hit.");
        return cached;
      }

      auto expected = _loader.LoadBitmap(fontFamily.Path(), PtSizeToPixels(desc.Size, _dpi));
      if (!expected.has_value())
      {
        KRYS_ERROR("Failed to load font '{}'", _fontFamilies.Get(desc.Family).Path().ToString());
        return {};
      }

      Font font {desc.Type, desc.Size, desc.Family, GL_RED};
      font.SetAtlasData(expected.value());

      FontHandle handle = _fonts.Add(std::move(font));
      _cache.Add(desc, handle);
      return handle;
    }
    else if (desc.Type == FontType::SDF || desc.Type == FontType::MSDF || desc.Type == FontType::MTSDF)
    {
      FontDesc key = {desc.Family, desc.Type, 0.f}; // size-independent cache key
      if (FontHandle cached = _cache.Get(key); cached.IsValid())
      {
        KRYS_DEBUG("Font cache hit.");
        return cached;
      }

      SDFParams params {};
      MTSDFResult result = LoadMTSDFAtlas(fontFamily.Path(), desc.Type, params);

      if (!result.Success)
      {
        KRYS_ERROR("Failed to load font '{}'", fontFamily.Path().ToString());
        return {};
      }

      Font font {
        desc.Type, desc.Size, desc.Family, {result.Texture, result.Characters, result.AtlasSize}, params};

      FontHandle handle = _fonts.Add(std::move(font));
      _cache.Add(key, handle);
      return handle;
    }

    std::unreachable();
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

      auto &fontFamily = _fontFamilies.Get(font.FontFamily());
      auto expected = _loader.LoadBitmap(fontFamily.Path(), PtSizeToPixels(font.PtSize(), _dpi));
      if (!expected.has_value())
      {
        KRYS_ERROR("Failed to load font '{}'", fontFamily.Path().ToString());
        KRYS_DEBUG_BREAK();
        continue;
      }

      font.SetAtlasData(expected.value());
    }
  }

  FontFamilyHandle FontRegistry::GetDefaultFontFamily() noexcept
  {
    return _defaultFontFamily;
  }
}