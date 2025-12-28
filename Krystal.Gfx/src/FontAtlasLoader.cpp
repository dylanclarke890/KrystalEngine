#include "Krystal.Gfx/FontAtlasLoader.hpp"
#include "Krystal.Lib/Core/DebugBreak.hpp"
#include "Krystal.Lib/Types/List.hpp"
#include "Krystal.Log/ILogger.hpp"
#include "Krystal.Maths/Round.hpp"
#include <algorithm>
#include <ft2build.h>
#include <stb_rect_pack.h>
#include FT_FREETYPE_H

#include <ext/import-font.h>
#include <msdf-atlas-gen.h>
#include <msdfgen-ext.h>
#include <msdfgen.h>

namespace
{
  using namespace Krys;
  using namespace Krys::Gfx;

  struct BitmapGlyph
  {
    UnicodeCodepoint Char {};
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
    bool _loaded = true;

  public:
    FreeTypeBitmapLoader(const IO::Path &path)
    {
      if (FT_Init_FreeType(&_library) != 0)
      {
        KRYS_ERROR("FREETYPE: Could not init FreeType Library");
        KRYS_DEBUG_BREAK();
        _loaded = false;
        return;
      }

      KRYS_INFO("FREETYPE: Loading bitmap font from '{}'", path.ToString());
      if (FT_New_Face(_library, path.ToString().c_str(), 0, &_face) != 0)
      {
        FT_Done_FreeType(_library);
        _library = nullptr;
        KRYS_ERROR("FREETYPE: Failed to load font '{}'", path.ToString());
        KRYS_DEBUG_BREAK();
        _loaded = false;
        return;
      }

      KRYS_INFO("FREETYPE: Loaded font: {}.", path.ToString());
      KRYS_INFO("FREETYPE:   Family: {}", _face->family_name);
      KRYS_INFO("FREETYPE:   Style: {}", _face->style_name);
      KRYS_INFO("FREETYPE:   {} face(s) in font", _face->num_faces);
    }

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

    KRYS_NODISCARD bool IsLoaded() const noexcept
    {
      return _loaded;
    }

    KRYS_NODISCARD bool SetPixelSize(uint32 height) const noexcept
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

    KRYS_NODISCARD List<BitmapGlyph> LoadGlyphs(uint8 paddingPerGlyph) const noexcept
    {
      List<BitmapGlyph> glyphs;

      FT_UInt gindex;
      uint32 charcode = FT_Get_First_Char(_face, &gindex);
      while (gindex != 0)
      {
        if (FT_Load_Char(_face, charcode, FT_LOAD_RENDER))
        {
          KRYS_WARN("FREETYPE: Failed to load glyph {}", charcode);
          continue;
        }

        if (_face->glyph->bitmap.pixel_mode != FT_PIXEL_MODE_GRAY)
        {
          KRYS_WARN("FREETYPE: Glyph {} not grayscale; skipping", charcode);
          continue;
        }

        const FT_GlyphSlot slot = _face->glyph;
        const FT_Bitmap &bm = slot->bitmap;
        BitmapGlyph glyph {
          .Char = UnicodeCodepoint(charcode),
          .ActualSize = {bm.width, bm.rows},
          .PaddedSize = glyph.ActualSize + (paddingPerGlyph * 2u),
          .Bearing = {slot->bitmap_left, slot->bitmap_top},
          .Advance = (int32)(slot->advance.x >> 6),
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

        charcode = FT_Get_Next_Char(_face, charcode, &gindex);
      }

      return glyphs;
    }

    KRYS_NODISCARD FontMetrics GetMetrics() const noexcept
    {
      auto ascender = static_cast<float>(_face->size->metrics.ascender >> 6);
      auto descender = static_cast<float>(_face->size->metrics.descender >> 6);
      auto height = static_cast<float>(_face->size->metrics.height >> 6); // typographic
      auto lineHeight = ascender - descender;                             // pixel-tight

      return {.Ascender = ascender, .Descender = descender, .Height = height, .LineHeight = lineHeight};
    }

    KRYS_NODISCARD CharacterMap ToCodepointsMap(List<BitmapGlyph> &glyphs, int padding,
                                            const Maths::Vec2u &atlasSize)
    {
      CharacterMap characters;
      characters.reserve(glyphs.size());

      for (const auto &glyph : glyphs)
      {
        const float u0 = float(glyph.Rect.x + padding) / float(atlasSize.x);
        const float v0 = float(glyph.Rect.y + padding) / float(atlasSize.y);

        const float u1 = float(glyph.Rect.x + padding + glyph.ActualSize.x) / float(atlasSize.x);
        const float v1 = float(glyph.Rect.y + padding + glyph.ActualSize.y) / float(atlasSize.y);

        characters[glyph.Char] = Character {
          .Size = glyph.ActualSize,
          .Bearing = glyph.Bearing,
          .Advance = glyph.Advance,
          .UVMin = {u0, v0},
          .UVMax = {u1, v1},
        };
      }

      return characters;
    }

    KRYS_NODISCARD List<uint8> ToPixels(const List<BitmapGlyph> &glyphs, const Maths::Vec2u &atlasSize)
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

    KRYS_NODISCARD bool TryPackGlyphs(List<BitmapGlyph> &glyphs, Maths::Vec2u &size)
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
  };

