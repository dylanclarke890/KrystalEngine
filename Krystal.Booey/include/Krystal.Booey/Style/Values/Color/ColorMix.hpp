#pragma once

#include "Krystal.Booey/CSS/Values/Color/ColorMix.hpp"
#include "Krystal.Booey/Graphics/Color/ColorInterpolationMethod.hpp"
#include "Krystal.Booey/Style/Values/Color/Color.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypes.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::style
{
  struct ColorResolutionState;

  /// @brief A `color-mix()` that couldn't be resolved during style building because a component depends on
  /// `currentcolor`.
  struct ColorMix
  {
    struct Component
    {
      using PercentageType = style::Percentage<css::Range {0, 100}>;

      style::Color Color;
      Maybe<PercentageType> Percentage;

      KRYS_NODISCARD bool operator==(const Component &) const noexcept = default;
    };

    boo::ColorInterpolationMethod ColorInterpolationMethod;
    Component MixComponents1;
    Component MixComponents2;

    KRYS_NODISCARD bool operator==(const ColorMix &) const noexcept = default;
  };

  KRYS_NODISCARD Color ToStyleColor(const css::ColorMix &unresolved, ColorResolutionState &state) noexcept;
  KRYS_NODISCARD boo::Color ResolveColor(const ColorMix &colorMix, const boo::Color &currentColor) noexcept;
  KRYS_NODISCARD bool ContainsCurrentColor(const ColorMix &colorMix) noexcept;

  void SerialisationForCSSTokenization(StringBuilder &builder, const css::SerialisationContext &context,
                                       const ColorMix &value) noexcept;
  KRYS_NODISCARD css::CSSOMString SerialisationForCSSTokenization(const css::SerialisationContext &context,
                                                                  const ColorMix &value) noexcept;
}
