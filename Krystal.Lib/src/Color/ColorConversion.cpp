#include "Krystal.Lib/Color/ColorConversion.hpp"
#include "Krystal.Lib/Color/Color.hpp"
#include "Krystal.Lib/Color/ColorSpace.hpp"
#include "Krystal.Lib/Color/ColorUtilities.hpp"
#include "Krystal.Lib/Color/DestinationColorSpace.hpp"
#include <numeric>

namespace Krys
{
  // MARK: Lab-Like to LCH-Like conversion utilities.

  template <typename LCHLike, typename LabLike>
  KRYS_NODISCARD LCHLike ConvertToPolarForm(const LabLike &color) noexcept
  {
    // https://drafts.csswg.org/css-color/#lab-to-lch
    auto [lightness, a, b, alpha] = color.Resolved();

    constexpr float epsilon = static_cast<float>(LabLike::Model::achromaticEpsilon);

    bool achromatic = std::abs(a) < epsilon && std::abs(b) < epsilon;

    if (achromatic)
    {
      return {lightness, 0, std::numeric_limits<float>::quiet_NaN(), alpha};
    }

    float chroma = std::hypot(a, b);
    float hue = rad2deg(atan2(b, a));

    return {lightness, chroma, hue >= 0.0f ? hue : hue + 360.0f, alpha};
  }

  template <typename LabLike, typename LCHLike>
  KRYS_NODISCARD LabLike convertToRectangularForm(const LCHLike &color) noexcept
  {
    // https://drafts.csswg.org/css-color/#lch-to-lab
    auto [lightness, chroma, hue, alpha] = color.Resolved();

    if (std::isnan(color.Unresolved().hue))
    {
      return {lightness, 0, 0, alpha};
    }

    float hueAngleRadians = deg2rad(hue);
    float a = chroma * std::cos(hueAngleRadians);
    float b = chroma * std::sin(hueAngleRadians);

    return {lightness, a, b, alpha};
  }

  // MARK: HSL conversions.

  HSLA<float>
    ColorConversion<HSLA<float>, ExtendedSRGBA<float>>::Convert(const ExtendedSRGBA<float> &color) noexcept
  {
    // https://drafts.csswg.org/css-color-4/#hsl-to-rgb

    auto [red, green, blue, alpha] = color.Resolved();

    auto [min, max] = std::minmax({red, green, blue});
    auto d = max - min;

    float hue = std::numeric_limits<float>::quiet_NaN();
    float lightness = std::midpoint(min, max);
    float saturation;

    if (d != 0.0f)
    {
      if (lightness == 0.0f || lightness == 1.0f)
      {
        saturation = 0.0f;
      }
      else
      {
        saturation = (max - lightness) / std::min(lightness, 1.0f - lightness);
      }

      if (max == red)
      {
        hue = ((green - blue) / d) + (green < blue ? 6.0f : 0.0f);
      }
      else if (max == green)
      {
        hue = ((blue - red) / d) + 2.0f;
      }
      else if (max == blue)
      {
        hue = ((red - green) / d) + 4.0f;
      }

      hue *= 60.0f;

      if (saturation < 0.0f)
      {
        hue += 180.0f;
        saturation = std::abs(saturation);
      }

      if (hue >= 360.0f)
      {
        hue -= 360.0f;
      }
    }
    else
    {
      saturation = 0.0f;
    }

    return {hue, saturation * 100.0f, lightness * 100.0f, alpha};
  }

  ExtendedSRGBA<float>
    ColorConversion<ExtendedSRGBA<float>, HSLA<float>>::Convert(const HSLA<float> &color) noexcept
  {
    // https://drafts.csswg.org/css-color-4/#hsl-to-rgb

    auto [hue, saturation, lightness, alpha] = color.Resolved();

    float scaledHue = hue / 30.0f;
    float scaledSaturation = saturation * (1.0f / 100.0f);
    float scaledLightness = lightness * (1.0f / 100.0f);

    auto a = scaledSaturation * std::min(scaledLightness, 1.0f - scaledLightness);

    auto HueToRGB = [&](float n)
    {
      auto k = std::fmod(n + scaledHue, 12.0f);
      return scaledLightness - (a * std::max(-1.0f, std::min({k - 3.0f, 9.0f - k, 1.0f})));
    };

    return {HueToRGB(0), HueToRGB(8), HueToRGB(4), alpha};
  }

