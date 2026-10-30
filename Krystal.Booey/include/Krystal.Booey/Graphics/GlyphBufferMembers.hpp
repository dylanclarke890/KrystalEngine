#pragma once

#include "Krystal.Booey/Graphics/FloatPoint.hpp"
#include "Krystal.Booey/Graphics/FloatSize.hpp"
#include "Krystal.Booey/Graphics/Glyph.hpp"

namespace krys::boo
{
  // When you write code that interacts with these types, use the functions below!
  using GlyphBufferGlyph = Glyph;
  using GlyphBufferAdvance = FloatSize;
  using GlyphBufferOrigin = FloatPoint;
  /// @brief A byte offset into the run's UTF-8 text.
  using GlyphBufferStringOffset = uint32;

  KRYS_NODISCARD constexpr GlyphBufferAdvance MakeGlyphBufferAdvance(const FloatSize &size) noexcept
  {
    return size;
  }

  KRYS_NODISCARD constexpr GlyphBufferAdvance MakeGlyphBufferAdvance(float width = 0,
                                                                     float height = 0) noexcept
  {
    return FloatSize(width, height);
  }

  KRYS_NODISCARD constexpr FloatSize Size(const GlyphBufferAdvance &advance) noexcept
  {
    return advance;
  }

  constexpr void SetWidth(GlyphBufferAdvance &advance, float width) noexcept
  {
    advance.SetWidth(width);
  }

  constexpr void SetHeight(GlyphBufferAdvance &advance, float height) noexcept
  {
    advance.SetHeight(height);
  }

  KRYS_NODISCARD constexpr float Width(const GlyphBufferAdvance &advance) noexcept
  {
    return advance.Width();
  }

  KRYS_NODISCARD constexpr float Height(const GlyphBufferAdvance &advance) noexcept
  {
    return advance.Height();
  }

  KRYS_NODISCARD constexpr GlyphBufferOrigin MakeGlyphBufferOrigin(const FloatPoint &point) noexcept
  {
    return point;
  }

  KRYS_NODISCARD constexpr GlyphBufferOrigin MakeGlyphBufferOrigin(float x = 0, float y = 0) noexcept
  {
    return FloatPoint(x, y);
  }

  KRYS_NODISCARD constexpr FloatPoint Point(const GlyphBufferOrigin &origin) noexcept
  {
    return origin;
  }

  constexpr void SetX(GlyphBufferOrigin &origin, float x) noexcept
  {
    origin.SetX(x);
  }

  constexpr void SetY(GlyphBufferOrigin &origin, float y) noexcept
  {
    origin.SetY(y);
  }

  KRYS_NODISCARD constexpr float X(const GlyphBufferOrigin &origin) noexcept
  {
    return origin.X();
  }

  KRYS_NODISCARD constexpr float Y(const GlyphBufferOrigin &origin) noexcept
  {
    return origin.Y();
  }
}
