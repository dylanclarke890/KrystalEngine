/*
 * Copyright (C) 2017 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Animation/Timing/FillMode.hpp"
#include "Krystal.Booey/Animation/Timing/PlaybackDirection.hpp"
#include "Krystal.Booey/CSS/TypedOM/CSSNumericValue.hpp"
#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/Variant.hpp"
#include "Krystal.Core/Visitor.hpp"

namespace krys::boo
{
  using OptionalDoubleOrString = Maybe<Variant<double, dom::DOMString>>;
  using DoubleOrCSSNumericValueOrString = Variant<double, RefPtr<css::CSSNumericValue>, dom::DOMString>;

  /// @brief An animation effect's specified timing, as the Web Animations interfaces give and take it.
  /// @see https://drafts.csswg.org/web-animations-1/#dictdef-effecttiming
  struct EffectTiming
  {
    /// @brief In milliseconds, a numeric value (a percentage, for a progress-based timeline), or the string
    /// `auto`.
    DoubleOrCSSNumericValueOrString Duration {dom::DOMString(u8"auto")};
    /// @brief In milliseconds.
    double Delay {0};
    /// @brief In milliseconds.
    double EndDelay {0};
    double IterationStart {0};
    double Iterations {1};
    dom::DOMString Easing {u8"linear"};
    FillMode Fill {FillMode::Auto};
    PlaybackDirection Direction {PlaybackDirection::Normal};

    /// @brief The duration as a number or a string; null for a numeric value.
    KRYS_NODISCARD OptionalDoubleOrString DurationAsDoubleOrString() const noexcept
    {
      return krys::SwitchOn(
        Duration, [](double doubleDuration) -> OptionalDoubleOrString { return doubleDuration; },
        [](const dom::DOMString &stringDuration) -> OptionalDoubleOrString { return stringDuration; },
        [](const RefPtr<css::CSSNumericValue> &) -> OptionalDoubleOrString { return null; });
    }
  };
}
