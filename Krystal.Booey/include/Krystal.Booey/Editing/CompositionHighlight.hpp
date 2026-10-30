#pragma once

#include "Krystal.Booey/Graphics/Color/Color.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include <compare>
#include <utility>

namespace krys::boo
{
  /// @brief Colours for part of the text being composed with an input method.
  struct CompositionHighlight
  {
    /// @brief The fill behind a composition that has no underlines.
    static constexpr auto DefaultCompositionFillColor = SRGBA<uint8> {225, 221, 85};

    uint32 StartOffset {0};
    uint32 EndOffset {0};
    Maybe<Color> BackgroundColor;
    Maybe<Color> ForegroundColor;

    friend std::strong_ordering operator<=>(const CompositionHighlight &a,
                                            const CompositionHighlight &b) noexcept
    {
      return std::make_pair(a.StartOffset, a.EndOffset) <=> std::make_pair(b.StartOffset, b.EndOffset);
    }

    friend bool operator==(const CompositionHighlight &, const CompositionHighlight &) noexcept = default;
  };
}
