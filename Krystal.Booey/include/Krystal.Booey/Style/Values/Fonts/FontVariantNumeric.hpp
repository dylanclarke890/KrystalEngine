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

#include "Krystal.Booey/Style/Values/Types.hpp"
#include "Krystal.Booey/Text/TextFlags.hpp"

namespace krys::boo::style
{
  // <'font-variant-numeric'> = normal | [ <numeric-figure-values> || <numeric-spacing-values> ||
  // <numeric-fraction-values> || ordinal || slashed-zero ]
  // https://drafts.csswg.org/css-fonts-4/#propdef-font-variant-numeric
  struct FontVariantNumeric
  {
    using Platform = boo::FontVariantNumericValues;

    constexpr FontVariantNumeric(css::keywords::Normal) noexcept : _platform {}
    {
    }

    constexpr FontVariantNumeric(Platform value) noexcept : _platform {value}
    {
    }

    KRYS_NODISCARD constexpr Platform GetPlatform() const noexcept
    {
      return _platform;
    }

    KRYS_NODISCARD constexpr bool IsNormal() const noexcept
    {
      return _platform.Figure == FontVariantNumericFigure::Normal
             && _platform.Spacing == FontVariantNumericSpacing::Normal
             && _platform.Fraction == FontVariantNumericFraction::Normal
             && _platform.Ordinal == FontVariantNumericOrdinal::Normal
             && _platform.SlashedZero == FontVariantNumericSlashedZero::Normal;
    }

    template <typename... F>
    decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      auto visitor = krys::CreateVisitor(std::forward<F>(f)...);

      if (IsNormal())
      {
        return visitor(css::keywords::Normal {});
      }

      auto figureValue =
        [](auto value) -> Maybe<Variant<css::keywords::LiningNums, css::keywords::OldstyleNums>>
      {
        using enum FontVariantNumericFigure;
        switch (value)
        {
          case Normal:          return std::nullopt;
          case LiningNumbers:   return css::keywords::LiningNums {};
          case OldStyleNumbers: return css::keywords::OldstyleNums {};
        }
        krys_unreachable();
      };
      auto spacingValue =
        [](auto value) -> Maybe<Variant<css::keywords::ProportionalNums, css::keywords::TabularNums>>
      {
        using enum FontVariantNumericSpacing;
        switch (value)
        {
          case Normal:              return std::nullopt;
          case ProportionalNumbers: return css::keywords::ProportionalNums {};
          case TabularNumbers:      return css::keywords::TabularNums {};
        }
        krys_unreachable();
      };
      auto fractionValue =
        [](auto value) -> Maybe<Variant<css::keywords::DiagonalFractions, css::keywords::StackedFractions>>
      {
        using enum FontVariantNumericFraction;
        switch (value)
        {
          case Normal:            return std::nullopt;
          case DiagonalFractions: return css::keywords::DiagonalFractions {};
          case StackedFractions:  return css::keywords::StackedFractions {};
        }
        krys_unreachable();
      };
      auto ordinalValue = [](auto value) -> Maybe<css::keywords::Ordinal>
      {
        using enum FontVariantNumericOrdinal;
        switch (value)
        {
          case Normal: return std::nullopt;
          case Yes:    return css::keywords::Ordinal {};
        }
        krys_unreachable();
      };
      auto slashedZeroValue = [](auto value) -> Maybe<css::keywords::SlashedZero>
      {
        using enum FontVariantNumericSlashedZero;
        switch (value)
        {
          case Normal: return std::nullopt;
          case Yes:    return css::keywords::SlashedZero {};
        }
        krys_unreachable();
      };

      return visitor(css::SpaceSeparatedTuple {
        figureValue(_platform.Figure),
        spacingValue(_platform.Spacing),
        fractionValue(_platform.Fraction),
        ordinalValue(_platform.Ordinal),
        slashedZeroValue(_platform.SlashedZero),
      });
    }

    KRYS_NODISCARD constexpr bool operator==(const FontVariantNumeric &) const noexcept = default;

  private:
    Platform _platform;
  };

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<FontVariantNumeric>
  {
    KRYS_NODISCARD FontVariantNumeric operator()(BuilderState &, const css::Value &) noexcept;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::FontVariantNumeric)
