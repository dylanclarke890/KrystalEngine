#pragma once

#include "Krystal.Booey/Core/Color/AlphaPremultiplication.hpp"
#include "Krystal.Booey/Core/Color/ColorTypes.hpp"
#include "Krystal.Core/Types/Variant.hpp"

namespace krys::boo
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
      using ColorType = krys::boo::HSLA<float>;
      HueInterpolationMethod hueInterpolationMethod = HueInterpolationMethod::Shorter;

      constexpr bool operator==(const HSL &) const noexcept = default;
    };

    struct HWB
    {
      constexpr static auto interpolationColorSpace = ColorInterpolationColorSpace::HWB;
      using ColorType = krys::boo::HWBA<float>;
      HueInterpolationMethod hueInterpolationMethod = HueInterpolationMethod::Shorter;

      constexpr bool operator==(const HWB &) const noexcept = default;
    };

    struct LCH
    {
      constexpr static auto interpolationColorSpace = ColorInterpolationColorSpace::LCH;
      using ColorType = krys::boo::LCHA<float>;
      HueInterpolationMethod hueInterpolationMethod = HueInterpolationMethod::Shorter;

      constexpr bool operator==(const LCH &) const noexcept = default;
    };

    struct Lab
    {
      constexpr static auto interpolationColorSpace = ColorInterpolationColorSpace::Lab;
      using ColorType = krys::boo::Lab<float>;

      constexpr bool operator==(const Lab &) const noexcept = default;
    };

    struct OKLCH
    {
      constexpr static auto interpolationColorSpace = ColorInterpolationColorSpace::OKLCH;
      using ColorType = krys::boo::OKLCHA<float>;
      HueInterpolationMethod hueInterpolationMethod = HueInterpolationMethod::Shorter;

      constexpr bool operator==(const OKLCH &) const noexcept = default;
    };

    struct OKLab
    {
      constexpr static auto interpolationColorSpace = ColorInterpolationColorSpace::OKLab;
      using ColorType = krys::boo::OKLab<float>;

      constexpr bool operator==(const OKLab &) const noexcept = default;
    };

    struct SRGB
    {
      constexpr static auto interpolationColorSpace = ColorInterpolationColorSpace::SRGB;
      using ColorType = krys::boo::ExtendedSRGBA<float>;

      constexpr bool operator==(const SRGB &) const noexcept = default;
    };

    struct SRGBLinear
    {
      constexpr static auto interpolationColorSpace = ColorInterpolationColorSpace::SRGBLinear;
      using ColorType = krys::boo::ExtendedLinearSRGBA<float>;

      constexpr bool operator==(const SRGBLinear &) const noexcept = default;
    };

    struct DisplayP3
    {
      constexpr static auto interpolationColorSpace = ColorInterpolationColorSpace::DisplayP3;
      using ColorType = krys::boo::ExtendedDisplayP3<float>;

      constexpr bool operator==(const DisplayP3 &) const noexcept = default;
    };

    struct DisplayP3Linear
    {
      constexpr static auto interpolationColorSpace = ColorInterpolationColorSpace::DisplayP3Linear;
      using ColorType = krys::boo::ExtendedLinearDisplayP3<float>;

      constexpr bool operator==(const DisplayP3Linear &) const noexcept = default;
    };

    struct A98RGB
    {
      constexpr static auto interpolationColorSpace = ColorInterpolationColorSpace::A98RGB;
      using ColorType = krys::boo::ExtendedA98RGB<float>;

      constexpr bool operator==(const A98RGB &) const noexcept = default;
    };

    struct ProPhotoRGB
    {
      constexpr static auto interpolationColorSpace = ColorInterpolationColorSpace::ProPhotoRGB;
      using ColorType = krys::boo::ExtendedProPhotoRGB<float>;

      constexpr bool operator==(const ProPhotoRGB &) const noexcept = default;
    };

    struct Rec2020
    {
      constexpr static auto interpolationColorSpace = ColorInterpolationColorSpace::Rec2020;
      using ColorType = krys::boo::ExtendedRec2020<float>;

      constexpr bool operator==(const Rec2020 &) const noexcept = default;
    };

    struct XYZD50
    {
      constexpr static auto interpolationColorSpace = ColorInterpolationColorSpace::XYZD50;
      using ColorType = krys::boo::XYZA<float, WhitePoint::D50>;

      constexpr bool operator==(const XYZD50 &) const noexcept = default;
    };

    struct XYZD65
    {
      constexpr static auto interpolationColorSpace = ColorInterpolationColorSpace::XYZD65;
      using ColorType = krys::boo::XYZA<float, WhitePoint::D65>;

      constexpr bool operator==(const XYZD65 &) const noexcept = default;
    };

    constexpr bool operator==(const ColorInterpolationMethod &) const noexcept = default;

    Variant<HSL, HWB, LCH, Lab, OKLCH, OKLab, SRGB, SRGBLinear, DisplayP3, DisplayP3Linear, A98RGB,
            ProPhotoRGB, Rec2020, XYZD50, XYZD65>
      colorSpace;
    AlphaPremultiplication alphaPremultiplication;
  };
}