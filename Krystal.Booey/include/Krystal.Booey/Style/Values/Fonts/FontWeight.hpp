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
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/Graphics/Fonts/FontSelectionAlgorithm.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypes.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo::style
{
  // <'font-weight'> = normal | bold | bolder | lighter | <number [1,1000]>
  // NOTE: Computed value is always resolved to a <number [1,1000]>.
  // https://drafts.csswg.org/css-fonts-4/#propdef-font-weight
  struct FontWeight
  {
    using Number = style::Number<css::Range {1, 1'000}>;

    FontWeight(css::keywords::Normal) noexcept : _platform {NormalWeightValue()}
    {
    }
    FontWeight(css::keywords::Bold) noexcept : _platform {BoldWeightValue()}
    {
    }
    FontWeight(Number number) noexcept
        : _platform {FontSelectionValue::ClampFloat(static_cast<float>(number.Value))}
    {
    }

    FontWeight(FontSelectionValue platform) noexcept : _platform {platform}
    {
    }

    KRYS_NODISCARD bool IsNormal() const noexcept
    {
      return _platform == NormalWeightValue();
    }
    KRYS_NODISCARD bool IsBold() const noexcept
    {
      return _platform == BoldWeightValue();
    }

    KRYS_NODISCARD Number GetNumber() const noexcept
    {
      return Number {static_cast<float>(_platform)};
    }

    template <typename... F>
    decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      auto visitor = krys::CreateVisitor(std::forward<F>(f)...);
      return visitor(GetNumber());
    }

    KRYS_NODISCARD FontSelectionValue Platform() const noexcept
    {
      return _platform;
    }

    // NOTE: This is not if the value would compute to the keyword `bold`, but rather a more general whether
    // the weight is large enough to be considered "bold" (see `boo::BoldThreshold()`).
    KRYS_NODISCARD bool IsConsideredBold() const noexcept
    {
      return boo::IsFontWeightBold(_platform);
    }

    KRYS_NODISCARD bool operator==(const FontWeight &) const noexcept = default;

  private:
    FontSelectionValue _platform;
  };

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<FontWeight>
  {
    KRYS_NODISCARD FontWeight operator()(BuilderState &, const css::Value &) noexcept;
  };

  // MARK: - Blending

  template <>
  struct Blending<FontWeight>
  {
    KRYS_NODISCARD FontWeight Blend(const FontWeight &, const FontWeight &, const BlendingContext &) noexcept;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::FontWeight)
