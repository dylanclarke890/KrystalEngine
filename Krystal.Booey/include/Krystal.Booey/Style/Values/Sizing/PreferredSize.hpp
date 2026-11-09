/*
 * Copyright (C) 2025 Samuel Weinig <sam@webkit.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/Style/Values/Primitives/LengthWrapper.hpp"

namespace krys::boo::style
{
  struct FlexBasis;
  struct MinimumSize;

  // <'width'>/<'height'> = auto | <length-percentage [0,∞]> | min-content | max-content |
  // fit-content(<length-percentage [0,∞]>) | <calc-size()> | stretch | fit-content | contain
  //
  // What is actually implemented is:
  //
  // <'width'>/<'height'> = auto | <length-percentage [0,∞]> | min-content | max-content | fit-content |
  // intrinsic | min-intrinsic | -webkit-fill-available
  //
  // MISSING:
  //    fit-content(<length-percentage [0,∞]>)
  //    <calc-size()>
  //    stretch
  //    contain

  // NON-STANDARD:
  //    intrinsic
  //    min-intrinsic
  //    -webkit-fill-available
  //
  // https://drafts.csswg.org/css-sizing-3/#preferred-size-properties
  // https://drafts.csswg.org/css-sizing-4/#sizing-values (additional values added)
  struct PreferredSize : LengthWrapperBase<LengthPercentage<css::NonNegativeUnzoomed>, css::keywords::Auto,
                                           css::keywords::MinContent, css::keywords::MaxContent,
                                           css::keywords::FitContent, css::keywords::Stretch>
  {
    using Base::Base;

    // `PreferredSize` is a structural twin to `MinimumSize` and therefore can be losslessly converted.
    KRYS_NODISCARD MinimumSize AsMinimumSize() const noexcept;

    // `PreferredSize` is a structural subset of `FlexBasis` and therefore can be losslessly converted.
    KRYS_NODISCARD FlexBasis AsFlexBasis() const noexcept;

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
    friend struct FlexBasis;
    friend struct MinimumSize;
  };

  using PreferredSizePair = css::SpaceSeparatedSize<PreferredSize>;
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::PreferredSize)

namespace krys
{
  template <>
  struct HashTraits<krys::boo::style::PreferredSize> : GenericHashTraits<krys::boo::style::PreferredSize>
  {
    KRYS_NODISCARD static krys::boo::style::PreferredSize EmptyValue() noexcept
    {
      return krys::boo::style::PreferredSize {HashTableEmptyValueTag {}};
    }
  };
}
