#pragma once

#include "Krystal.Booey/Graphics/Color/Color.hpp"
#include "Krystal.Core/Numeric.hpp"
#include <compare>
#include <utility>

namespace krys::boo
{
  enum class CompositionUnderlineColor : bool
  {
    GivenColor,
    TextColor
  };

  /// @brief An underline under part of the text being composed with an input method (a clause), thick for the
  /// clause being converted.
  struct CompositionUnderline
  {
    uint32 StartOffset {0};
    uint32 EndOffset {0};
    CompositionUnderlineColor UnderlineColor {CompositionUnderlineColor::TextColor};
    boo::Color Color;
    bool Thick {false};

    friend std::strong_ordering operator<=>(const CompositionUnderline &a,
                                            const CompositionUnderline &b) noexcept
    {
      return std::make_pair(a.StartOffset, a.EndOffset) <=> std::make_pair(b.StartOffset, b.EndOffset);
    }

    friend bool operator==(const CompositionUnderline &, const CompositionUnderline &) noexcept = default;
  };
}
