#include "Krystal.Gfx/Render2D/FontFace2D.hpp"
#include "Krystal.Core/Utils/Move.hpp"
#include <atomic>
#include <cmath>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_ADVANCES_H
#include FT_OUTLINE_H
#include FT_SIZES_H

namespace krys::gfx
{
  namespace
  {
    /// @brief How far synthetic oblique slants glyphs: x moves by this much of y.
    constexpr float ObliqueSlant = 0.25f;

    KRYS_NODISCARD FT_Fixed ToFixed(float value) noexcept
    {
      return static_cast<FT_Fixed>(std::lround(static_cast<double>(value) * 65'536.0));
    }

    /// @brief Thickens an outline as synthetic bold does: by a 24th of the em, as Skia does.
    void Embolden(FT_Outline &outline, FT_Pos emSize) noexcept
    {
      FT_Outline_Embolden(&outline, emSize / 24);
    }

    /// @brief Builds a `Path2D` from a FreeType outline in font units, scaled to pixels and slanted.
    struct OutlineBuilder
    {
      Path2D Path {};
      float Scale {1.f};
      float Slant {0.f};
      Vec2f Origin {};
      bool Open {false};

      KRYS_NODISCARD Vec2f Map(const FT_Vector &point) const noexcept
      {
        const float x = static_cast<float>(point.x);
        const float y = static_cast<float>(point.y);
        return {Origin.x + (x + Slant * y) * Scale, Origin.y - y * Scale};
      }
    };
  }

  Expected<UniquePtr<FontFace2D>> FontFace2D::Create(Ref<text::FreeTypeFace> face,
                                                     const FontFace2DOptions &options) noexcept
  {
    FT_Size size = nullptr;
    if (FT_New_Size(face->Face(), &size))
    {
      return Unexpected<string>("FreeType couldn't make a size for the face.");
    }
    return CreateUnique<FontFace2D>(krys::move(face), options, size);
  }

  FontFace2D::FontFace2D(Ref<text::FreeTypeFace> &&face, const FontFace2DOptions &options,
                         FT_Size size) noexcept
      : _face(krys::move(face)), _options(options), _size(size)
  {
    static std::atomic<uint64> nextId {1u};
    _id = nextId.fetch_add(1u, std::memory_order_relaxed);
  }

  FontFace2D::~FontFace2D() noexcept
  {
    FT_Done_Size(_size);
  }

  bool FontFace2D::HasColour() const noexcept
  {
    return FT_HAS_COLOR(_face->Face());
  }

  uint32 FontFace2D::GlyphIndex(char32 codePoint) const noexcept
  {
    return _face->GlyphIndex(codePoint);
  }

  float FontFace2D::Advance(uint32 glyph, float size) const noexcept
  {
    FT_Fixed advance = 0;
    if (FT_Get_Advance(_face->Face(), glyph, FT_LOAD_NO_SCALE, &advance))
    {
      return 0.f;
    }
    return static_cast<float>(advance) * size / static_cast<float>(_face->UnitsPerEm());
  }

  bool FontFace2D::UseSize(float size) const noexcept
  {
    FT_Face face = _face->Face();
    if (FT_Activate_Size(_size))
    {
      return false;
    }
    if (size != _sizeInPixels)
    {
      // At 72 dots per inch, points are pixels.
      const auto nominal = static_cast<FT_F26Dot6>(std::lround(static_cast<double>(size) * 64.0));
      if (FT_Set_Char_Size(face, 0, nominal, 72u, 72u))
      {
        _sizeInPixels = 0.f;
        return false;
      }
      _sizeInPixels = size;
    }
    return true;
  }

  Maybe<GlyphBitmap> FontFace2D::Rasterise(uint32 glyph, float size, float subpixel,
                                           const Transform2D &linear) const noexcept
  {
    if (!(size > 0.f) || !std::isfinite(size) || !std::isfinite(subpixel) || !UseSize(size))
    {
      return {};
    }
    FT_Face face = _face->Face();

    // FreeType's y points up: the transform is flipped into it, and the slant applied before it.
    const bool transformed = linear.A != 1.f || linear.B != 0.f || linear.C != 0.f || linear.D != 1.f;
    const float slant = _options.SyntheticOblique ? ObliqueSlant : 0.f;
    FT_Matrix matrix {.xx = ToFixed(linear.A),
                      .xy = ToFixed(-linear.C + linear.A * slant),
                      .yx = ToFixed(-linear.B),
                      .yy = ToFixed(linear.D - linear.B * slant)};
    FT_Vector delta {.x = static_cast<FT_Pos>(std::lround(static_cast<double>(subpixel) * 64.0)), .y = 0};
    FT_Set_Transform(face, transformed || slant != 0.f ? &matrix : nullptr, &delta);

    FT_Int32 flags = FT_LOAD_DEFAULT;
    if (transformed || slant != 0.f || _options.Hinting == FontHinting::None)
    {
      flags |= FT_LOAD_NO_HINTING;
    }
    else if (_options.Hinting == FontHinting::Slight)
    {
      flags |= FT_LOAD_TARGET_LIGHT;
    }
    flags |= FT_HAS_COLOR(face) ? FT_LOAD_COLOR : FT_LOAD_NO_BITMAP;

    const bool loaded = FT_Load_Glyph(face, glyph, flags) == 0;
    // The face is shared: leave no transform on it.
    FT_Set_Transform(face, nullptr, nullptr);
    if (!loaded)
    {
      return {};
    }

    FT_GlyphSlot slot = face->glyph;
    if (_options.SyntheticBold && slot->format == FT_GLYPH_FORMAT_OUTLINE)
    {
      Embolden(slot->outline, FT_MulFix(face->units_per_EM, face->size->metrics.y_scale));
    }
    if (slot->format != FT_GLYPH_FORMAT_BITMAP && FT_Render_Glyph(slot, FT_RENDER_MODE_NORMAL))
    {
      return {};
    }

    const FT_Bitmap &bitmap = slot->bitmap;
    GlyphBitmap result {.Bounds = {.X = slot->bitmap_left,
                                   .Y = -slot->bitmap_top,
                                   .Width = bitmap.width,
                                   .Height = bitmap.rows},
                        .Pixels = {},
                        .Colour = bitmap.pixel_mode == FT_PIXEL_MODE_BGRA};
    if (result.Bounds.IsEmpty())
    {
      return result;
    }

    const uint32 bytesPerPixel = result.Colour ? 4u : 1u;
    result.Pixels.resize(static_cast<size_t>(bitmap.width) * bitmap.rows * bytesPerPixel);
    for (uint32 row = 0u; row < bitmap.rows; ++row)
    {
      const uint8 *source = bitmap.buffer + static_cast<ptrdiff_t>(row) * bitmap.pitch;
      uint8 *destination = result.Pixels.data() + static_cast<size_t>(row) * bitmap.width * bytesPerPixel;
      for (uint32 column = 0u; column < bitmap.width; ++column)
      {
        switch (bitmap.pixel_mode)
        {
          case FT_PIXEL_MODE_GRAY:
            destination[column] = static_cast<uint8>(source[column] * 255u / (bitmap.num_grays - 1u));
            break;
          case FT_PIXEL_MODE_MONO:
            destination[column] = (source[column / 8u] & (0x80u >> (column % 8u))) != 0u ? 255u : 0u;
            break;
          case FT_PIXEL_MODE_BGRA:
            // Premultiplied BGRA, swizzled to RGBA.
            destination[column * 4u + 0u] = source[column * 4u + 2u];
            destination[column * 4u + 1u] = source[column * 4u + 1u];
            destination[column * 4u + 2u] = source[column * 4u + 0u];
            destination[column * 4u + 3u] = source[column * 4u + 3u];
            break;
          default: return {};
        }
      }
    }
    return result;
  }

  Path2D FontFace2D::GlyphPath(uint32 glyph, float size, Vec2f origin) const noexcept
  {
    FT_Face face = _face->Face();
    if (!(size > 0.f) || !std::isfinite(size)
        || FT_Load_Glyph(face, glyph, FT_LOAD_NO_SCALE | FT_LOAD_NO_HINTING | FT_LOAD_NO_BITMAP)
        || face->glyph->format != FT_GLYPH_FORMAT_OUTLINE)
    {
      return {};
    }

    FT_Outline &outline = face->glyph->outline;
    if (_options.SyntheticBold)
    {
      // In font units, the em is the units per em.
      Embolden(outline, static_cast<FT_Pos>(face->units_per_EM));
    }

    OutlineBuilder builder {.Path = {},
                            .Scale = size / static_cast<float>(face->units_per_EM),
                            .Slant = _options.SyntheticOblique ? ObliqueSlant : 0.f,
                            .Origin = origin,
                            .Open = false};
    FT_Outline_Funcs functions {};
    functions.move_to = [](const FT_Vector *to, void *user)
    {
      auto &b = *static_cast<OutlineBuilder *>(user);
      if (b.Open)
      {
        b.Path.Close();
      }
      b.Path.MoveTo(b.Map(*to));
      b.Open = true;
      return 0;
    };
    functions.line_to = [](const FT_Vector *to, void *user)
    {
      auto &b = *static_cast<OutlineBuilder *>(user);
      b.Path.LineTo(b.Map(*to));
      return 0;
    };
    functions.conic_to = [](const FT_Vector *control, const FT_Vector *to, void *user)
    {
      auto &b = *static_cast<OutlineBuilder *>(user);
      b.Path.QuadTo(b.Map(*control), b.Map(*to));
      return 0;
    };
    functions.cubic_to =
      [](const FT_Vector *control1, const FT_Vector *control2, const FT_Vector *to, void *user)
    {
      auto &b = *static_cast<OutlineBuilder *>(user);
      b.Path.CubicTo(b.Map(*control1), b.Map(*control2), b.Map(*to));
      return 0;
    };
    if (FT_Outline_Decompose(&outline, &functions, &builder))
    {
      return {};
    }
    if (builder.Open)
    {
      builder.Path.Close();
    }
    return krys::move(builder.Path);
  }
}
