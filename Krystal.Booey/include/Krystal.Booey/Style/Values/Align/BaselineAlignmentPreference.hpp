#pragma once

#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo::style
{
  // <baseline-position-preference> = first | last
  // https://drafts.csswg.org/css-align/#baseline-alignment-preference
  using BaselineAlignmentPreference = Variant<css::keywords::First, css::keywords::Last>;

  // Used by alignment types in bitfields.
  enum class BaselineAlignmentPreferenceKind : bool
  {
    First,
    Last
  };

  KRYS_NODISCARD constexpr BaselineAlignmentPreferenceKind
    ComputeKind(Maybe<BaselineAlignmentPreference> preference) noexcept
  {
    if (!preference)
    {
      return BaselineAlignmentPreferenceKind::First;
    }
    return krys::SwitchOn(
      *preference, [](css::keywords::First) { return BaselineAlignmentPreferenceKind::First; },
      [](css::keywords::Last) { return BaselineAlignmentPreferenceKind::Last; });
  }

  constexpr decltype(auto) VisitBaselineAlignmentPreference(auto &&primaryKeyword,
                                                            BaselineAlignmentPreferenceKind kind,
                                                            auto &&visitor) noexcept
  {
    switch (kind)
    {
      case BaselineAlignmentPreferenceKind::First: return visitor(primaryKeyword);
      case BaselineAlignmentPreferenceKind::Last:
        return visitor(css::SpaceSeparatedTuple {css::keywords::Last {}, primaryKeyword});
    }
    krys_unreachable();
  }
}
