#pragma once

#include "Krystal.Lib/Color/AlphaPremultiplication.hpp"
#include "Krystal.Lib/Color/ColorInterpolationMethod.hpp"
#include "Krystal.Lib/Color/ColorTypes.hpp"
#include "Krystal.Lib/Color/ColorUtilities.hpp"

namespace Krys
{
  // Provides support for premulitplied and unpremultiplied operations on two color
  // operands. https://www.w3.org/TR/css-values-4/#combine-colors

  class Color;

  template <AlphaPremultiplication, typename InterpolationMethodColorSpace>
  typename InterpolationMethodColorSpace::ColorType InterpolateColorComponents(
    InterpolationMethodColorSpace, typename InterpolationMethodColorSpace::ColorType color1,
    double color1Multiplier, typename InterpolationMethodColorSpace::ColorType color2,
    double color2Multiplier) noexcept;

  Color InterpolateColors(ColorInterpolationMethod, Color color1, double color1Multiplier, Color color2,
                          double color2Multiplier) noexcept;

  template <AlphaPremultiplication, typename InterpolationMethodColorSpace>
  typename InterpolationMethodColorSpace::ColorType
    AddColorComponents(InterpolationMethodColorSpace,
                       typename InterpolationMethodColorSpace::ColorType color1,
                       typename InterpolationMethodColorSpace::ColorType color2) noexcept;

  // MARK: - Pre-interpolation normalization/fixup.

  std::pair<float, float> FixupHueComponentsPriorToInterpolation(HueInterpolationMethod, float component1,
                                                                 float component2) noexcept;

  // MARK: - Premultiplication-agnostic interpolation helpers.

  inline float InterpolateComponentWithoutAccountingForNaN(float componentFromColor1, double color1Multiplier,
                                                           float componentFromColor2, double color2Multiplier) noexcept
  {
    return static_cast<float>((componentFromColor1 * color1Multiplier) + (componentFromColor2 * color2Multiplier));
  }

  inline float InterpolateComponentAccountingForNaN(float componentFromColor1, double color1Multiplier,
                                                    float componentFromColor2, double color2Multiplier) noexcept
  {
    if (std::isnan(componentFromColor1))
    {
      return componentFromColor2;
    }

    if (std::isnan(componentFromColor2))
    {
      return componentFromColor1;
    }

    return InterpolateComponentWithoutAccountingForNaN(componentFromColor1, color1Multiplier,
                                                       componentFromColor2, color2Multiplier);
  }

  template <typename InterpolationMethodColorSpace>
  float InterpolateHue(InterpolationMethodColorSpace interpolationMethodColorSpace, float componentFromColor1,
                       double color1Multiplier, float componentFromColor2, double color2Multiplier) noexcept
  {
    if (std::isnan(componentFromColor1))
    {
      return componentFromColor2;
    }

    if (std::isnan(componentFromColor2))
    {
      return componentFromColor1;
    }

    auto [fixedupComponent1, fixedupComponent2] = FixupHueComponentsPriorToInterpolation(
      interpolationMethodColorSpace.hueInterpolationMethod, componentFromColor1, componentFromColor2);
    return InterpolateComponentWithoutAccountingForNaN(fixedupComponent1, color1Multiplier, fixedupComponent2,
                                                       color2Multiplier);
  }

  // MARK: - Premultiplied interpolation.

  struct PremultipliedAlphaState
  {
    float alphaForPremultiplicationOfColor1;
    float alphaForPremultiplicationOfColor2;
    float alphaForUnpremultiplication;
    float resultAlpha;
  };

