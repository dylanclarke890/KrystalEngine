#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/Expected.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"
#include "Krystal.Gfx/Render2D/Path2D.hpp"
#include "Krystal.Gfx/Render2D/Rect2D.hpp"
#include "Krystal.Gfx/Render2D/Transform2D.hpp"
#include "Krystal.Text/FreeTypeFace.hpp"

// FreeType's own declaration of a size handle.
typedef struct FT_SizeRec_ *FT_Size;

namespace krys::gfx
{
  /// @brief How far glyphs' outlines are fitted to the pixel grid.
  enum class FontHinting : uint8
  {
    /// @brief Not at all: glyphs keep their exact shapes.
    None,
    /// @brief Vertically only, so stems and baselines are sharp and widths keep their proportions; what
    /// fontconfig asks for by default, so what Linux browsers draw.
    Slight,
    /// @brief Both ways, as the font's own instructions say.
    Full,
  };

  struct FontFace2DOptions
  {
    FontHinting Hinting {FontHinting::Slight};
    /// @brief Thickens glyphs, for a bold the face doesn't have.
    bool SyntheticBold {false};
    /// @brief Slants glyphs by a quarter (about 14 degrees), for an italic the face doesn't have.
    bool SyntheticOblique {false};
  };

  /// @brief A glyph rasterised for drawing.
  struct GlyphBitmap
  {
    /// @brief The pixels it covers, relative to the pixel the pen is in: the pen is at that pixel's top-left
    /// corner plus the subpixel offset; y points down, so the baseline is y = 0.
    PixelRect2D Bounds {};
    /// @brief Rows from the top: one byte of coverage per pixel, or four (premultiplied RGBA) for colour
    /// glyphs.
    List<uint8> Pixels {};
    bool Colour {false};
  };

  /// @brief A face of a font as Render2D draws it: Krystal.Text's face with hinting and synthetic styles,
  /// rasterising glyphs with FreeType into 8-bit coverage (or RGBA, for colour glyphs).
  /// @attention Not thread-safe: it sets its own FreeType size on the shared face as it rasterises.
  class FontFace2D
  {
    KRYS_NON_COPY_MOVABLE(FontFace2D);

  private:
    Ref<text::FreeTypeFace> _face;
    FontFace2DOptions _options;
    uint64 _id;
    FT_Size _size {nullptr};
    /// @brief The pixels per em `_size` is set to, or 0.
    mutable float _sizeInPixels {0.f};

  public:
    KRYS_NODISCARD static Expected<UniquePtr<FontFace2D>>
      Create(Ref<text::FreeTypeFace> face, const FontFace2DOptions &options = {}) noexcept;

    /// @brief Use `Create`.
    FontFace2D(Ref<text::FreeTypeFace> &&face, const FontFace2DOptions &options, FT_Size size) noexcept;
    ~FontFace2D() noexcept;

    KRYS_NODISCARD const text::FreeTypeFace &Face() const noexcept
    {
      return *_face;
    }

    KRYS_NODISCARD const FontFace2DOptions &Options() const noexcept
    {
      return _options;
    }

    /// @brief Unique among the faces made in this process, so caches can't mistake a new face for an old one.
    KRYS_NODISCARD uint64 Id() const noexcept
    {
      return _id;
    }

    /// @brief The glyph of `codePoint`, or 0 if the face has none.
    KRYS_NODISCARD uint32 GlyphIndex(char32 codePoint) const noexcept;
    /// @brief How far the pen moves after `glyph` at `size` pixels per em, unhinted, as text is laid out.
    KRYS_NODISCARD float Advance(uint32 glyph, float size) const noexcept;

    /// @brief Rasterises `glyph` at `size` pixels per em, with the pen `subpixel` pixels (0 to 1) right of a
    /// pixel's left edge, under `linear` (its translation is ignored). Hinting applies only without a
    /// transform or synthetic oblique, as fitting to the grid only means anything on it. Nothing if FreeType
    /// can't load it.
    KRYS_NODISCARD Maybe<GlyphBitmap> Rasterise(uint32 glyph, float size, float subpixel,
                                                const Transform2D &linear = {}) const noexcept;

    /// @brief The outline of `glyph` at `size` pixels per em, the pen at the origin, y pointing down, with
    /// the synthetic styles; unhinted. Empty for a glyph without one.
    KRYS_NODISCARD Path2D GlyphPath(uint32 glyph, float size) const noexcept;

    /// @brief Whether the face has colour glyphs, which come only as bitmaps.
    KRYS_NODISCARD bool HasColour() const noexcept;

  private:
    /// @brief Makes `_size` the face's size, at `size` pixels per em.
    KRYS_NODISCARD bool UseSize(float size) const noexcept;
  };
}
