#pragma once

#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo::style
{
  // <overflow-position> = unsafe | safe
  // https://drafts.csswg.org/css-align/#typedef-overflow-position
  using OverflowPosition = Variant<css::keywords::Unsafe, css::keywords::Safe>;

  // Used by alignment types in bitfields.
  enum class OverflowPositionKind : uint8
  {
    None,
    Unsafe,
    Safe
  };

  KRYS_NODISCARD constexpr OverflowPositionKind ComputeKind(Maybe<OverflowPosition> overflow) noexcept
  {
    if (!overflow)
    {
      return OverflowPositionKind::None;
    }
    return krys::SwitchOn(
      *overflow, [](css::keywords::Unsafe) { return OverflowPositionKind::Unsafe; },
      [](css::keywords::Safe) { return OverflowPositionKind::Safe; });
  }

  constexpr decltype(auto) VisitOverflowPosition(auto &&primaryKeyword, OverflowPositionKind kind,
                                                 auto &&visitor) noexcept
  {
    switch (kind)
    {
      case OverflowPositionKind::None: return visitor(primaryKeyword);
      case OverflowPositionKind::Unsafe:
        return visitor(css::SpaceSeparatedTuple {css::keywords::Unsafe {}, primaryKeyword});
      case OverflowPositionKind::Safe:
        return visitor(css::SpaceSeparatedTuple {css::keywords::Safe {}, primaryKeyword});
    }
    krys_unreachable();
  }
}
