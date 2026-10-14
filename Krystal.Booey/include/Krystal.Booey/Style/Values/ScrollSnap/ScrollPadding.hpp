#pragma once

#include "Krystal.Booey/Core/BoxExtents.hpp"
#include "Krystal.Booey/Style/Values/Primitives/LengthWrapper.hpp"

namespace krys::boo
{
  class LayoutRect;

  namespace style
  {
    // <'scroll-padding-*'> = auto | <length-percentage [0,∞]>
    // https://drafts.csswg.org/css-scroll-snap-1/#padding-longhands-physical
    struct ScrollPaddingEdge : LengthWrapperBase<LengthPercentage<css::NonNegative>, css::keywords::Auto>
    {
      using Base::Base;
    };

    // <'scroll-padding'> = [ auto | <length-percentage [0,∞]> ]{1,4}
    // https://drafts.csswg.org/css-scroll-snap-1/#propdef-scroll-padding
    using ScrollPaddingBox = css::MinimallySerialisingSpaceSeparatedRectEdges<ScrollPaddingEdge>;

    // MARK: - Evaluation

    template <>
    struct Evaluation<ScrollPaddingEdge, LayoutUnit>
    {
      KRYS_NODISCARD LayoutUnit operator()(const ScrollPaddingEdge &, LayoutUnit referenceLength,
                                           ZoomNeeded) noexcept;
    };
    template <>
    struct Evaluation<ScrollPaddingEdge, float>
    {
      KRYS_NODISCARD float operator()(const ScrollPaddingEdge &, float referenceLength, ZoomNeeded) noexcept;
    };

    // MARK: - Extent

    KRYS_NODISCARD LayoutBoxExtent ExtentForRect(const ScrollPaddingBox &, const LayoutRect &,
                                                 ZoomNeeded) noexcept;

  }
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::ScrollPaddingEdge)
