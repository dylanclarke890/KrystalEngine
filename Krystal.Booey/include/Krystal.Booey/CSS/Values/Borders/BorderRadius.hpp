/*
 * Copyright (C) 2024 Samuel Weinig <sam@webkit.org>
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

#include "Krystal.Booey/CSS/Values/Aggregates.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypes.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::css
{
  // <'border-radius'> = <length-percentage [0,∞]>{1,4} [ / <length-percentage [0,∞]>{1,4} ]?
  // https://drafts.csswg.org/css-backgrounds-3/#propdef-border-radius
  struct BorderRadius
  {
    using Axis = SpaceSeparatedArray<LengthPercentage<NonNegative>, 4>;
    using Corner = MinimallySerialisingSpaceSeparatedSize<LengthPercentage<NonNegative>>;

    Axis Horizontal;
    Axis Vertical;

    KRYS_NODISCARD static BorderRadius DefaultValue() noexcept
    {
      return BorderRadius {
        .Horizontal = {0_css_px, 0_css_px, 0_css_px, 0_css_px},
        .Vertical = {0_css_px, 0_css_px, 0_css_px, 0_css_px},
      };
    }

    KRYS_NODISCARD Corner TopLeft() const noexcept
    {
      return {Horizontal.Value[0], Vertical.Value[0]};
    }

    KRYS_NODISCARD Corner TopRight() const noexcept
    {
      return {Horizontal.Value[1], Vertical.Value[1]};
    }

    KRYS_NODISCARD Corner BottomRight() const noexcept
    {
      return {Horizontal.Value[2], Vertical.Value[2]};
    }

    KRYS_NODISCARD Corner BottomLeft() const noexcept
    {
      return {Horizontal.Value[3], Vertical.Value[3]};
    }

    KRYS_NODISCARD bool operator==(const BorderRadius &) const noexcept = default;
  };

  template <size_t I>
  KRYS_NODISCARD const auto &get(const BorderRadius &value) noexcept
  {
    if constexpr (!I)
    {
      return value.Horizontal;
    }
    else if constexpr (I == 1)
    {
      return value.Vertical;
    }
  }

  // Returns true if the provided `BorderRadius` contains the default value. This
  // is used to know ahead of time if serialization is needed.
  KRYS_NODISCARD bool HasDefaultValue(const BorderRadius &borderRadius) noexcept;

  template <>
  struct Serialiser<BorderRadius>
  {
    void operator()(StringBuilder &builder, const SerialisationContext &context,
                    const BorderRadius &value) noexcept;
  };
}

DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::css::BorderRadius, 2)
