#pragma once

#include "Krystal.Booey/Style/Values/Primitives/LengthWrapper.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypes.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo::style
{
  // <'item-tolerance'> = normal | <length-percentage [0,∞]> | infinite
  // https://drafts.csswg.org/css-grid-3/#item-tolerance
  struct ItemTolerance : LengthWrapperBase<LengthPercentage<css::NonNegativeUnzoomed>, css::keywords::Normal,
                                           css::keywords::Infinite>
  {
    using Base::Base;

    KRYS_ALWAYS_INLINE KRYS_NODISCARD bool IsNormal() const noexcept
    {
      return HoldsAlternative<css::keywords::Normal>();
    }
    KRYS_ALWAYS_INLINE KRYS_NODISCARD bool IsInfinite() const noexcept
    {
      return HoldsAlternative<css::keywords::Infinite>();
    }
  };

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<ItemTolerance>
  {
    KRYS_NODISCARD ItemTolerance operator()(BuilderState &, const css::Value &) noexcept;
  };

  // MARK: - Blending

  template <>
  struct Blending<ItemTolerance>
  {
    KRYS_NODISCARD bool CanBlend(const ItemTolerance &, const ItemTolerance &) noexcept;
    KRYS_NODISCARD ItemTolerance Blend(const ItemTolerance &, const ItemTolerance &,
                                       const BlendingContext &) noexcept;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::ItemTolerance)
