#include "Krystal.Booey/CSS/Values/Color/ColorMix.hpp"
#include "Krystal.Booey/CSS/Values/Color/ColorMixResolver.hpp"
#include "Krystal.Booey/CSS/Values/Color/PlatformColorResolutionState.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesComputedStyleDependencies.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesSerialisation.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesValueVisitation.hpp"
#include "Krystal.Booey/Graphics/Color/ColorSerialisation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesConversions.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::css
{
  boo::Color CreateColor(const ColorMix &unresolved, PlatformColorResolutionState &state) noexcept
  {
    PlatformColorResolutionStateNester nester {state};

    auto component1Color = CreateColor(unresolved.MixComponents1.Color, state);
    if (!component1Color.IsValid())
    {
      return {};
    }

    auto component2Color = CreateColor(unresolved.MixComponents2.Color, state);
    if (!component2Color.IsValid())
    {
      return {};
    }

    Maybe<style::Percentage<Range {0, 100}>> percentage1;
    Maybe<style::Percentage<Range {0, 100}>> percentage2;
    if (RequiresConversionData(unresolved.MixComponents1.Percentage)
        || RequiresConversionData(unresolved.MixComponents2.Percentage))
    {
      if (!state.ConversionData)
      {
        return {};
      }

      percentage1 = style::ConvertToStyle(unresolved.MixComponents1.Percentage, *state.ConversionData);
      percentage2 = style::ConvertToStyle(unresolved.MixComponents2.Percentage, *state.ConversionData);
    }
    else
    {
      percentage1 = style::ConvertToStyleNoConversionDataRequired(unresolved.MixComponents1.Percentage);
      percentage2 = style::ConvertToStyleNoConversionDataRequired(unresolved.MixComponents2.Percentage);
    }

    return Mix(ColorMixResolver {
      unresolved.ColorInterpolationMethod,
      ColorMixResolver::Component {krys::move(component1Color), krys::move(percentage1)},
      ColorMixResolver::Component {krys::move(component2Color), krys::move(percentage2)},
    });
  }

  bool ContainsCurrentColor(const ColorMix &unresolved) noexcept
  {
    return ContainsCurrentColor(unresolved.MixComponents1.Color)
           || ContainsCurrentColor(unresolved.MixComponents2.Color);
  }

  bool ContainsColorSchemeDependentColor(const ColorMix &unresolved) noexcept
  {
    return ContainsColorSchemeDependentColor(unresolved.MixComponents1.Color)
           || ContainsColorSchemeDependentColor(unresolved.MixComponents2.Color);
  }

  namespace
  {
    KRYS_NODISCARD bool SumTo100Percent(const ColorMix::Component::PercentageType &a,
                                        const ColorMix::Component::PercentageType &b) noexcept
    {
      return a.IsRaw() && b.IsRaw() && a.GetRaw()->Value + b.GetRaw()->Value == 100.0;
    }

    KRYS_NODISCARD Maybe<PercentageRaw<>>
      SubtractFrom100Percent(const ColorMix::Component::PercentageType &percentage) noexcept
    {
      return percentage.IsRaw() ? std::make_optional(PercentageRaw<> {100.0 - percentage.GetRaw()->Value})
                                : null;
    }

    void SerialiseColorMixPercentage1(StringBuilder &builder, const SerialisationContext &context,
                                      const ColorMix &value) noexcept
    {
      if (value.MixComponents1.Percentage && value.MixComponents2.Percentage)
      {
        if (*value.MixComponents1.Percentage == 50_css_percentage
            && *value.MixComponents2.Percentage == 50_css_percentage)
        {
          return;
        }
        builder.Append(u8' ');
        Serialise(builder, context, *value.MixComponents1.Percentage);
      }
      else if (value.MixComponents1.Percentage)
      {
        if (*value.MixComponents1.Percentage == 50_css_percentage)
        {
          return;
        }
        builder.Append(u8' ');
        Serialise(builder, context, *value.MixComponents1.Percentage);
      }
      else if (value.MixComponents2.Percentage)
      {
        if (*value.MixComponents2.Percentage == 50_css_percentage)
        {
          return;
        }

        auto subtractedPercent = SubtractFrom100Percent(*value.MixComponents2.Percentage);
        if (!subtractedPercent)
        {
          return;
        }

        builder.Append(u8' ');
        Serialise(builder, context, *subtractedPercent);
      }
    }

    void SerialiseColorMixPercentage2(StringBuilder &builder, const SerialisationContext &context,
                                      const ColorMix &value) noexcept
    {
      if (value.MixComponents1.Percentage && value.MixComponents2.Percentage)
      {
        if (SumTo100Percent(*value.MixComponents1.Percentage, *value.MixComponents2.Percentage))
        {
          return;
        }

        builder.Append(u8' ');
        Serialise(builder, context, *value.MixComponents2.Percentage);
      }
      else if (value.MixComponents2.Percentage)
      {
        if (*value.MixComponents2.Percentage == 50_css_percentage)
        {
          return;
        }
        if (!value.MixComponents2.Percentage->IsCalc())
        {
          return;
        }

        builder.Append(u8' ');
        Serialise(builder, context, *value.MixComponents2.Percentage);
      }
    }
  }

  void Serialiser<ColorMix>::operator()(StringBuilder &builder, const SerialisationContext &context,
                                        const ColorMix &value) noexcept
  {
    builder.Append(u8"color-mix(");
    if (value.ColorInterpolationMethod != DefaultInterpolationMethodForColorMix)
    {
      builder.Append(u8"in ");
      boo::SerialisationForCSS(builder, value.ColorInterpolationMethod);
      builder.Append(u8", ");
    }

    Serialise(builder, context, value.MixComponents1.Color);
    SerialiseColorMixPercentage1(builder, context, value);
    builder.Append(u8", ");
    Serialise(builder, context, value.MixComponents2.Color);
    SerialiseColorMixPercentage2(builder, context, value);
    builder.Append(u8')');
  }

  void ComputedStyleDependenciesCollector<ColorMix>::operator()(ComputedStyleDependencies &dependencies,
                                                                const ColorMix &value) noexcept
  {
    CollectComputedStyleDependencies(dependencies, value.MixComponents1.Color);
    CollectComputedStyleDependencies(dependencies, value.MixComponents1.Percentage);
    CollectComputedStyleDependencies(dependencies, value.MixComponents2.Color);
    CollectComputedStyleDependencies(dependencies, value.MixComponents2.Percentage);
  }

  IterationStatus
    ValueChildrenVisitor<ColorMix>::operator()(KRYS_NOESCAPE const Func<IterationStatus(Value &)> &func,
                                               const ColorMix &value) noexcept
  {
    if (VisitValueChildren(func, value.MixComponents1.Color) == IterationStatus::Done)
    {
      return IterationStatus::Done;
    }
    if (VisitValueChildren(func, value.MixComponents1.Percentage) == IterationStatus::Done)
    {
      return IterationStatus::Done;
    }
    if (VisitValueChildren(func, value.MixComponents2.Color) == IterationStatus::Done)
    {
      return IterationStatus::Done;
    }
    if (VisitValueChildren(func, value.MixComponents2.Percentage) == IterationStatus::Done)
    {
      return IterationStatus::Done;
    }
    return IterationStatus::Continue;
  }
}