  // MARK: HWB conversions.

  HWBA<float>
    ColorConversion<HWBA<float>, ExtendedSRGBA<float>>::Convert(const ExtendedSRGBA<float> &color) noexcept
  {
    // https://drafts.csswg.org/css-color-4/#rgb-to-hwb

    auto [red, green, blue, alpha] = color.Resolved();

    auto [min, max] = std::minmax({red, green, blue});
    auto d = max - min;

    float hue = std::numeric_limits<float>::quiet_NaN();

    // Compute `hue` as done in conversion to HSLA, but don't adjust for negative saturation, in order to
    // respect out-of-gamut colors.
    if (d != 0.0f)
    {
      if (max == red)
      {
        hue = ((green - blue) / d) + (green < blue ? 6.0f : 0.0f);
      }
      else if (max == green)
      {
        hue = ((blue - red) / d) + 2.0f;
      }
      else if (max == blue)
      {
        hue = ((red - green) / d) + 4.0f;
      }

      hue *= 60.0f;

      if (hue >= 360.0f)
      {
        hue -= 360.0f;
      }
    }

    auto whiteness = min * 100.0f;
    auto blackness = (1.0f - max) * 100.0f;

    return {hue, whiteness, blackness, alpha};
  }

  ExtendedSRGBA<float>
    ColorConversion<ExtendedSRGBA<float>, HWBA<float>>::Convert(const HWBA<float> &color) noexcept
  {
    // https://drafts.csswg.org/css-color-4/#hwb-to-rgb
    auto [hue, whiteness, blackness, alpha] = color.Resolved();

    float scaledWhiteness = whiteness / 100.0f;
    float scaledBlackness = blackness / 100.0f;

    if (scaledWhiteness + scaledBlackness >= 1.0f)
    {
      auto grey = scaledWhiteness / (scaledWhiteness + scaledBlackness);
      return {grey, grey, grey, alpha};
    }

    float scaledHue = hue / 30.0f;
    float whitenessBlacknessFactor = 1.0f - scaledWhiteness - scaledBlackness;

    auto HueToRGB = [&](float n)
    {
      // Perform RGB selection from HSL conversion as if called with [ hue, 100%, 50% ].
      auto k = std::fmod(n + scaledHue, 12.0f);
      auto component = 0.5f - (std::max(-1.0f, std::min({k - 3.0f, 9.0f - k, 1.0f})) / 2.0f);

      // Then apply whiteness/blackness to the component.
      return (component * whitenessBlacknessFactor) + scaledWhiteness;
    };

    return {HueToRGB(0), HueToRGB(8), HueToRGB(4), alpha};
  }

  // MARK: Lab conversions.

  constexpr float LABe = static_cast<float>(216.0 / 24389.0);
  constexpr float LABk = static_cast<float>(24389.0 / 27.0);

  XYZA<float, WhitePoint::D50>
    ColorConversion<XYZA<float, WhitePoint::D50>, Lab<float>>::Convert(const Lab<float> &color) noexcept
  {
    auto [lightness, a, b, alpha] = color.Resolved();

    float f1 = (lightness + 16.0f) / 116.0f;
    float f0 = f1 + (a / 500.0f);
    float f2 = f1 - (b / 200.0f);

    auto ComputeXAndZ = [](float t)
    {
      float tCubed = t * t * t;
      if (tCubed > LABe)
      {
        return tCubed;
      }

      return (116.0f * t - 16.0f) / LABk;
    };

    auto ComputeY = [](float t)
    {
      if (t > (LABk * LABe))
      {
        float value = static_cast<float>((t + 16.0f) / 116.0);
        return value * value * value;
      }

      return t / LABk;
    };

    float x = D50WhitePoint[0] * ComputeXAndZ(f0);
    float y = D50WhitePoint[1] * ComputeY(lightness);
    float z = D50WhitePoint[2] * ComputeXAndZ(f2);

    return {x, y, z, alpha};
  }

