#pragma once

#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo::style
{
  // <'scroll-snap-align'> = [ none | start | end | center ]{1,2}
  // https://drafts.csswg.org/css-scroll-snap-1/#propdef-scroll-snap-align
  struct ScrollSnapAlign
  {
    constexpr ScrollSnapAlign(css::keywords::None) noexcept
    {
    }
    constexpr ScrollSnapAlign(ScrollSnapAxisAlignType bothAxes) noexcept
        : BlockAlign {bothAxes}, InlineAlign {bothAxes}
    {
    }
    constexpr ScrollSnapAlign(ScrollSnapAxisAlignType blockAlign,
                              ScrollSnapAxisAlignType inlineAlign) noexcept
        : BlockAlign {blockAlign}, InlineAlign {inlineAlign}
    {
    }

    KRYS_NODISCARD bool IsNone() const noexcept
    {
      return BlockAlign == ScrollSnapAxisAlignType::None && InlineAlign == ScrollSnapAxisAlignType::None;
    }

    ScrollSnapAxisAlignType BlockAlign {ScrollSnapAxisAlignType::None};
    ScrollSnapAxisAlignType InlineAlign {ScrollSnapAxisAlignType::None};

    KRYS_NODISCARD constexpr bool operator==(const ScrollSnapAlign &) const noexcept = default;
  };

  template <size_t I>
  constexpr const auto &get(const ScrollSnapAlign &value)
  {
    if constexpr (!I)
    {
      return value.BlockAlign;
    }
    else if constexpr (I == 1)
    {
      return value.InlineAlign;
    }
  }

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<ScrollSnapAlign>
  {
    KRYS_NODISCARD ScrollSnapAlign operator()(BuilderState &, const css::Value &) noexcept;
  };
}

DEFINE_COALESCING_SPACE_SEPARATED_TUPLE_LIKE_CONFORMANCE(krys::boo::style::ScrollSnapAlign, 2)
