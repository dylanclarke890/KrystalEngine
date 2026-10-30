#pragma once

#include "Krystal.Booey/Style/Values/Primitives/LengthWrapper.hpp"

namespace krys::boo::style
{
  // <'offset-distance'> = <length-percentage>
  // https://drafts.fxtf.org/motion/#propdef-offset-distance
  struct OffsetDistance : LengthWrapperBase<LengthPercentage<>>
  {
    using Base::Base;
  };

  // MARK: - Evaluation
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::OffsetDistance)
