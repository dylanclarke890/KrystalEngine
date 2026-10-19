#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/Array.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Gfx/Device.hpp"
#include "Krystal.Gfx/Handles.hpp"
#include "Krystal.Gfx/Render2D/AtlasPages.hpp"
#include "Krystal.Gfx/Render2D/FontFace2D.hpp"
#include "Krystal.Gfx/Render2D/Rect2D.hpp"
#include "Krystal.Gfx/Render2D/Transform2D.hpp"
#include <utility>

namespace krys::gfx
{
  struct GlyphAtlasDesc
  {
    /// @brief The width and height of each page, in texels.
    uint32 PageSize {1'024u};
    /// @brief The most coverage (R8) pages.
    uint32 MaxPages {4u};
    /// @brief The most colour (RGBA) pages.
    uint32 MaxColourPages {2u};
  };

  /// @brief Where a rasterised glyph is, and where it goes.
  struct GlyphLocation
  {
    /// @brief The R8 or RGBA texture holding it; invalid for a glyph with no pixels, such as a space.
    TextureHandle Texture {};
    PixelRect2D Texels {};
    /// @brief The pixels it covers relative to the pixel the pen is in, y down (see `GlyphBitmap`).
    PixelRect2D Bounds {};
    bool Colour {false};
  };

  struct GlyphAtlasStats
  {
    /// @brief Glyphs rasterised because they weren't cached.
    uint32 Rasterised {0u};
    /// @brief Glyphs found in the cache.
    uint32 Hits {0u};
    /// @brief Pages cleared to make room.
    uint32 PageEvictions {0u};
  };

  /// @brief Rasterises glyphs and caches them, coverage on R8 pages and colour glyphs on RGBA pages, separate
  /// from the mask atlas (glyphs are small, reused every frame and long-lived; masks come and go).
  ///
  /// A glyph is cached under its face, its index, its size, its pen's position within a pixel (a quarter of a
  /// pixel horizontally, as Skia positions glyphs; vertically, pens go on whole pixels) and the transform's
  /// linear part. Pages reclaim space a page at a time (`AtlasPages`). Glyphs larger than a page aren't
  /// cached: they're drawn as paths.
  class GlyphAtlas
  {
    KRYS_NON_COPY_MOVABLE(GlyphAtlas);

    constexpr static uint32 NoPage = ~0u;

    struct Key
    {
      uint64 Face;
      uint32 Glyph;
      float Size;
      uint32 Subpixel;
      Array<float, 4> Linear;

      KRYS_NODISCARD bool operator==(const Key &) const noexcept = default;
    };

    struct KeyHash
    {
      KRYS_NODISCARD size_t operator()(const Key &key) const noexcept;
    };

    struct Entry
    {
      bool Colour {false};
      /// @brief Its page, or `NoPage` for a glyph with no pixels.
      uint32 Page {NoPage};
      PixelRect2D Texels {};
      PixelRect2D Bounds {};
    };

  public:
    /// @brief How many positions within a pixel glyphs are rasterised at, horizontally.
    constexpr static uint32 SubpixelPositions = 4u;

  private:
    AtlasPages _coverage;
    AtlasPages _colour;
    HashMap<Key, Entry, KeyHash> _entries {};
    GlyphAtlasStats _stats {};

  public:
    GlyphAtlas(Device &device, const GlyphAtlasDesc &desc) noexcept;

    /// @brief The glyph rasterised at `size` pixels per em, its pen `subpixel` quarters of a pixel right of a
    /// pixel's left edge, under `linear` (its translation is ignored); rasterised if it isn't cached. Nothing
    /// if FreeType can't load it, or there's no room for it: draw it as a path instead.
    KRYS_NODISCARD Maybe<GlyphLocation> GetGlyph(const FontFace2D &face, uint32 glyph, float size,
                                                 uint32 subpixel, const Transform2D &linear = {}) noexcept;

    /// @brief Splits a pen's x into the whole pixel it's in and its quarter within it, rounding to the
    /// nearest quarter (a pen that rounds up to the next pixel is in that pixel, at quarter 0).
    KRYS_NODISCARD static std::pair<int32, uint32> SnapPen(float x) noexcept;

    KRYS_NODISCARD const GlyphAtlasStats &Stats() const noexcept
    {
      return _stats;
    }

    void ResetStats() noexcept
    {
      _stats = {};
    }

    KRYS_NODISCARD uint32 PageCount() const noexcept
    {
      return _coverage.PageCount() + _colour.PageCount();
    }
  };
}