  Lab<float> ColorConversion<Lab<float>, XYZA<float, WhitePoint::D50>>::Convert(
    const XYZA<float, WhitePoint::D50> &color) noexcept
  {
    auto [x, y, z, alpha] = color.Resolved();

    float adjustedX = x / D50WhitePoint[0];
    float adjustedY = y / D50WhitePoint[1];
    float adjustedZ = z / D50WhitePoint[2];

    auto fTransform = [](float value)
    {
      return value > LABe ? std::cbrt(value) : (LABk * value + 16.0f) / 116.0f;
    };

    float f0 = fTransform(adjustedX);
    float f1 = fTransform(adjustedY);
    float f2 = fTransform(adjustedZ);

    float lightness = (116.0f * f1) - 16.0f;
    float a = 500.0f * (f0 - f1);
    float b = 200.0f * (f1 - f2);

    return CreateFromComponentsClampingExceptAlpha<Lab<float>>(lightness, a, b, alpha);
  }

  // MARK: LCH conversions.

  LCHA<float> ColorConversion<LCHA<float>, Lab<float>>::Convert(const Lab<float> &color) noexcept
  {
    return ConvertToPolarForm<LCHA<float>>(color);
  }

  Lab<float> ColorConversion<Lab<float>, LCHA<float>>::Convert(const LCHA<float> &color) noexcept
  {
    return convertToRectangularForm<Lab<float>>(color);
  }

  // MARK: OKLab conversions.

  XYZA<float, WhitePoint::D65>
    ColorConversion<XYZA<float, WhitePoint::D65>, OKLab<float>>::Convert(const OKLab<float> &color) noexcept
  {
    // FIXME: This could be optimized for when we are not explicitly converting to XYZ-D65 by pre-multiplying
    // the 'LMSToXYZD65' matrix with any subsequent matrices in the conversion. This would mean teaching the
    // main conversion about this matrix and adding new logic for this transform.

    // https://bottosson.github.io/posts/oklab/ with XYZ <-> LMS matrices recalculated for consistent
    // reference white in https://github.com/w3c/csswg-drafts/issues/6642#issuecomment-943521484

    constexpr ColorMatrix<3, 3> LinearLMSToXYZD65 {
      1.2268798758459243,  -0.5578149944602171, 0.2813910456659647,  -0.0405757452148008, 1.1122868032803170,
      -0.0717110580655164, -0.0763729366746601, -0.4214933324022432, 1.5869240198367816,
    };

    constexpr ColorMatrix<3, 3> OKLabToNonLinearLMS {
      1.0000000000000000,  0.3963377773761749, 0.2158037573099136,  1.0000000000000000,  -0.1055613458156586,
      -0.0638541728258133, 1.0000000000000000, -0.0894841775298119, -1.2914855480194092,
    };

    auto [lightness, a, b, alpha] = color.Resolved();

    auto components = ColorComponents<float, 3> {lightness, a, b};

    // 1. Transform from Lab-coordinates into non-linear LMS "approximate cone responses".
    auto nonLinearLMS = OKLabToNonLinearLMS.TransformedColorComponents(components);

    // 2. Apply linearity.
    auto linearLMS = nonLinearLMS.Map([](float v) { return v * v * v; });

    // 3. Convert to XYZ.
    auto [x, y, z] = LinearLMSToXYZD65.TransformedColorComponents(linearLMS);

    return {x, y, z, alpha};
  }

