#pragma once

#include "Krystal.Lib/Color/AlphaPremultiplication.hpp"
#include "Krystal.Lib/Color/ColorTypes.hpp"
#include "Krystal.Lib/Types/Variant.hpp"

namespace Krys
{
  enum class HueInterpolationMethod : uint8
  {
    Shorter,
    Longer,
    Increasing,
    Decreasing
  };

  enum class ColorInterpolationColorSpace : uint8
  {
    HSL,
    HWB,
    LCH,
    Lab,
    OKLCH,
    OKLab,
    SRGB,
    SRGBLinear,
    DisplayP3,
    DisplayP3Linear,
    A98RGB,
    ProPhotoRGB,
    Rec2020,
    XYZD50,
    XYZD65
  };

  template <typename T, typename = void>
  struct HasHueInterpolationMethod : std::false_type
  {
  };

  template <typename T>
  struct HasHueInterpolationMethod<T, std::void_t<decltype(std::declval<T>().hueInterpolationMethod)>>
      : std::true_type
  {
  };

  template <typename T>
  constexpr bool hasHueInterpolationMethod = HasHueInterpolationMethod<T>::value;

  struct ColorInterpolationMethod
  {
    struct HSL
    {
      constexpr static auto interpolationColorSpace = ColorInterpolationColorSpace::HSL;
      using ColorType = Krys::HSLA<float>;
      HueInterpolationMethod hueInterpolationMethod = HueInterpolationMethod::Shorter;

      constexpr bool operator==(const HSL &) const noexcept = default;
    };

    struct HWB
    {
      constexpr static auto interpolationColorSpace = ColorInterpolationColorSpace::HWB;
      using ColorType = Krys::HWBA<float>;
      HueInterpolationMethod hueInterpolationMethod = HueInterpolationMethod::Shorter;

      constexpr bool operator==(const HWB &) const noexcept = default;
    };

    struct LCH
    {
      constexpr static auto interpolationColorSpace = ColorInterpolationColorSpace::LCH;
      using ColorType = Krys::LCHA<float>;
      HueInterpolationMethod hueInterpolationMethod = HueInterpolationMethod::Shorter;

      constexpr bool operator==(const LCH &) const noexcept = default;
    };

    struct Lab
    {
      constexpr static auto interpolationColorSpace = ColorInterpolationColorSpace::Lab;
      using ColorType = Krys::Lab<float>;

      constexpr bool operator==(const Lab &) const noexcept = default;
    };

    struct OKLCH
    {
      constexpr static auto interpolationColorSpace = ColorInterpolationColorSpace::OKLCH;
      using ColorType = Krys::OKLCHA<float>;
      HueInterpolationMethod hueInterpolationMethod = HueInterpolationMethod::Shorter;

      constexpr bool operator==(const OKLCH &) const noexcept = default;
    };

    struct OKLab
    {
      constexpr static auto interpolationColorSpace = ColorInterpolationColorSpace::OKLab;
      using ColorType = Krys::OKLab<float>;

      constexpr bool operator==(const OKLab &) const noexcept = default;
    };

    struct SRGB
    {
      constexpr static auto interpolationColorSpace = ColorInterpolationColorSpace::SRGB;
      using ColorType = Krys::ExtendedSRGBA<float>;

      constexpr bool operator==(const SRGB &) const noexcept = default;
    };

    struct SRGBLinear
    {
      constexpr static auto interpolationColorSpace = ColorInterpolationColorSpace::SRGBLinear;
      using ColorType = Krys::ExtendedLinearSRGBA<float>;

      constexpr bool operator==(const SRGBLinear &) const noexcept = default;
    };

    struct DisplayP3
    {
      constexpr static auto interpolationColorSpace = ColorInterpolationColorSpace::DisplayP3;
      using ColorType = Krys::ExtendedDisplayP3<float>;

      constexpr bool operator==(const DisplayP3 &) const noexcept = default;
    };

    struct DisplayP3Linear
    {
      constexpr static auto interpolationColorSpace = ColorInterpolationColorSpace::DisplayP3Linear;
      using ColorType = Krys::ExtendedLinearDisplayP3<float>;

      constexpr bool operator==(const DisplayP3Linear &) const noexcept = default;
    };

    struct A98RGB
    {
      constexpr static auto interpolationColorSpace = ColorInterpolationColorSpace::A98RGB;
      using ColorType = Krys::ExtendedA98RGB<float>;

      constexpr bool operator==(const A98RGB &) const noexcept = default;
    };

    struct ProPhotoRGB
    {
      constexpr static auto interpolationColorSpace = ColorInterpolationColorSpace::ProPhotoRGB;
      using ColorType = Krys::ExtendedProPhotoRGB<float>;

      constexpr bool operator==(const ProPhotoRGB &) const noexcept = default;
    };

    struct Rec2020
    {
      constexpr static auto interpolationColorSpace = ColorInterpolationColorSpace::Rec2020;
      using ColorType = Krys::ExtendedRec2020<float>;

      constexpr bool operator==(const Rec2020 &) const noexcept = default;
    };

    struct XYZD50
    {
      constexpr static auto interpolationColorSpace = ColorInterpolationColorSpace::XYZD50;
      using ColorType = Krys::XYZA<float, WhitePoint::D50>;

      constexpr bool operator==(const XYZD50 &) const noexcept = default;
    };

    struct XYZD65
    {
      constexpr static auto interpolationColorSpace = ColorInterpolationColorSpace::XYZD65;
      using ColorType = Krys::XYZA<float, WhitePoint::D65>;

      constexpr bool operator==(const XYZD65 &) const noexcept = default;
    };

    constexpr bool operator==(const ColorInterpolationMethod &) const noexcept = default;

    Variant<HSL, HWB, LCH, Lab, OKLCH, OKLab, SRGB, SRGBLinear, DisplayP3, DisplayP3Linear, A98RGB,
            ProPhotoRGB, Rec2020, XYZD50, XYZD65>
      colorSpace;
    AlphaPremultiplication alphaPremultiplication;
  };
}