  class msdfLoader
  {
    FT_Library _library = nullptr;
    FT_Face _face = nullptr;
    msdfgen::FreetypeHandle *_ft = nullptr;
    msdfgen::FontHandle *_font = nullptr;
    const SDFParams &_params;
    bool _loaded = true;

  public:
    msdfLoader(const IO::Path &path, const SDFParams &params) : _params(params)
    {
      if (FT_Init_FreeType(&_library) != 0)
      {
        KRYS_ERROR("FREETYPE: Could not init FreeType Library");
        KRYS_DEBUG_BREAK();
        _loaded = false;
        return;
      }

      KRYS_INFO("FREETYPE: Loading sdf-based font from '{}'", path.ToString());
      if (FT_New_Face(_library, path.ToString().c_str(), 0, &_face) != 0)
      {
        FT_Done_FreeType(_library);
        _library = nullptr;
        KRYS_ERROR("FREETYPE: Failed to load font '{}'", path.ToString());
        KRYS_DEBUG_BREAK();
        _loaded = false;
        return;
      }

      KRYS_INFO("MSDFGEN: Loading font from '{}'", path.ToString());
      _font = msdfgen::adoptFreetypeFont(_face);
      if (_font == nullptr)
      {
        msdfgen::deinitializeFreetype(_ft);
        _ft = nullptr;
        KRYS_ERROR("MSDFGEN: Failed to load font '{}'", path.ToString());
        KRYS_DEBUG_BREAK();
        _loaded = false;
        return;
      }

      KRYS_INFO("MSDFGEN: Loaded font: {}.", path.ToString());
    }

    ~msdfLoader()
    {
      if (_font != nullptr)
      {
        msdfgen::destroyFont(_font);
      }

      FT_Done_Face(_face);

      if (_ft != nullptr)
      {
        msdfgen::deinitializeFreetype(_ft);
      }
    }

    bool IsLoaded() const noexcept
    {
      return _loaded;
    }

    List<msdf_atlas::GlyphGeometry> LoadGlyphs()
    {
      List<msdf_atlas::GlyphGeometry> glyphs;
      msdf_atlas::FontGeometry fontGeometry(&glyphs);
      msdf_atlas::Charset charset;

      FT_UInt gindex;
      uint32 charcode = FT_Get_First_Char(_face, &gindex);
      while (gindex != 0)
      {
        charset.add(charcode);
        charcode = FT_Get_Next_Char(_face, charcode, &gindex);
      }

      fontGeometry.loadCharset(_font, 1.0, charset, true, true);

      const double maxCornerAngle = 3.0; // Apply MSDF edge coloring. edge-coloring.h for other strategies.
      for (msdf_atlas::GlyphGeometry &glyph : glyphs)
      {
        glyph.edgeColoring(&msdfgen::edgeColoringInkTrap, maxCornerAngle, 0);
      }

      return glyphs;
    }

