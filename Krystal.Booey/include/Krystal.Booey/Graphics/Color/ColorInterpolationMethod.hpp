#pragma once

#include "Krystal.Booey/Graphics/Color/AlphaPremultiplication.hpp"
#include "Krystal.Booey/Graphics/Color/ColorTypes.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Hash.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"
#include "Krystal.Core/Types/String.hpp"
#include "Krystal.Core/Types/Variant.hpp"
#include "Krystal.Core/Visitor.hpp"

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
  struct HasHueInterpolationMethod<T, std::void_t<decltype(std::declval<T>().HueInterpolationMethod)>>
      : std::true_type
  {
  };

  template <typename T>
  constexpr bool hasHueInterpolationMethod = HasHueInterpolationMethod<T>::value;

  struct ColorInterpolationMethod
  {
    struct HSL
    {
      constexpr static auto InterpolationColorSpace = ColorInterpolationColorSpace::HSL;
      using ColorType = krys::boo::HSLA<float>;
      boo::HueInterpolationMethod HueInterpolationMethod = boo::HueInterpolationMethod::Shorter;

      constexpr bool operator==(const HSL &) const noexcept = default;
    };

    struct HWB
    {
      constexpr static auto InterpolationColorSpace = ColorInterpolationColorSpace::HWB;
      using ColorType = krys::boo::HWBA<float>;
      boo::HueInterpolationMethod HueInterpolationMethod = boo::HueInterpolationMethod::Shorter;

      constexpr bool operator==(const HWB &) const noexcept = default;
    };

    struct LCH
    {
      constexpr static auto InterpolationColorSpace = ColorInterpolationColorSpace::LCH;
      using ColorType = krys::boo::LCHA<float>;
      boo::HueInterpolationMethod HueInterpolationMethod = boo::HueInterpolationMethod::Shorter;

      constexpr bool operator==(const LCH &) const noexcept = default;
    };

    struct Lab
    {
      constexpr static auto InterpolationColorSpace = ColorInterpolationColorSpace::Lab;
      using ColorType = krys::boo::Lab<float>;

      constexpr bool operator==(const Lab &) const noexcept = default;
    };

    struct OKLCH
    {
      constexpr static auto InterpolationColorSpace = ColorInterpolationColorSpace::OKLCH;
      using ColorType = krys::boo::OKLCHA<float>;
      boo::HueInterpolationMethod HueInterpolationMethod = boo::HueInterpolationMethod::Shorter;

      constexpr bool operator==(const OKLCH &) const noexcept = default;
    };

    struct OKLab
    {
      constexpr static auto InterpolationColorSpace = ColorInterpolationColorSpace::OKLab;
      using ColorType = krys::boo::OKLab<float>;

      constexpr bool operator==(const OKLab &) const noexcept = default;
    };

    struct SRGB
    {
      constexpr static auto InterpolationColorSpace = ColorInterpolationColorSpace::SRGB;
      using ColorType = krys::boo::ExtendedSRGBA<float>;

      constexpr bool operator==(const SRGB &) const noexcept = default;
    };

    struct SRGBLinear
    {
      constexpr static auto InterpolationColorSpace = ColorInterpolationColorSpace::SRGBLinear;
      using ColorType = krys::boo::ExtendedLinearSRGBA<float>;

      constexpr bool operator==(const SRGBLinear &) const noexcept = default;
    };

    struct DisplayP3
    {
      constexpr static auto InterpolationColorSpace = ColorInterpolationColorSpace::DisplayP3;
      using ColorType = krys::boo::ExtendedDisplayP3<float>;

      constexpr bool operator==(const DisplayP3 &) const noexcept = default;
    };

    struct DisplayP3Linear
    {
      constexpr static auto InterpolationColorSpace = ColorInterpolationColorSpace::DisplayP3Linear;
      using ColorType = krys::boo::ExtendedLinearDisplayP3<float>;

      constexpr bool operator==(const DisplayP3Linear &) const noexcept = default;
    };

    struct A98RGB
    {
      constexpr static auto InterpolationColorSpace = ColorInterpolationColorSpace::A98RGB;
      using ColorType = krys::boo::ExtendedA98RGB<float>;

      constexpr bool operator==(const A98RGB &) const noexcept = default;
    };

    struct ProPhotoRGB
    {
      constexpr static auto InterpolationColorSpace = ColorInterpolationColorSpace::ProPhotoRGB;
      using ColorType = krys::boo::ExtendedProPhotoRGB<float>;

      constexpr bool operator==(const ProPhotoRGB &) const noexcept = default;
    };

    struct Rec2020
    {
      constexpr static auto InterpolationColorSpace = ColorInterpolationColorSpace::Rec2020;
      using ColorType = krys::boo::ExtendedRec2020<float>;

      constexpr bool operator==(const Rec2020 &) const noexcept = default;
    };

    struct XYZD50
    {
      constexpr static auto InterpolationColorSpace = ColorInterpolationColorSpace::XYZD50;
      using ColorType = krys::boo::XYZA<float, WhitePoint::D50>;

      constexpr bool operator==(const XYZD50 &) const noexcept = default;
    };

    struct XYZD65
    {
      constexpr static auto InterpolationColorSpace = ColorInterpolationColorSpace::XYZD65;
      using ColorType = krys::boo::XYZA<float, WhitePoint::D65>;

      constexpr bool operator==(const XYZD65 &) const noexcept = default;
    };

    constexpr bool operator==(const ColorInterpolationMethod &) const noexcept = default;

    Variant<HSL, HWB, LCH, Lab, OKLCH, OKLab, SRGB, SRGBLinear, DisplayP3, DisplayP3Linear, A98RGB,
            ProPhotoRGB, Rec2020, XYZD50, XYZD65>
      ColorSpace;
    boo::AlphaPremultiplication AlphaPremultiplication;
  };

  KRYS_NODISCARD constexpr utf8_stringview
    SerialisationForCSS(ColorInterpolationColorSpace interpolationColorSpace) noexcept
  {
    switch (interpolationColorSpace)
    {
      case ColorInterpolationColorSpace::HSL:             return u8"hsl";
      case ColorInterpolationColorSpace::HWB:             return u8"hwb";
      case ColorInterpolationColorSpace::LCH:             return u8"lch";
      case ColorInterpolationColorSpace::Lab:             return u8"lab";
      case ColorInterpolationColorSpace::OKLCH:           return u8"oklch";
      case ColorInterpolationColorSpace::OKLab:           return u8"oklab";
      case ColorInterpolationColorSpace::SRGB:            return u8"srgb";
      case ColorInterpolationColorSpace::SRGBLinear:      return u8"srgb-linear";
      case ColorInterpolationColorSpace::DisplayP3:       return u8"display-p3";
      case ColorInterpolationColorSpace::DisplayP3Linear: return u8"display-p3-linear";
      case ColorInterpolationColorSpace::A98RGB:          return u8"a98-rgb";
      case ColorInterpolationColorSpace::ProPhotoRGB:     return u8"prophoto-rgb";
      case ColorInterpolationColorSpace::Rec2020:         return u8"rec2020";
      case ColorInterpolationColorSpace::XYZD50:          return u8"xyz-d50";
      case ColorInterpolationColorSpace::XYZD65:          return u8"xyz-d65";
    }

    krys_unreachable();
  }

  void SerialisationForCSS(StringBuilder &builder,
                           ColorInterpolationColorSpace interpolationColorSpace) noexcept;

  void SerialisationForCSS(StringBuilder &builder, HueInterpolationMethod hueInterpolationMethod) noexcept;

  void SerialisationForCSS(StringBuilder &builder, const ColorInterpolationMethod &method) noexcept;

  KRYS_NODISCARD utf8_string SerialisationForCSS(const ColorInterpolationMethod &method) noexcept;
}

namespace krys
{
  template <>
  struct DefaultHash<krys::boo::ColorInterpolationMethod>
  {
    KRYS_NODISCARD static size_t Hash(const krys::boo::ColorInterpolationMethod &method) noexcept
    {
      return krys::SwitchOn(method.ColorSpace,
                            [&]<typename MethodColorSpace>(const MethodColorSpace &methodColorSpace) -> size_t
                            {
                              if constexpr (MethodColorSpace::ColorType::Model::CoordinateSystem
                                            == krys::boo::ColorSpaceCoordinateSystem::CylindricalPolar)
                              {
                                return krys::Hash::Combine(method.AlphaPremultiplication,
                                                           methodColorSpace.InterpolationColorSpace,
                                                           methodColorSpace.HueInterpolationMethod);
                              }
                              else
                              {
                                return krys::Hash::Combine(method.AlphaPremultiplication,
                                                           methodColorSpace.InterpolationColorSpace);
                              }
                            });
    }

    KRYS_NODISCARD static bool Equal(const krys::boo::ColorInterpolationMethod &a,
                                     const krys::boo::ColorInterpolationMethod &b) noexcept
    {
      return a == b;
    }
  };
}
