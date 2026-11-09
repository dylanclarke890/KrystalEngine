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

#include "Krystal.Booey/Graphics/Animation/TimingFunction.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo
{
  namespace css
  {
    struct EasingFunction;
  }

  namespace style
  {
    // <easing-function> = linear | <cubic-bezier-easing-function> | <step-easing-function>
    // https://www.w3.org/TR/css-easing-1/#typedef-easing-function
    struct EasingFunction
    {
      Ref<TimingFunction> Value;

      KRYS_NODISCARD bool operator==(const EasingFunction &other) const noexcept
      {
        return Value.get() == other.Value.get() || *Value == *other.Value;
      }
    };

    // MARK: - Deprecated Conversions

    KRYS_NODISCARD Ref<TimingFunction> CreateTimingFunctionDeprecated(const css::EasingFunction &) noexcept;
    KRYS_NODISCARD RefPtr<TimingFunction> CreateTimingFunctionDeprecated(const css::Value &) noexcept;

    // MARK: - Conversion

    template <>
    struct CSSValueConversion<EasingFunction>
    {
      KRYS_NODISCARD EasingFunction operator()(BuilderState &, const css::Value &) noexcept;
    };
    template <>
    struct CSSValueCreation<EasingFunction>
    {
      KRYS_NODISCARD Ref<css::Value> operator()(css::ValuePool &, const RenderStyle &,
                                                const EasingFunction &) noexcept;
    };

    // MARK: - Serialization

    template <>
    struct Serialiser<EasingFunction>
    {
      void operator()(StringBuilder &, const css::SerialisationContext &, const RenderStyle &,
                      const EasingFunction &) noexcept;
    };
  }
}
