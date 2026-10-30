#pragma once

#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo::style
{
  // <legacy-position> = left | right | center
  // https://drafts.csswg.org/css-align/#valdef-justify-items-legacy
  using LegacyPosition = Variant<css::keywords::Left, css::keywords::Right, css::keywords::Center>;

  // Used by alignment types in bitfields.
  enum class LegacyPositionKind : uint8
  {
    None,
    Left,
    Right,
    Center
  };

  KRYS_NODISCARD constexpr LegacyPositionKind ComputeKind(Maybe<LegacyPosition> position) noexcept
  {
    if (!position)
    {
      return LegacyPositionKind::None;
    }
    return krys::SwitchOn(
      *position, [](css::keywords::Left) { return LegacyPositionKind::Left; },
      [](css::keywords::Right) { return LegacyPositionKind::Right; },
      [](css::keywords::Center) { return LegacyPositionKind::Center; });
  }

  constexpr decltype(auto) VisitLegacyPosition(auto &&primaryKeyword, LegacyPositionKind kind,
                                               auto &&visitor) noexcept
  {
    switch (kind)
    {
      case LegacyPositionKind::None:
        // `legacy` on its own computes to `normal`.
        // https://drafts.csswg.org/css-align/#valdef-justify-items-legacy
        return visitor(css::keywords::Normal {});
      case LegacyPositionKind::Left:
        return visitor(css::SpaceSeparatedTuple {primaryKeyword, css::keywords::Left {}});
      case LegacyPositionKind::Right:
        return visitor(css::SpaceSeparatedTuple {primaryKeyword, css::keywords::Right {}});
      case LegacyPositionKind::Center:
        return visitor(css::SpaceSeparatedTuple {primaryKeyword, css::keywords::Center {}});
    }
    krys_unreachable();
  }
}
