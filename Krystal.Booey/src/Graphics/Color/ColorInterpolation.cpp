#include "Krystal.Booey/Graphics/Color/ColorInterpolation.hpp"
#include "Krystal.Booey/Graphics/Color/Color.hpp"
#include "Krystal.Core/Visitor.hpp"

namespace krys::boo
{
  std::pair<float, float> FixupHueComponentsPriorToInterpolation(HueInterpolationMethod method,
                                                                 float component1, float component2) noexcept
  {
    auto NormalizeAnglesUsingShorterAlgorithm = [](auto theta1, auto theta2) -> std::pair<float, float>
    {
      // https://drafts.csswg.org/css-color-4/#hue-shorter
      auto difference = theta2 - theta1;
      if (difference > 180.f)
        return {theta1 + 360.f, theta2};
      if (difference < -180.f)
        return {theta1, theta2 + 360.f};
      return {theta1, theta2};
    };

    auto NormalizeAnglesUsingLongerAlgorithm = [](auto theta1, auto theta2) -> std::pair<float, float>
    {
      // https://drafts.csswg.org/css-color-4/#hue-longer
      auto difference = theta2 - theta1;
      if (difference > 0 && difference < 180.f)
      {
        return {theta1 + 360.f, theta2};
      }

      if (difference > -180.f && difference <= 0)
      {
        return {theta1, theta2 + 360.f};
      }

      return {theta1, theta2};
    };

    auto NormalizeAnglesUsingIncreasingAlgorithm = [](auto theta1, auto theta2) -> std::pair<float, float>
    {
      // https://drafts.csswg.org/css-color-4/#hue-increasing
      if (theta2 < theta1)
      {
        return {theta1, theta2 + 360.f};
      }

      return {theta1, theta2};
    };

    auto NormalizeAnglesUsingDecreasingAlgorithm = [](auto theta1, auto theta2) -> std::pair<float, float>
    {
      // https://drafts.csswg.org/css-color-4/#hue-decreasing
      if (theta1 < theta2)
      {
        return {theta1 + 360.f, theta2};
      }

      return {theta1, theta2};
    };

    // https://www.w3.org/TR/css-color-4/#hue-interpolation
    //    "Both angles need to be constrained to [0, 360] prior to interpolation.
    //     One way to do this is θ = ((θ % 360) + 360) % 360."

    switch (method)
    {
      case HueInterpolationMethod::Shorter:
      {
        return NormalizeAnglesUsingShorterAlgorithm(NormalizeHue(component1), NormalizeHue(component2));
      }
      case HueInterpolationMethod::Longer:
      {
        return NormalizeAnglesUsingLongerAlgorithm(NormalizeHue(component1), NormalizeHue(component2));
      }
      case HueInterpolationMethod::Increasing:
      {
        return NormalizeAnglesUsingIncreasingAlgorithm(NormalizeHue(component1), NormalizeHue(component2));
      }
      case HueInterpolationMethod::Decreasing:
      {
        return NormalizeAnglesUsingDecreasingAlgorithm(NormalizeHue(component1), NormalizeHue(component2));
      }
    }

    std::unreachable();
  }

  Color InterpolateColors(ColorInterpolationMethod colorInterpolationMethod, Color color1,
                          double color1Multiplier, Color color2, double color2Multiplier) noexcept
  {
    return krys::SwitchOn(
      colorInterpolationMethod.colorSpace,
      [&]<typename MethodColorSpace>(const MethodColorSpace &colorSpace) -> Color
      {
        using ColorType = typename MethodColorSpace::ColorType;
        switch (colorInterpolationMethod.alphaPremultiplication)
        {
          case AlphaPremultiplication::Premultiplied:
          {
            return InterpolateColorComponents<AlphaPremultiplication::Premultiplied>(
              colorSpace, color1.ToColorTypeLossyCarryingForwardMissing<ColorType>(), color1Multiplier,
              color2.ToColorTypeLossyCarryingForwardMissing<ColorType>(), color2Multiplier);
          }
          case AlphaPremultiplication::Unpremultiplied:
          {
            return InterpolateColorComponents<AlphaPremultiplication::Unpremultiplied>(
              colorSpace, color1.ToColorTypeLossyCarryingForwardMissing<ColorType>(), color1Multiplier,
              color2.ToColorTypeLossyCarryingForwardMissing<ColorType>(), color2Multiplier);
          }
        }

        std::unreachable();
      });
  }
}