#pragma once

#include "Krystal.Booey/Rendering/SVG/SVGTextMetrics.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/List.hpp"
#include <cmath>
#include <limits>

namespace krys::boo
{
  class RenderSVGInlineText;

  /// @brief The x/y/dx/dy/rotate values a character of SVG text was given.
  struct SVGCharacterData
  {
    float X {std::numeric_limits<float>::quiet_NaN()};
    float Y {std::numeric_limits<float>::quiet_NaN()};
    float Dx {std::numeric_limits<float>::quiet_NaN()};
    float Dy {std::numeric_limits<float>::quiet_NaN()};
    float Rotate {std::numeric_limits<float>::quiet_NaN()};
  };

  /// @brief Character data keyed by one plus a byte offset into a text renderer's text (or, while building,
  /// one plus a character's index in the whole text).
  using SVGCharacterDataMap = HashMap<uint32, SVGCharacterData>;

  /// @brief The layout attributes of an SVG text renderer: its characters' positioning data, and its metrics.
  class SVGTextLayoutAttributes
  {
    KRYS_NON_COPYABLE(SVGTextLayoutAttributes);

  private:
    RenderSVGInlineText &_context;
    SVGCharacterDataMap _characterDataMap;
    List<SVGTextMetrics> _textMetricsValues;

  public:
    explicit SVGTextLayoutAttributes(RenderSVGInlineText &) noexcept;

    void Clear() noexcept;

    KRYS_NODISCARD static constexpr float EmptyValue() noexcept
    {
      return std::numeric_limits<float>::quiet_NaN();
    }
    KRYS_NODISCARD static bool IsEmptyValue(float value) noexcept
    {
      return std::isnan(value);
    }

    KRYS_NODISCARD RenderSVGInlineText &Context() noexcept
    {
      return _context;
    }
    KRYS_NODISCARD const RenderSVGInlineText &Context() const noexcept
    {
      return _context;
    }

    KRYS_NODISCARD SVGCharacterDataMap &CharacterDataMap() noexcept
    {
      return _characterDataMap;
    }
    KRYS_NODISCARD const SVGCharacterDataMap &CharacterDataMap() const noexcept
    {
      return _characterDataMap;
    }

    KRYS_NODISCARD List<SVGTextMetrics> &TextMetricsValues() noexcept
    {
      return _textMetricsValues;
    }
    KRYS_NODISCARD const List<SVGTextMetrics> &TextMetricsValues() const noexcept
    {
      return _textMetricsValues;
    }
  };
}
