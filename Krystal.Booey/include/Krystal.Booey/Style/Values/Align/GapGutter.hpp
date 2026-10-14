#pragma once

#include "Krystal.Booey/Style/Values/Primitives/LengthWrapper.hpp"

namespace krys::boo::style
{
  // <'row-gap/column-gap'> = normal | <length-percentage [0,∞]>
  // https://drafts.csswg.org/css-align/#column-row-gap
  struct GapGutter : LengthWrapperBase<LengthPercentage<css::NonNegative>, css::keywords::Normal>
  {
    using Base::Base;

    KRYS_ALWAYS_INLINE KRYS_NODISCARD bool IsNormal() const noexcept
    {
      return HoldsAlternative<css::keywords::Normal>();
    }
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::GapGutter)