    FontMetrics GetMetrics() const noexcept
    {
      msdfgen::FontMetrics metrics;
      msdfgen::getFontMetrics(metrics, _font, msdfgen::FONT_SCALING_EM_NORMALIZED);

      float scale = _params.EMSizeInPixels;
      auto ascender = static_cast<float>(metrics.ascenderY * scale);
      auto descender = static_cast<float>(metrics.descenderY * scale);
      auto height = static_cast<float>(metrics.lineHeight * scale);
      auto lineHeight = static_cast<float>((metrics.ascenderY - metrics.descenderY) * scale);

      return {.Ascender = ascender, .Descender = descender, .Height = height, .LineHeight = lineHeight};
    }

    Maths::Vec2u PackAtlas(List<msdf_atlas::GlyphGeometry> &glyphs)
    {
      msdf_atlas::TightAtlasPacker packer;
      packer.setDimensionsConstraint(msdf_atlas::DimensionsConstraint::SQUARE);
      packer.setScale(_params.EMSizeInPixels);
      packer.setPixelRange(_params.PixelRange);
      packer.setMiterLimit(_params.MiterLimit);
      packer.pack(glyphs.data(), static_cast<int>(glyphs.size()));
      int width = 0, height = 0;
      packer.getDimensions(width, height);
      return {static_cast<uint32>(width), static_cast<uint32>(height)};
    }

    CharacterMap ToCodepointsMap(Krys::List<msdf_atlas::GlyphGeometry> &glyphs, const Maths::Vec2 &atlasSize)
    {
      CharacterMap characters;
      characters.reserve(glyphs.size());

      for (const msdf_atlas::GlyphGeometry &glyph : glyphs)
      {
        double b, t, l, r;
        glyph.getQuadAtlasBounds(l, b, r, t);

        double pl, pb, pr, pt;
        glyph.getQuadPlaneBounds(pl, pb, pr, pt);

        double scale = _params.EMSizeInPixels;

        using namespace Krys::Maths;
        characters[UnicodeCodepoint(glyph.getCodepoint())] = Character {
          .Size = Vec2u(Round(Vec2d(pr - pl, pt - pb) * scale)),
          .Bearing = Vec2i(Round(Vec2d(pl, pt) * scale)),
          .Advance = static_cast<int32>(Round(glyph.getAdvance() * scale)),
          .UVMin = Vec2((float)l, (float)t) / atlasSize,
          .UVMax = Vec2((float)r, (float)b) / atlasSize,
        };
      }

      return characters;
    }
  };
}

namespace Krys::Gfx
{
  Expected<FontAtlasData> FontAtlasLoader::LoadBitmap(const IO::Path &path, uint32 fontSizeInPixels,
                                                      uint8 paddingPerGlyph) noexcept
  {
    FreeTypeBitmapLoader loader {path};
    if (!loader.IsLoaded())
    {
      return Unexpected("Failed to load font");
    }

    if (!loader.SetPixelSize(fontSizeInPixels))
    {
      return Unexpected("Failed to set font size");
    }

    List<BitmapGlyph> glyphs = loader.LoadGlyphs(paddingPerGlyph);
    if (glyphs.empty())
    {
      return Unexpected("No glyphs loaded");
    }

    auto atlasSize = Maths::Vec2u {512u, 512u}; // Will be resized as needed.
    if (!loader.TryPackGlyphs(glyphs, atlasSize))
    {
      return Unexpected("Unable to pack glyphs");
    }

    FontAtlasData result {};
    result.Size = atlasSize;
    result.Pixels = loader.ToPixels(glyphs, atlasSize);
    result.Characters = loader.ToCodepointsMap(glyphs, paddingPerGlyph, atlasSize);
    result.Metrics = loader.GetMetrics();
    return result;
  }

