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

#include "Krystal.Booey/CSS/Values/Easing/CubicBezierEasingFunction.hpp"
#include "Krystal.Booey/CSS/Values/Easing/LinearEasingFunction.hpp"
#include "Krystal.Booey/CSS/Values/Easing/SpringEasingFunction.hpp"
#include "Krystal.Booey/CSS/Values/Easing/StepsEasingFunction.hpp"
#include "Krystal.Core/Types/Variant.hpp"

namespace krys::boo::css
{
  // `EasingFunction` uses a wrapping a struct to allow forward declaration.
  struct EasingFunction
  {
    Variant<
      // <linear()>
      keywords::Linear, // Equivalent to linear(0, 1)
      LinearEasingFunction,
      // <cubic-bezier()>
      keywords::Ease,      // Equivalent to cubic-bezier(0.25, 0.1, 0.25, 1)
      keywords::EaseIn,    // Equivalent to cubic-bezier(0.42, 0, 1, 1)
      keywords::EaseOut,   // Equivalent to cubic-bezier(0, 0, 0.58, 1)
      keywords::EaseInOut, // Equivalent to cubic-bezier(0.42, 0, 0.58, 1)
      CubicBezierEasingFunction,
      // <steps()>
      keywords::StepStart, // Equivalent to steps(1, start)
      keywords::StepEnd,   // Equivalent to steps(1, end)
      StepsEasingFunction,
      // <spring()>
      SpringEasingFunction>
      Value;

    template <typename... F>
    KRYS_NODISCARD constexpr decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      return krys::SwitchOn(Value, std::forward<F>(f)...);
    }

    KRYS_NODISCARD constexpr bool operator==(const EasingFunction &) const noexcept = default;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::css::EasingFunction)