  inline PremultipliedAlphaState InterpolateAlphaPremulitplied(float alphaForColor1, double color1Multiplier,
                                                               float alphaForColor2, double color2Multiplier) noexcept
  {
    // If both alpha channels are none/missing, no premultiplication is performed and the resulting color will
    // have a none/missing alpha channel. If only one alpha channels is none/missing, the other alpha channel
    // is used premultiplication of both colors and is the resulting color's alpha channel. If neither alpha
    // channel is none/missing, each alpha channel is used for the premultiplication of its associated color
    // and the interpolated result of the two alpha channels is the resulting color's alpha channel.

    if (std::isnan(alphaForColor1))
    {
      if (std::isnan(alphaForColor2))
      {
        return {1.0f, 1.0f, 1.0f, std::numeric_limits<float>::quiet_NaN()};
      }

      return {alphaForColor2, alphaForColor2, alphaForColor2, alphaForColor2};
    }

    if (std::isnan(alphaForColor2))
    {
      return {alphaForColor1, alphaForColor1, alphaForColor1, alphaForColor1};
    }

    auto interpolatedAlpha = std::clamp(InterpolateComponentWithoutAccountingForNaN(
                                          alphaForColor1, color1Multiplier, alphaForColor2, color2Multiplier),
                                        0.0f, 1.0f);
    return {alphaForColor1, alphaForColor2, interpolatedAlpha, interpolatedAlpha};
  }

  template <size_t I, typename InterpolationMethodColorSpace>
  float InterpolateComponentUsingPremultipliedAlpha(
    InterpolationMethodColorSpace interpolationMethodColorSpace, ColorComponents<float, 4> colorComponents1,
    double color1Multiplier, ColorComponents<float, 4> colorComponents2, double color2Multiplier,
    PremultipliedAlphaState interpolatedAlpha) noexcept
  {
    using ColorType = typename InterpolationMethodColorSpace::ColorType;
    constexpr auto componentInfo = ColorType::Model::componentInfo;

    if constexpr (componentInfo[I].type == ColorComponentType::Angle)
    {
      return InterpolateHue(interpolationMethodColorSpace, colorComponents1[I], color1Multiplier,
                            colorComponents2[I], color2Multiplier);
    }
    else
    {
      if (std::isnan(colorComponents1[I]))
      {
        return colorComponents2[I];
      }

      if (std::isnan(colorComponents2[I]))
      {
        return colorComponents1[I];
      }

      auto premultipliedComponent1 =
        colorComponents1[I] * interpolatedAlpha.alphaForPremultiplicationOfColor1;
      auto premultipliedComponent2 =
        colorComponents2[I] * interpolatedAlpha.alphaForPremultiplicationOfColor2;

      auto premultipliedResult = InterpolateComponentWithoutAccountingForNaN(
        premultipliedComponent1, color1Multiplier, premultipliedComponent2, color2Multiplier);

      if (interpolatedAlpha.alphaForUnpremultiplication == 0.0f)
      {
        return premultipliedResult;
      }

      return premultipliedResult / interpolatedAlpha.alphaForUnpremultiplication;
    }
  }

  // MARK: - Unpremultiplied interpolation.

  inline float InterpolateAlphaUnpremulitplied(float alphaForColor1, double color1Multiplier,
                                               float alphaForColor2, double color2Multiplier) noexcept
  {
    return InterpolateComponentAccountingForNaN(alphaForColor1, color1Multiplier, alphaForColor2,
                                                color2Multiplier);
  }

  template <size_t I, typename InterpolationMethodColorSpace>
  float InterpolateComponentUsingUnpremultipliedAlpha(
    InterpolationMethodColorSpace interpolationMethodColorSpace, ColorComponents<float, 4> colorComponents1,
    double color1Multiplier, ColorComponents<float, 4> colorComponents2, double color2Multiplier) noexcept
  {
    using ColorType = typename InterpolationMethodColorSpace::ColorType;
    constexpr auto componentInfo = ColorType::Model::componentInfo;

    if constexpr (componentInfo[I].type == ColorComponentType::Angle)
    {
      return InterpolateHue(interpolationMethodColorSpace, colorComponents1[I], color1Multiplier,
                            colorComponents2[I], color2Multiplier);
    }
    else
    {
      return InterpolateComponentAccountingForNaN(colorComponents1[I], color1Multiplier, colorComponents2[I],
                                                  color2Multiplier);
    }
  }

