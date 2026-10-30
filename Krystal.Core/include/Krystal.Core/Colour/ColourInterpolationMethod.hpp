#pragma once

#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Colour/AlphaPremultiplication.hpp"
#include "Krystal.Core/Colour/ColourTypes.hpp"
#include "Krystal.Core/Hash.hpp"
#include "Krystal.Core/Types/String.hpp"
#include "Krystal.Core/Types/Variant.hpp"
#include "Krystal.Core/Visitor.hpp"

namespace krys
{
  enum class HueInterpolationMethod : uint8
  {
    Shorter,
    Longer,
    Increasing,
    Decreasing
  };

  enum class ColourInterpolationColourSpace : uint8
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

  struct ColourInterpolationMethod
  {
    struct HSL
    {
      constexpr static auto InterpolationColourSpace = ColourInterpolationColourSpace::HSL;
      using ColourType = krys::HSLA<float>;
      krys::HueInterpolationMethod HueInterpolationMethod = krys::HueInterpolationMethod::Shorter;

      constexpr bool operator==(const HSL &) const noexcept = default;
    };

    struct HWB
    {
      constexpr static auto InterpolationColourSpace = ColourInterpolationColourSpace::HWB;
      using ColourType = krys::HWBA<float>;
      krys::HueInterpolationMethod HueInterpolationMethod = krys::HueInterpolationMethod::Shorter;

      constexpr bool operator==(const HWB &) const noexcept = default;
    };

    struct LCH
    {
      constexpr static auto InterpolationColourSpace = ColourInterpolationColourSpace::LCH;
      using ColourType = krys::LCHA<float>;
      krys::HueInterpolationMethod HueInterpolationMethod = krys::HueInterpolationMethod::Shorter;

      constexpr bool operator==(const LCH &) const noexcept = default;
    };

    struct Lab
    {
      constexpr static auto InterpolationColourSpace = ColourInterpolationColourSpace::Lab;
      using ColourType = krys::Lab<float>;

      constexpr bool operator==(const Lab &) const noexcept = default;
    };

    struct OKLCH
    {
      constexpr static auto InterpolationColourSpace = ColourInterpolationColourSpace::OKLCH;
      using ColourType = krys::OKLCHA<float>;
      krys::HueInterpolationMethod HueInterpolationMethod = krys::HueInterpolationMethod::Shorter;

      constexpr bool operator==(const OKLCH &) const noexcept = default;
    };

    struct OKLab
    {
      constexpr static auto InterpolationColourSpace = ColourInterpolationColourSpace::OKLab;
      using ColourType = krys::OKLab<float>;

      constexpr bool operator==(const OKLab &) const noexcept = default;
    };

    struct SRGB
    {
      constexpr static auto InterpolationColourSpace = ColourInterpolationColourSpace::SRGB;
      using ColourType = krys::ExtendedSRGBA<float>;

      constexpr bool operator==(const SRGB &) const noexcept = default;
    };

    struct SRGBLinear
    {
      constexpr static auto InterpolationColourSpace = ColourInterpolationColourSpace::SRGBLinear;
      using ColourType = krys::ExtendedLinearSRGBA<float>;

      constexpr bool operator==(const SRGBLinear &) const noexcept = default;
    };

    struct DisplayP3
    {
      constexpr static auto InterpolationColourSpace = ColourInterpolationColourSpace::DisplayP3;
      using ColourType = krys::ExtendedDisplayP3<float>;

      constexpr bool operator==(const DisplayP3 &) const noexcept = default;
    };

    struct DisplayP3Linear
    {
      constexpr static auto InterpolationColourSpace = ColourInterpolationColourSpace::DisplayP3Linear;
      using ColourType = krys::ExtendedLinearDisplayP3<float>;

      constexpr bool operator==(const DisplayP3Linear &) const noexcept = default;
    };

    struct A98RGB
    {
      constexpr static auto InterpolationColourSpace = ColourInterpolationColourSpace::A98RGB;
      using ColourType = krys::ExtendedA98RGB<float>;

      constexpr bool operator==(const A98RGB &) const noexcept = default;
    };

    struct ProPhotoRGB
    {
      constexpr static auto InterpolationColourSpace = ColourInterpolationColourSpace::ProPhotoRGB;
      using ColourType = krys::ExtendedProPhotoRGB<float>;

      constexpr bool operator==(const ProPhotoRGB &) const noexcept = default;
    };

    struct Rec2020
    {
      constexpr static auto InterpolationColourSpace = ColourInterpolationColourSpace::Rec2020;
      using ColourType = krys::ExtendedRec2020<float>;

      constexpr bool operator==(const Rec2020 &) const noexcept = default;
    };

    struct XYZD50
    {
      constexpr static auto InterpolationColourSpace = ColourInterpolationColourSpace::XYZD50;
      using ColourType = krys::XYZA<float, WhitePoint::D50>;

      constexpr bool operator==(const XYZD50 &) const noexcept = default;
    };

    struct XYZD65
    {
      constexpr static auto InterpolationColourSpace = ColourInterpolationColourSpace::XYZD65;
      using ColourType = krys::XYZA<float, WhitePoint::D65>;

      constexpr bool operator==(const XYZD65 &) const noexcept = default;
    };

    constexpr bool operator==(const ColourInterpolationMethod &) const noexcept = default;

    Variant<HSL, HWB, LCH, Lab, OKLCH, OKLab, SRGB, SRGBLinear, DisplayP3, DisplayP3Linear, A98RGB,
            ProPhotoRGB, Rec2020, XYZD50, XYZD65>
      ColourSpace;
    krys::AlphaPremultiplication AlphaPremultiplication;
  };
}

namespace krys
{
  template <>
  struct DefaultHash<krys::ColourInterpolationMethod>
  {
    KRYS_NODISCARD static size_t Hash(const krys::ColourInterpolationMethod &method) noexcept
    {
      return krys::SwitchOn(
        method.ColourSpace,
        [&]<typename MethodColourSpace>(const MethodColourSpace &methodColorSpace) -> size_t
        {
          if constexpr (MethodColourSpace::ColourType::Model::CoordinateSystem
                        == krys::ColourSpaceCoordinateSystem::CylindricalPolar)
          {
            return krys::Hash::Combine(method.AlphaPremultiplication,
                                       methodColorSpace.InterpolationColourSpace,
                                       methodColorSpace.HueInterpolationMethod);
          }
          else
          {
            return krys::Hash::Combine(method.AlphaPremultiplication,
                                       methodColorSpace.InterpolationColourSpace);
          }
        });
    }

    KRYS_NODISCARD static bool Equal(const krys::ColourInterpolationMethod &a,
                                     const krys::ColourInterpolationMethod &b) noexcept
    {
      return a == b;
    }
  };
}
