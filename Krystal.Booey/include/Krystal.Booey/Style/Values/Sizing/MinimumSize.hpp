#pragma once

#include "Krystal.Booey/Style/Values/Primitives/LengthWrapper.hpp"

namespace krys::boo::style
{
  struct PreferredSize;

  // <'min-width'>/<'min-height'> = auto | <length-percentage [0,∞]> | min-content | max-content |
  // fit-content(<length-percentage [0,∞]>) | <calc-size()> | stretch | fit-content | contain
  //
  // What is actually implemented is:
  //
  // <'min-width'>/<'min-height'> = auto | <length-percentage [0,∞]> | min-content | max-content | fit-content
  // | intrinsic | min-intrinsic | -webkit-fill-available
  //
  // MISSING:
  //    fit-content(<length-percentage [0,∞]>)
  //    <calc-size()>
  //    stretch
  //    contain
  //
  // NON-STANDARD:
  //    intrinsic
  //    min-intrinsic
  //    -webkit-fill-available
  //
  // https://drafts.csswg.org/css-sizing-3/#min-size-properties
  // https://drafts.csswg.org/css-sizing-4/#sizing-values (additional values added)
  struct MinimumSize : LengthWrapperBase<LengthPercentage<css::NonNegativeUnzoomed>, css::keywords::Auto,
                                         css::keywords::MinContent, css::keywords::MaxContent,
                                         css::keywords::FitContent, css::keywords::Stretch>
  {
    using Base::Base;

    KRYS_ALWAYS_INLINE KRYS_NODISCARD bool IsAuto() const noexcept
    {
      return HoldsAlternative<css::keywords::Auto>();
    }
    KRYS_ALWAYS_INLINE KRYS_NODISCARD bool IsMinContent() const noexcept
    {
      return HoldsAlternative<css::keywords::MinContent>();
    }
    KRYS_ALWAYS_INLINE KRYS_NODISCARD bool IsMaxContent() const noexcept
    {
      return HoldsAlternative<css::keywords::MaxContent>();
    }
    KRYS_ALWAYS_INLINE KRYS_NODISCARD bool IsFitContent() const noexcept
    {
      return HoldsAlternative<css::keywords::FitContent>();
    }
    KRYS_ALWAYS_INLINE KRYS_NODISCARD bool IsFillAvailable() const noexcept
    {
      return HoldsAlternative<css::keywords::Stretch>();
    }

    KRYS_ALWAYS_INLINE KRYS_NODISCARD bool IsIntrinsic() const noexcept
    {
      return HoldsAlternative<css::keywords::MinContent>() || HoldsAlternative<css::keywords::MaxContent>()
             || HoldsAlternative<css::keywords::Stretch>() || HoldsAlternative<css::keywords::FitContent>();
    }
    KRYS_ALWAYS_INLINE KRYS_NODISCARD bool IsIntrinsicOrAuto() const noexcept
    {
      return HoldsAlternative<css::keywords::MinContent>() || HoldsAlternative<css::keywords::MaxContent>()
             || HoldsAlternative<css::keywords::Stretch>() || HoldsAlternative<css::keywords::FitContent>()
             || HoldsAlternative<css::keywords::Auto>();
    }

  private:
    friend struct PreferredSize;
  };

  using MinimumSizePair = css::SpaceSeparatedSize<MinimumSize>;
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::MinimumSize)
