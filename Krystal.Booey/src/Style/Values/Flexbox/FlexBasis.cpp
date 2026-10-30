#include "Krystal.Booey/Style/Values/Flexbox/FlexBasis.hpp"

#include "Krystal.Booey/Style/Values/Sizing/PreferredSize.hpp"

namespace krys::boo::style
{
  Maybe<PreferredSize> FlexBasis::TryPreferredSize() const noexcept
  {
    return krys::SwitchOn(
      *this, [&](const css::keywords::Content &) -> Maybe<PreferredSize> { return {}; },
      [&](const auto &value) -> Maybe<PreferredSize> { return PreferredSize {value}; });
  }
}