  Expected<FontAtlasData> FontAtlasLoader::LoadSDF(const IO::Path &path, const SDFParams &params) noexcept
  {
    using namespace msdf_atlas;
    using SDFAtlasData = const msdfgen::BitmapConstRef<msdfgen::byte, 1> &;
    using SDFGenerator =
      ImmediateAtlasGenerator<float, 1, sdfGenerator, BitmapAtlasStorage<msdfgen::byte, 1>>;

    msdfLoader loader {path, params};
    if (!loader.IsLoaded())
    {
      return Unexpected("Failed to load font");
    }

    auto glyphs = loader.LoadGlyphs();
    auto packedAtlas = loader.PackAtlas(glyphs);

    SDFGenerator generator(packedAtlas.x, packedAtlas.y);
    generator.setAttributes({.scanlinePass = true});
    generator.generate(glyphs.data(), (int)glyphs.size());
    SDFAtlasData atlas = generator.atlasStorage();

    size_t pixelCount = static_cast<size_t>(atlas.width) * atlas.height * 1;
    FontAtlasData result {};
    result.Size = {(uint32)atlas.width, (uint32)atlas.height};
    result.Pixels = List<uint8>(atlas.pixels, atlas.pixels + pixelCount);
    result.Characters = loader.ToCodepointsMap(glyphs, result.Size);
    result.Metrics = loader.GetMetrics();

    return result;
  }

  Expected<FontAtlasData> FontAtlasLoader::LoadMSDF(const IO::Path &path, const SDFParams &params) noexcept
  {
    using namespace msdf_atlas;
    using MSDFAtlasData = const msdfgen::BitmapConstRef<msdfgen::byte, 3> &;
    using MSDFGenerator =
      ImmediateAtlasGenerator<float, 3, msdfGenerator, BitmapAtlasStorage<msdfgen::byte, 3>>;

    msdfLoader loader {path, params};
    if (!loader.IsLoaded())
    {
      return Unexpected("Failed to load font");
    }
    auto glyphs = loader.LoadGlyphs();
    auto packedAtlas = loader.PackAtlas(glyphs);

    MSDFGenerator generator(packedAtlas.x, packedAtlas.y);
    generator.setAttributes({.scanlinePass = true});
    generator.generate(glyphs.data(), (int)glyphs.size());
    MSDFAtlasData atlas = generator.atlasStorage();

    size_t pixelCount = static_cast<size_t>(atlas.width) * atlas.height * 3;
    FontAtlasData result {};
    result.Pixels = List<uint8>(atlas.pixels, atlas.pixels + pixelCount);
    result.Size = {(uint32)atlas.width, (uint32)atlas.height};
    result.Characters = loader.ToCodepointsMap(glyphs, result.Size);
    result.Metrics = loader.GetMetrics();

    return result;
  }

  Expected<FontAtlasData> FontAtlasLoader::LoadMTSDF(const IO::Path &path, const SDFParams &params) noexcept
  {
    using namespace msdf_atlas;
    using MTSDFAtlasData = const msdfgen::BitmapConstRef<msdfgen::byte, 4> &;
    using MTSDFGenerator =
      ImmediateAtlasGenerator<float, 4, mtsdfGenerator, BitmapAtlasStorage<msdfgen::byte, 4>>;

    msdfLoader loader {path, params};
    if (!loader.IsLoaded())
    {
      return Unexpected("Failed to load font");
    }
    auto glyphs = loader.LoadGlyphs();
    auto packedAtlas = loader.PackAtlas(glyphs);

    MTSDFGenerator generator(packedAtlas.x, packedAtlas.y);
    generator.setAttributes({.scanlinePass = true});
    generator.generate(glyphs.data(), (int)glyphs.size());
    MTSDFAtlasData atlas = generator.atlasStorage();

    size_t pixelCount = static_cast<size_t>(atlas.width) * atlas.height * 4;
    FontAtlasData result {};
    result.Pixels = List<uint8>(atlas.pixels, atlas.pixels + pixelCount);
    result.Size = {(uint32)atlas.width, (uint32)atlas.height};
    result.Characters = loader.ToCodepointsMap(glyphs, result.Size);
    result.Metrics = loader.GetMetrics();

    return result;
  }
}