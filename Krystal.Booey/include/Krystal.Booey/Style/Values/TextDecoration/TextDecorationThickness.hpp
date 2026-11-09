/*
 * Copyright (C) 2018-2025 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/Style/Values/Primitives/LengthWrapper.hpp"

namespace krys::boo
{
  class FontMetrics;

  namespace style
  {
    struct TextDecorationThicknessLength : LengthWrapperBase<LengthPercentage<css::AllUnzoomed>>
    {
      using Base::Base;
    };

    // <'text-decoration-thickness'> = auto | from-font | <length-percentage>
    // https://drafts.csswg.org/css-text-decor-4/#propdef-text-decoration-thickness
    struct TextDecorationThickness
    {
      TextDecorationThickness(css::keywords::Auto keyword) noexcept : _value {keyword}
      {
      }

      TextDecorationThickness(css::keywords::FromFont keyword) noexcept : _value {keyword}
      {
      }

      TextDecorationThickness(TextDecorationThicknessLength &&length) noexcept : _value {krys::move(length)}
      {
      }

      KRYS_NODISCARD bool IsAuto() const noexcept
      {
        return krys::HoldsAlternative<css::keywords::Auto>(_value);
      }
      KRYS_NODISCARD bool IsFromFont() const noexcept
      {
        return krys::HoldsAlternative<css::keywords::FromFont>(_value);
      }
      KRYS_NODISCARD bool IsLength() const noexcept
      {
        return krys::HoldsAlternative<TextDecorationThicknessLength>(_value);
      }

      KRYS_NODISCARD float Resolve(const RenderStyle &) const noexcept;

      template <typename... F>
      decltype(auto) SwitchOn(F &&...f) const noexcept
      {
        return krys::SwitchOn(_value, std::forward<F>(f)...);
      }

      KRYS_NODISCARD bool operator==(const TextDecorationThickness &) const noexcept = default;

    private:
      Variant<css::keywords::Auto, css::keywords::FromFont, TextDecorationThicknessLength> _value;
    };

    // MARK: - Conversion

    template <>
    struct CSSValueConversion<TextDecorationThickness>
    {
      KRYS_NODISCARD TextDecorationThickness operator()(BuilderState &, const css::Value &) noexcept;
    };

    // MARK: - Blending

    template <>
    struct Blending<TextDecorationThickness>
    {
      KRYS_NODISCARD bool CanBlend(const TextDecorationThickness &, const TextDecorationThickness &,
                                   const RenderStyle &, const RenderStyle &) noexcept;
      KRYS_NODISCARD TextDecorationThickness Blend(const TextDecorationThickness &,
                                                   const TextDecorationThickness &, const RenderStyle &,
                                                   const RenderStyle &, const BlendingContext &) noexcept;
    };

  }
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::TextDecorationThicknessLength)
DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::TextDecorationThickness)