  OKLab<float> ColorConversion<OKLab<float>, XYZA<float, WhitePoint::D65>>::Convert(
    const XYZA<float, WhitePoint::D65> &color) noexcept
  {
    // FIXME: This could be optimized for when we are not explicitly converting from XYZ-D65 by
    // pre-multiplying the 'XYZD65ToLMS' matrix with any previous matrices in the conversion. This would mean
    // teaching the main conversion about this matrix and adding new logic for this transform.

    // https://bottosson.github.io/posts/oklab/ with XYZ <-> LMS matrices recalculated for consistent
    // reference white in https://github.com/w3c/csswg-drafts/issues/6642#issuecomment-943521484

    constexpr ColorMatrix<3, 3> XYZD65ToLinearLMS {
      0.8190224379967030, 0.3619062600528904, -0.1288737815209879, 0.0329836539323885, 0.9292868615863434,
      0.0361446663506424, 0.0481771893596242, 0.2642395317527308,  0.6335478284694309,
    };

    constexpr ColorMatrix<3, 3> NonLinearLMSToOKLab {
      0.2104542683093140, 0.7936177747023054, -0.0040720430116193, 1.9779985324311684,  -2.4285922420485799,
      0.4505937096174110, 0.0259040424655478, 0.7827717124575296,  -0.8086757549230774,
    };

    auto [x, y, z, alpha] = color.Resolved();

    // 1. Convert XYZ into LMS "approximate cone responses".
    auto linearLMS = XYZD65ToLinearLMS.TransformedColorComponents(ColorComponents<float, 3> {x, y, z});

    // 2. Apply non-linearity.
    auto nonLinearLMS = linearLMS.Map([](float v) { return std::cbrt(v); });

    // 3. Transform into Lab-coordinates.
    auto [lightness, a, b] = NonLinearLMSToOKLab.TransformedColorComponents(nonLinearLMS);

    return CreateFromComponentsClampingExceptAlpha<OKLab<float>>(lightness, a, b, alpha);
  }

  // MARK: OKLCH conversions.

  OKLCHA<float> ColorConversion<OKLCHA<float>, OKLab<float>>::Convert(const OKLab<float> &color) noexcept
  {
    return ConvertToPolarForm<OKLCHA<float>>(color);
  }

  OKLab<float> ColorConversion<OKLab<float>, OKLCHA<float>>::Convert(const OKLCHA<float> &color) noexcept
  {
    return convertToRectangularForm<OKLab<float>>(color);
  }

  // MARK: Conversion functions for raw color components with associated color spaces.

  ColorComponents<float, 4> ConvertAndResolveColorComponents(ColorSpace inputColorSpace,
                                                             ColorComponents<float, 4> inputColorComponents,
                                                             ColorSpace outputColorSpace) noexcept
  {
    return CallWithColorType<float>(
      inputColorSpace,
      [&]<typename InputColorType>()
      {
        auto inputColor = CreateFromComponents<InputColorType>(inputColorComponents);
        return CallWithColorType<float>(
          outputColorSpace, [&]<typename OutputColorType>()
          { return AsColorComponents(ConvertColor<OutputColorType>(inputColor).Resolved()); });
      });
  }

  ColorComponents<float, 4>
    ConvertAndResolveColorComponents(ColorSpace inputColorSpace,
                                     ColorComponents<float, 4> inputColorComponents,
                                     const DestinationColorSpace &outputColorSpace) noexcept
  {
    return {};
    // TODO:
    // return CallWithColorType(
    //  inputColorComponents, inputColorSpace,
    //  [outputColorSpace](const auto &inputColor)
    //  {
    //    if (outputColorSpace == DestinationColorSpace::SRGB())
    //      return AsColorComponents(ConvertColor<SRGBA<float>>(inputColor).Resolved());
    //    if (outputColorSpace == DestinationColorSpace::LinearSRGB())
    //      return AsColorComponents(ConvertColor<LinearSRGBA<float>>(inputColor).Resolved());
    //    if (outputColorSpace == DestinationColorSpace::DisplayP3())
    //      return AsColorComponents(ConvertColor<DisplayP3<float>>(inputColor).Resolved());

    // assert(false);
    // return AsColorComponents(ConvertColor<SRGBA<float>>(inputColor).Resolved());
    // });
  }

}