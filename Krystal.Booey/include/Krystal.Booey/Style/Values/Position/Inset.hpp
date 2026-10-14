#pragma once

#include "Krystal.Booey/Style/Values/Primitives/LengthWrapper.hpp"

namespace krys::boo::style
{
  // <'top'>/<'right'>/<'bottom'>/<'left'> = auto | <length-percentage>
  // https://drafts.csswg.org/css-position/#insets
  struct InsetEdge : LengthWrapperBase<LengthPercentage<css::AllUnzoomed>, css::keywords::Auto>
  {
    using Base::Base;

    KRYS_ALWAYS_INLINE KRYS_NODISCARD bool IsAuto() const noexcept
    {
      return HoldsAlternative<css::keywords::Auto>();
    }
  };

  // <'inset'> = <'top'>{1,4}
  // https://drafts.csswg.org/css-position/#propdef-inset
  using InsetBox = css::MinimallySerialisingSpaceSeparatedRectEdges<InsetEdge>;
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::InsetEdge)
