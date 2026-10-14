#pragma once

#include "Krystal.Booey/Style/Values/Primitives/NumericTypes.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo::style
{
  // <'block-step-size'> = none | <length [0,∞]>
  // https://www.w3.org/TR/css-rhythm-1/#propdef-block-step-size
  struct BlockStepSize : css::ValueOrKeyword<Length<css::NonNegative, float>, css::keywords::None>
  {
    using Base::Base;
    using Length = typename Base::Value;

    KRYS_NODISCARD bool IsNone() const noexcept
    {
      return IsKeyword();
    }
    KRYS_NODISCARD bool IsLength() const noexcept
    {
      return IsValue();
    }
    KRYS_NODISCARD Maybe<Length> TryLength() const noexcept
    {
      return TryValue();
    }
  };

  // MARK: - Blending

  template <>
  struct Blending<BlockStepSize>
  {
    KRYS_NODISCARD bool CanBlend(const BlockStepSize &, const BlockStepSize &) noexcept;
    KRYS_NODISCARD bool RequiresInterpolationForAccumulativeIteration(const BlockStepSize &,
                                                                      const BlockStepSize &) noexcept;
    KRYS_NODISCARD BlockStepSize Blend(const BlockStepSize &, const BlockStepSize &,
                                       const BlendingContext &) noexcept;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::BlockStepSize)
