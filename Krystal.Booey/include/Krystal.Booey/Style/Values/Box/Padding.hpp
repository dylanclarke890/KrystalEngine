#pragma once

#include "Krystal.Booey/Style/Values/Primitives/LengthWrapper.hpp"

namespace krys::boo::style
{
  // <'padding-*'> = <length-percentage [0,∞]>
  // https://drafts.csswg.org/css-box/#padding-physical
  struct PaddingEdge : LengthWrapperBase<LengthPercentage<css::NonNegativeUnzoomed>>
  {
    using Base::Base;
  };

  // <'padding'> = <'padding-top'>{1,4}
  // https://drafts.csswg.org/css-box/#propdef-padding
  using PaddingBox = css::MinimallySerialisingSpaceSeparatedRectEdges<PaddingEdge>;
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::PaddingEdge)
