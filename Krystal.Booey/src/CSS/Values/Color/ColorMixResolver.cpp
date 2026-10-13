#include "Krystal.Booey/CSS/Values/Color/ColorMixResolver.hpp"
#include "Krystal.Booey/Graphics/Color/ColorInterpolation.hpp"

namespace krys::boo::css
{
  namespace
  {
    struct ColorMixPercentages
    {
      double P1;
      double P2;
      Maybe<double> AlphaMultiplier {};
    };

    KRYS_NODISCARD Maybe<ColorMixPercentages> NormalizedMixPercentages(
      Maybe<ColorMixResolver::Component::PercentageType> mixComponents1Percentage,
      Maybe<ColorMixResolver::Component::PercentageType> mixComponents2Percentage) noexcept
    {
      // The percentages are normalized as follows:

      // 1. Let p1 be the first percentage and p2 the second one.

      // 2. If both percentages are omitted, they each default to 50% (an equal mix of the two colors).
      if (!mixComponents1Percentage && !mixComponents2Percentage)
      {
        return ColorMixPercentages {50.0, 50.0};
      }

      ColorMixPercentages result;

      if (!mixComponents2Percentage)
      {
        // 3. Otherwise, if p2 is omitted, it becomes 100% - p1
        result.P1 = mixComponents1Percentage->Value;
        result.P2 = 100.0 - result.P1;
      }
      else if (!mixComponents1Percentage)
      {
        // 4. Otherwise, if p1 is omitted, it becomes 100% - p2
        result.P2 = mixComponents2Percentage->Value;
        result.P1 = 100.0 - result.P2;
      }
      else
      {
        result.P1 = mixComponents1Percentage->Value;
        result.P2 = mixComponents2Percentage->Value;
      }

      auto sum = result.P1 + result.P2;

      // 5.If the percentages sum to zero, the function is invalid.
      if (sum == 0)
      {
        return null;
      }

      if (sum > 100.0)
      {
        // 6. Otherwise, if both are provided but do not add up to 100%, they are scaled accordingly so that
        // they
        //    add up to 100%.
        result.P1 *= 100.0 / sum;
        result.P2 *= 100.0 / sum;
      }
      else if (sum < 100.0)
      {
        // 7. Otherwise, if both are provided and add up to less than 100%, the sum is saved as an alpha
        //    multiplier. They are then scaled accordingly so that they add up to 100%.
        result.P1 *= 100.0 / sum;
        result.P2 *= 100.0 / sum;
        result.AlphaMultiplier = sum;
      }

      return result;
    }

    template <typename InterpolationMethod>
    KRYS_NODISCARD boo::Color MixColorComponentsUsingColorInterpolationMethod(
      InterpolationMethod interpolationMethod, ColorMixPercentages mixPercentages, const boo::Color &color1,
      const boo::Color &color2) noexcept
    {
      using ColorType = typename InterpolationMethod::ColorType;

      // 1. Both colors are converted to the specified <color-space>. If the specified color space has a
      // smaller
      //    gamut than the one in which the color to be adjusted is specified, gamut mapping will occur.
      auto convertedColor1 = color1.template ToColorTypeLossyCarryingForwardMissing<ColorType>();
      auto convertedColor2 = color2.template ToColorTypeLossyCarryingForwardMissing<ColorType>();

      // 2. Colors are then interpolated in the specified color space, as described in CSS Color 4 § 13
      //    Interpolation. [...]
      auto mixedColor = InterpolateColorComponents<AlphaPremultiplication::Premultiplied>(
                          interpolationMethod, convertedColor1, mixPercentages.P1 / 100.0, convertedColor2,
                          mixPercentages.P2 / 100.0)
                          .Unresolved();

      // 3. If an alpha multiplier was produced during percentage normalization, the alpha component of the
      //    interpolated result is multiplied by the alpha multiplier.
      if (mixPercentages.AlphaMultiplier && !std::isnan(mixedColor.Alpha))
      {
        mixedColor.Alpha *= static_cast<float>(*mixPercentages.AlphaMultiplier / 100.0);
      }

      auto flags = ColorFlags::UseColorFunctionSerialisation;
      if (color1.IsSemantic() || color2.IsSemantic())
      {
        flags = flags | ColorFlags::Semantic;
      }

      return {mixedColor, flags};
    }
  }

  boo::Color Mix(const ColorMixResolver &colorMix) noexcept
  {
    auto mixPercentages =
      NormalizedMixPercentages(colorMix.MixComponents1.Percentage, colorMix.MixComponents2.Percentage);
    if (!mixPercentages)
    {
      return {};
    }

    return krys::SwitchOn(colorMix.ColorInterpolationMethod.ColorSpace,
                          [&](const auto &methodColorSpace)
                          {
                            return MixColorComponentsUsingColorInterpolationMethod(
                              methodColorSpace, *mixPercentages, colorMix.MixComponents1.Color,
                              colorMix.MixComponents2.Color);
                          });
  }
}
