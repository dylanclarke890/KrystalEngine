#include "Krystal.Booey/Style/Values/Color/ColorMix.hpp"
#include "Krystal.Booey/CSS/Values/Color/ColorMixResolver.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesSerialisation.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Graphics/Color/ColorSerialisation.hpp"
#include "Krystal.Booey/Style/Values/Color/ColorResolutionState.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesConversions.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::style
{
  using namespace css::Literals;

  // MARK: - Conversion

  Color ToStyleColor(const css::ColorMix &unresolved, ColorResolutionState &state) noexcept
  {
    ColorResolutionStateNester nester {state};

    auto component1Color = ToStyleColor(unresolved.MixComponents1.Color, state);
    auto component2Color = ToStyleColor(unresolved.MixComponents2.Color, state);

    auto percentage1 = ConvertToStyle(unresolved.MixComponents1.Percentage, state.ConversionData);
    auto percentage2 = ConvertToStyle(unresolved.MixComponents2.Percentage, state.ConversionData);

    if (!component1Color.IsResolvedColor() || !component2Color.IsResolvedColor())
    {
      // If the either component is not resolved, we cannot fully resolve the color
      // yet. Instead, we resolve the calc values using the conversion data, and return
      // a style::ColorMix to be resolved at use time.
      return Color {ColorMix {
        unresolved.ColorInterpolationMethod,
        ColorMix::Component {krys::move(component1Color), krys::move(percentage1)},
        ColorMix::Component {krys::move(component2Color), krys::move(percentage2)},
      }};
    }

    return css::Mix(css::ColorMixResolver {
      unresolved.ColorInterpolationMethod,
      css::ColorMixResolver::Component {component1Color.GetResolvedColor(), krys::move(percentage1)},
      css::ColorMixResolver::Component {component2Color.GetResolvedColor(), krys::move(percentage2)},
    });
  }

  // MARK: - Resolve

  boo::Color ResolveColor(const ColorMix &colorMix, const boo::Color &currentColor) noexcept
  {
    return css::Mix(css::ColorMixResolver {
      colorMix.ColorInterpolationMethod,
      css::ColorMixResolver::Component {colorMix.MixComponents1.Color.ResolveColor(currentColor),
                                        colorMix.MixComponents1.Percentage},
      css::ColorMixResolver::Component {colorMix.MixComponents2.Color.ResolveColor(currentColor),
                                        colorMix.MixComponents2.Percentage},
    });
  }

  // MARK: - Current Color

  bool ContainsCurrentColor(const ColorMix &colorMix) noexcept
  {
    return style::ContainsCurrentColor(colorMix.MixComponents1.Color)
           || style::ContainsCurrentColor(colorMix.MixComponents2.Color);
  }

  // MARK: - Serialisation

  namespace
  {
    KRYS_NODISCARD bool SumTo100Percent(const ColorMix::Component::PercentageType &a,
                                        const ColorMix::Component::PercentageType &b) noexcept
    {
      return a.Value + b.Value == 100.0;
    }

    KRYS_NODISCARD Maybe<ColorMix::Component::PercentageType>
      SubtractFrom100Percent(const ColorMix::Component::PercentageType &percentage) noexcept
    {
      return ColorMix::Component::PercentageType {100.0 - percentage.Value};
    }

    void SerialiseColorMixPercentage(StringBuilder &builder, const css::SerialisationContext &context,
                                     const ColorMix::Component::PercentageType &percentage) noexcept
    {
      css::Serialise(builder, context, css::PercentageRaw<> {percentage.Value});
    }

    void SerialisationForColorMixPercentage1(StringBuilder &builder, const css::SerialisationContext &context,
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
        SerialiseColorMixPercentage(builder, context, *value.MixComponents1.Percentage);
      }
      else if (value.MixComponents1.Percentage)
      {
        if (*value.MixComponents1.Percentage == 50_css_percentage)
        {
          return;
        }
        builder.Append(u8' ');
        SerialiseColorMixPercentage(builder, context, *value.MixComponents1.Percentage);
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
        SerialiseColorMixPercentage(builder, context, *subtractedPercent);
      }
    }

    void SerialisationForColorMixPercentage2(StringBuilder &builder, const css::SerialisationContext &context,
                                             const ColorMix &value) noexcept
    {
      if (value.MixComponents1.Percentage && value.MixComponents2.Percentage)
      {
        if (SumTo100Percent(*value.MixComponents1.Percentage, *value.MixComponents2.Percentage))
        {
          return;
        }

        builder.Append(u8' ');
        SerialiseColorMixPercentage(builder, context, *value.MixComponents2.Percentage);
      }
    }
  }

  void SerialisationForCSSTokenization(StringBuilder &builder, const css::SerialisationContext &context,
                                       const ColorMix &colorMix) noexcept
  {
    builder.Append(u8"color-mix(");

    if (colorMix.ColorInterpolationMethod != css::DefaultInterpolationMethodForColorMix)
    {
      builder.Append(u8"in ");
      boo::SerialisationForCSS(builder, colorMix.ColorInterpolationMethod);
      builder.Append(u8", ");
    }

    SerialisationForCSSTokenization(builder, context, colorMix.MixComponents1.Color);
    SerialisationForColorMixPercentage1(builder, context, colorMix);
    builder.Append(u8", ");
    SerialisationForCSSTokenization(builder, context, colorMix.MixComponents2.Color);
    SerialisationForColorMixPercentage2(builder, context, colorMix);
    builder.Append(u8')');
  }

  css::CSSOMString SerialisationForCSSTokenization(const css::SerialisationContext &context,
                                                   const ColorMix &colorMix) noexcept
  {
    StringBuilder builder;
    SerialisationForCSSTokenization(builder, context, colorMix);
    return krys::move(builder).ToString();
  }
}