  // MARK: - Interpolation.

  template <AlphaPremultiplication alphaPremultiplication, typename InterpolationMethodColorSpace>
  typename InterpolationMethodColorSpace::ColorType InterpolateColorComponents(
    InterpolationMethodColorSpace interpolationMethodColorSpace,
    typename InterpolationMethodColorSpace::ColorType color1, double color1Multiplier,
    typename InterpolationMethodColorSpace::ColorType color2, double color2Multiplier) noexcept
  {
    auto colorComponents1 = AsColorComponents(color1.Unresolved());
    auto colorComponents2 = AsColorComponents(color2.Unresolved());

    if constexpr (alphaPremultiplication == AlphaPremultiplication::Premultiplied)
    {
      auto interpolatedAlpha = InterpolateAlphaPremulitplied(colorComponents1[3], color1Multiplier,
                                                             colorComponents2[3], color2Multiplier);
      auto interpolatedComponent1 = InterpolateComponentUsingPremultipliedAlpha<0>(
        interpolationMethodColorSpace, colorComponents1, color1Multiplier, colorComponents2, color2Multiplier,
        interpolatedAlpha);
      auto interpolatedComponent2 = InterpolateComponentUsingPremultipliedAlpha<1>(
        interpolationMethodColorSpace, colorComponents1, color1Multiplier, colorComponents2, color2Multiplier,
        interpolatedAlpha);
      auto interpolatedComponent3 = InterpolateComponentUsingPremultipliedAlpha<2>(
        interpolationMethodColorSpace, colorComponents1, color1Multiplier, colorComponents2, color2Multiplier,
        interpolatedAlpha);

      return CreateColorTypeByNormalizingComponents<typename InterpolationMethodColorSpace::ColorType>(
        {interpolatedComponent1, interpolatedComponent2, interpolatedComponent3,
         interpolatedAlpha.resultAlpha});
    }
    else
    {
      auto interpolatedAlpha = InterpolateAlphaUnpremulitplied(colorComponents1[3], color1Multiplier,
                                                               colorComponents2[3], color2Multiplier);
      auto interpolatedComponent1 = InterpolateComponentUsingUnpremultipliedAlpha<0>(
        interpolationMethodColorSpace, colorComponents1, color1Multiplier, colorComponents2,
        color2Multiplier);
      auto interpolatedComponent2 = InterpolateComponentUsingUnpremultipliedAlpha<1>(
        interpolationMethodColorSpace, colorComponents1, color1Multiplier, colorComponents2,
        color2Multiplier);
      auto interpolatedComponent3 = InterpolateComponentUsingUnpremultipliedAlpha<2>(
        interpolationMethodColorSpace, colorComponents1, color1Multiplier, colorComponents2,
        color2Multiplier);

      return CreateColorTypeByNormalizingComponents<typename InterpolationMethodColorSpace::ColorType>(
        {interpolatedComponent1, interpolatedComponent2, interpolatedComponent3, interpolatedAlpha});
    }
  }

  // MARK: - Addition.

  template <AlphaPremultiplication alphaPremultiplication, typename InterpolationMethodColorSpace>
  typename InterpolationMethodColorSpace::ColorType
    AddColorComponents(InterpolationMethodColorSpace interpolationMethodColorSpace,
                       typename InterpolationMethodColorSpace::ColorType color1,
                       typename InterpolationMethodColorSpace::ColorType color2) noexcept
  {
    // Color addition can utilize the existing interpolation infrastructure by
    // combining two colors at 100% strength each.
    return InterpolateColorComponents<alphaPremultiplication>(interpolationMethodColorSpace, color1, 1.0,
                                                              color2, 1.0);
  }
}