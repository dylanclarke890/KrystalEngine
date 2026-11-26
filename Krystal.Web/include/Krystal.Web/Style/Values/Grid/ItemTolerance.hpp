/*
 * Copyright (C) 2025 Apple Inc. All rights reserved.
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
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Web/Style/Values/Primitives/LengthWrapper.hpp"
#include "Krystal.Web/Style/Values/Primitives/NumericTypes.hpp"
#include "Krystal.Web/Style/Values/Types.hpp"

namespace krys::web::style
{
  // <'item-tolerance'> = normal | <length-percentage [0,∞]> | infinite
  // https://drafts.csswg.org/css-grid-3/#item-tolerance
  struct ItemTolerance : LengthWrapperBase<LengthPercentage<css::NonNegativeUnzoomed>, css::keywords::Normal,
                                           css::keywords::Infinite>
  {
    using Base::Base;

    KRYS_NODISCARD KRYS_ALWAYS_INLINE bool IsNormal() const noexcept
    {
      return HoldsAlternative<css::keywords::Normal>();
    }

    KRYS_NODISCARD KRYS_ALWAYS_INLINE bool IsInfinite() const noexcept
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
    KRYS_NODISCARD static bool CanBlend(const ItemTolerance &, const ItemTolerance &) noexcept;

    KRYS_NODISCARD static ItemTolerance Blend(const ItemTolerance &, const ItemTolerance &,
                                              const BlendingContext &) noexcept;
  };
}

KRYS_DEFINE_VARIANT_LIKE_CONFORMANCE(krys::web::style::ItemTolerance)
