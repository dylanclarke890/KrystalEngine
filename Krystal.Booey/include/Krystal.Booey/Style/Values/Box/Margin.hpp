#pragma once

#include "Krystal.Booey/Style/Values/Primitives/LengthWrapper.hpp"

namespace krys::boo::style
{
  // <'margin-*'> = auto | <length-percentage>
  // https://drafts.csswg.org/css-box/#margin-physical
  struct MarginEdge : LengthWrapperBase<LengthPercentage<css::AllUnzoomed>, css::keywords::Auto>
  {
    using Base::Base;
    using Base::HasQuirk;

    KRYS_ALWAYS_INLINE KRYS_NODISCARD bool IsAuto() const noexcept
    {
      return HoldsAlternative<css::keywords::Auto>();
    }
  };

  template <>
  struct CSSValueConversion<MarginEdge>
  {
    KRYS_NODISCARD MarginEdge operator()(BuilderState &, const css::Value &) noexcept;
    KRYS_NODISCARD MarginEdge operator()(BuilderState &, const css::PrimitiveValue &) noexcept;
  };

  // <'margin'> = <'margin-top'>{1,4}
  // https://drafts.csswg.org/css-box/#propdef-margin
  using MarginBox = css::MinimallySerialisingSpaceSeparatedRectEdges<MarginEdge>;
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::MarginEdge)
