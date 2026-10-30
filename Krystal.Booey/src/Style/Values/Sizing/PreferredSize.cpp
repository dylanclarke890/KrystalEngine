#include "Krystal.Booey/Style/Values/Sizing/PreferredSize.hpp"

#include "Krystal.Booey/Style/Values/Flexbox/FlexBasis.hpp"
#include "Krystal.Booey/Style/Values/Sizing/MinimumSize.hpp"

namespace krys::boo::style
{
  MinimumSize PreferredSize::AsMinimumSize() const noexcept
  {
    return krys::SwitchOn(*this, [&](const auto &value) { return MinimumSize {value}; });
  }

  FlexBasis PreferredSize::AsFlexBasis() const noexcept
  {
    return krys::SwitchOn(*this, [&](const auto &value) { return FlexBasis {value}; });
  }
}
