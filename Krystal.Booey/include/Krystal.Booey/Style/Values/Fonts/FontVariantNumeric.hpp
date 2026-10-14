#pragma once

#include "Krystal.Booey/Core/TextFlags.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"

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
