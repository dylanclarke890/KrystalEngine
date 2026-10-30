#pragma once

#include "Krystal.Booey/Graphics/FontBaseline.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/Markable.hpp"
#include <algorithm>
#include <cmath>

namespace krys::boo
{
  class Font;

  /// @brief The vertical and reference metrics of a font, in pixels.
  class FontMetrics
  {
    friend class Font;

  public:
    constexpr static unsigned DefaultUnitsPerEm = 1'000;

  private:
    unsigned _unitsPerEm {DefaultUnitsPerEm};

    Markable<float> _ascent;
    Markable<float> _capHeight;
    Markable<float> _descent;
    Markable<float> _ideogramWidth;
    Markable<float> _lineGap;
    Markable<float> _lineSpacing;
    Markable<float> _underlinePosition;
    Markable<float> _underlineThickness;
    Markable<float> _xHeight;
    Markable<float> _zeroWidth;

    // Integer variants of certain metrics are cached for HTML rendering performance.
    int _intAscent {0};
    int _intDescent {0};
    int _intLineGap {0};
    int _intLineSpacing {0};
    int _intCapHeight {0};

  public:
    KRYS_NODISCARD unsigned UnitsPerEm() const noexcept
    {
      return _unitsPerEm;
    }
    void SetUnitsPerEm(unsigned unitsPerEm) noexcept
    {
      _unitsPerEm = unitsPerEm;
    }

    KRYS_NODISCARD float Height(FontBaseline baselineType = FontBaseline::Alphabetic) const noexcept
    {
      return Ascent(baselineType) + Descent(baselineType);
    }
    KRYS_NODISCARD int IntHeight(FontBaseline baselineType = FontBaseline::Alphabetic) const noexcept
    {
      return IntAscent(baselineType) + IntDescent(baselineType);
    }

    KRYS_NODISCARD float Ascent(FontBaseline baselineType = FontBaseline::Alphabetic) const noexcept
    {
      if (baselineType == FontBaseline::Alphabetic)
      {
        return _ascent.value_or(0.f);
      }
      return Height() / 2;
    }
    KRYS_NODISCARD int IntAscent(FontBaseline baselineType = FontBaseline::Alphabetic) const noexcept
    {
      if (baselineType == FontBaseline::Alphabetic)
      {
        return _intAscent;
      }
      return IntHeight() - IntHeight() / 2;
    }
    void SetAscent(float ascent) noexcept
    {
      _ascent = ascent;
      _intAscent = std::max(static_cast<int>(std::lroundf(ascent)), 0);
    }

    KRYS_NODISCARD float Descent(FontBaseline baselineType = FontBaseline::Alphabetic) const noexcept
    {
      if (baselineType == FontBaseline::Alphabetic)
      {
        return _descent.value_or(0.f);
      }
      return Height() / 2;
    }
    KRYS_NODISCARD int IntDescent(FontBaseline baselineType = FontBaseline::Alphabetic) const noexcept
    {
      if (baselineType == FontBaseline::Alphabetic)
      {
        return _intDescent;
      }
      return IntHeight() / 2;
    }
    void SetDescent(float descent) noexcept
    {
      _descent = descent;
      _intDescent = static_cast<int>(std::lroundf(descent));
    }

    KRYS_NODISCARD float LineGap() const noexcept
    {
      return _lineGap.value_or(0.f);
    }
    KRYS_NODISCARD int IntLineGap() const noexcept
    {
      return _intLineGap;
    }
    void SetLineGap(float lineGap) noexcept
    {
      _lineGap = lineGap;
      _intLineGap = static_cast<int>(std::lroundf(lineGap));
    }

    KRYS_NODISCARD float LineSpacing() const noexcept
    {
      return _lineSpacing.value_or(0.f);
    }
    KRYS_NODISCARD int IntLineSpacing() const noexcept
    {
      return _intLineSpacing;
    }
    void SetLineSpacing(float lineSpacing) noexcept
    {
      _lineSpacing = lineSpacing;
      _intLineSpacing = static_cast<int>(std::lroundf(lineSpacing));
    }

    KRYS_NODISCARD Markable<float> XHeight() const noexcept
    {
      return _xHeight;
    }
    void SetXHeight(float xHeight) noexcept
    {
      _xHeight = xHeight;
    }

    KRYS_NODISCARD Markable<float> CapHeight() const noexcept
    {
      return _capHeight;
    }
    KRYS_NODISCARD int IntCapHeight() const noexcept
    {
      return _intCapHeight;
    }
    void SetCapHeight(float capHeight) noexcept
    {
      _capHeight = capHeight;
      _intCapHeight = static_cast<int>(std::lroundf(capHeight));
    }

    KRYS_NODISCARD Markable<float> ZeroWidth() const noexcept
    {
      return _zeroWidth;
    }
    void SetZeroWidth(float zeroWidth) noexcept
    {
      _zeroWidth = zeroWidth;
    }

    KRYS_NODISCARD Markable<float> IdeogramWidth() const noexcept
    {
      return _ideogramWidth;
    }
    void SetIdeogramWidth(float ideogramWidth) noexcept
    {
      _ideogramWidth = ideogramWidth;
    }

    KRYS_NODISCARD Markable<float> UnderlinePosition() const noexcept
    {
      return _underlinePosition;
    }
    void SetUnderlinePosition(float underlinePosition) noexcept
    {
      _underlinePosition = underlinePosition;
    }

    KRYS_NODISCARD Markable<float> UnderlineThickness() const noexcept
    {
      return _underlineThickness;
    }
    void SetUnderlineThickness(float underlineThickness) noexcept
    {
      _underlineThickness = underlineThickness;
    }

    KRYS_NODISCARD bool HasIdenticalAscentDescentAndLineGap(const FontMetrics &other) const noexcept
    {
      return IntAscent() == other.IntAscent() && IntDescent() == other.IntDescent()
             && IntLineGap() == other.IntLineGap();
    }

  private:
    void Reset() noexcept
    {
      *this = FontMetrics {};
    }
  };

  KRYS_NODISCARD inline float ScaleEmToUnits(float x, unsigned unitsPerEm) noexcept
  {
    return unitsPerEm ? x / static_cast<float>(unitsPerEm) : x;
  }
}
