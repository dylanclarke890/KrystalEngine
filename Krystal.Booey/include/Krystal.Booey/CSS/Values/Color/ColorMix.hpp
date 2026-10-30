#pragma once

#include "Krystal.Booey/CSS/Values/Color/Color.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypes.hpp"
#include "Krystal.Booey/Graphics/Color/ColorInterpolationMethod.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypes.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::css
{
  struct PlatformColorResolutionState;

  struct ColorMix
  {
    struct Component
    {
      using PercentageType = css::Percentage<Range {0, 100}>;

      css::Color Color;
      Maybe<PercentageType> Percentage;

      KRYS_NODISCARD bool operator==(const Component &) const noexcept = default;
    };

    krys::ColourInterpolationMethod ColourInterpolationMethod;
    Component MixComponents1;
    Component MixComponents2;

    KRYS_NODISCARD bool operator==(const ColorMix &) const noexcept = default;
  };

  constexpr auto DefaultInterpolationMethodForColorMix =
    ColourInterpolationMethod {ColourInterpolationMethod::OKLab {}, AlphaPremultiplication::Premultiplied};

  KRYS_NODISCARD boo::Color CreateColor(const ColorMix &unresolved,
                                        PlatformColorResolutionState &state) noexcept;
  KRYS_NODISCARD bool ContainsCurrentColor(const ColorMix &unresolved) noexcept;
  KRYS_NODISCARD bool ContainsColorSchemeDependentColor(const ColorMix &unresolved) noexcept;

  template <>
  struct Serialiser<ColorMix>
  {
    void operator()(StringBuilder &builder, const SerialisationContext &context,
                    const ColorMix &value) noexcept;
  };

  template <>
  struct ComputedStyleDependenciesCollector<ColorMix>
  {
    void operator()(ComputedStyleDependencies &dependencies, const ColorMix &value) noexcept;
  };

  template <>
  struct ValueChildrenVisitor<ColorMix>
  {
    IterationStatus operator()(KRYS_NOESCAPE const Func<IterationStatus(Value &)> &func,
                               const ColorMix &value) noexcept;
  };
}
