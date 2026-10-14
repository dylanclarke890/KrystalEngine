#pragma once

#include "Krystal.Booey/Style/Values/Primitives/LengthWrapper.hpp"

namespace krys::boo::style
{
  struct PreferredSize;

  // <'flex-basis'> = content | <‘width’>
  // https://drafts.csswg.org/css-flexbox/#propdef-flex-basis
  struct FlexBasis
      : LengthWrapperBase<LengthPercentage<css::NonNegativeUnzoomed>, css::keywords::Content,
                          css::keywords::Auto, css::keywords::MinContent, css::keywords::MaxContent,
                          css::keywords::FitContent, css::keywords::WebkitFillAvailable,
                          css::keywords::Intrinsic, css::keywords::MinIntrinsic>
  {
    using Base::Base;

    // `FlexBasis` is a superset of `PreferredSize` and therefore this conversion can fail when type is
    // `content`.
    KRYS_NODISCARD Maybe<PreferredSize> TryPreferredSize() const noexcept;

    KRYS_ALWAYS_INLINE KRYS_NODISCARD bool IsContent() const noexcept
    {
      return HoldsAlternative<css::keywords::Content>();
    }
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
      return HoldsAlternative<css::keywords::WebkitFillAvailable>();
    }
    KRYS_ALWAYS_INLINE KRYS_NODISCARD bool IsIntrinsicKeyword() const noexcept
    {
      return HoldsAlternative<css::keywords::Intrinsic>();
    }
    KRYS_ALWAYS_INLINE KRYS_NODISCARD bool IsMinIntrinsic() const noexcept
    {
      return HoldsAlternative<css::keywords::MinIntrinsic>();
    }

    KRYS_ALWAYS_INLINE KRYS_NODISCARD bool IsIntrinsic() const noexcept
    {
      return HoldsAlternative<css::keywords::MinContent>() || HoldsAlternative<css::keywords::MaxContent>()
             || HoldsAlternative<css::keywords::WebkitFillAvailable>()
             || HoldsAlternative<css::keywords::FitContent>();
    }
    KRYS_ALWAYS_INLINE KRYS_NODISCARD bool IsLegacyIntrinsic() const noexcept
    {
      return HoldsAlternative<css::keywords::Intrinsic>() || HoldsAlternative<css::keywords::MinIntrinsic>();
    }
    KRYS_ALWAYS_INLINE KRYS_NODISCARD bool IsIntrinsicOrLegacyIntrinsicOrAuto() const noexcept
    {
      return HoldsAlternative<css::keywords::MinContent>() || HoldsAlternative<css::keywords::MaxContent>()
             || HoldsAlternative<css::keywords::WebkitFillAvailable>()
             || HoldsAlternative<css::keywords::FitContent>() || HoldsAlternative<css::keywords::Intrinsic>()
             || HoldsAlternative<css::keywords::MinIntrinsic>() || HoldsAlternative<css::keywords::Auto>();
    }
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::FlexBasis)
