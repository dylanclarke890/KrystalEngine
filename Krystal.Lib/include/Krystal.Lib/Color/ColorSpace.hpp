#pragma once

#include "Krystal.Lib/Color/ColorTypes.hpp"
#include <cassert>

namespace Krys
{
  enum class ColorSpace : uint8
  {
    A98RGB,
    DisplayP3,
    ExtendedA98RGB,
    ExtendedDisplayP3,
    ExtendedLinearDisplayP3,
    ExtendedLinearSRGB,
    ExtendedProPhotoRGB,
    ExtendedRec2020,
    ExtendedSRGB,
    HSL,
    HWB,
    LCH,
    Lab,
    LinearDisplayP3,
    LinearSRGB,
    OKLCH,
    OKLab,
    ProPhotoRGB,
    Rec2020,
    SRGB,
    XYZ_D50,
    XYZ_D65,
  };

  template <typename>
  struct ColorSpaceMapping;

  template <typename T>
  struct ColorSpaceMapping<A98RGB<T>>
  {
    constexpr static auto colorSpace {ColorSpace::A98RGB};
  };

  template <typename T>
  struct ColorSpaceMapping<DisplayP3<T>>
  {
    constexpr static auto colorSpace {ColorSpace::DisplayP3};
  };

  template <typename T>
  struct ColorSpaceMapping<ExtendedA98RGB<T>>
  {
    constexpr static auto colorSpace {ColorSpace::ExtendedA98RGB};
  };

  template <typename T>
  struct ColorSpaceMapping<ExtendedDisplayP3<T>>
  {
    constexpr static auto colorSpace {ColorSpace::ExtendedDisplayP3};
  };

  template <typename T>
  struct ColorSpaceMapping<ExtendedLinearDisplayP3<T>>
  {
    constexpr static auto colorSpace {ColorSpace::ExtendedLinearDisplayP3};
  };

  template <typename T>
  struct ColorSpaceMapping<ExtendedLinearSRGBA<T>>
  {
    constexpr static auto colorSpace {ColorSpace::ExtendedLinearSRGB};
  };

  template <typename T>
  struct ColorSpaceMapping<ExtendedProPhotoRGB<T>>
  {
    constexpr static auto colorSpace {ColorSpace::ExtendedProPhotoRGB};
  };

  template <typename T>
  struct ColorSpaceMapping<ExtendedRec2020<T>>
  {
    constexpr static auto colorSpace {ColorSpace::ExtendedRec2020};
  };

  template <typename T>
  struct ColorSpaceMapping<ExtendedSRGBA<T>>
  {
    constexpr static auto colorSpace {ColorSpace::ExtendedSRGB};
  };

  template <typename T>
  struct ColorSpaceMapping<HSLA<T>>
  {
    constexpr static auto colorSpace {ColorSpace::HSL};
  };

  template <typename T>
  struct ColorSpaceMapping<HWBA<T>>
  {
    constexpr static auto colorSpace {ColorSpace::HWB};
  };

  template <typename T>
  struct ColorSpaceMapping<LCHA<T>>
  {
    constexpr static auto colorSpace {ColorSpace::LCH};
  };

  template <typename T>
  struct ColorSpaceMapping<Lab<T>>
  {
    constexpr static auto colorSpace {ColorSpace::Lab};
  };

  template <typename T>
  struct ColorSpaceMapping<LinearDisplayP3<T>>
  {
    constexpr static auto colorSpace {ColorSpace::LinearDisplayP3};
  };

  template <typename T>
  struct ColorSpaceMapping<LinearSRGBA<T>>
  {
    constexpr static auto colorSpace {ColorSpace::LinearSRGB};
  };

  template <typename T>
  struct ColorSpaceMapping<OKLab<T>>
  {
    constexpr static auto colorSpace {ColorSpace::OKLab};
  };

  template <typename T>
  struct ColorSpaceMapping<OKLCHA<T>>
  {
    constexpr static auto colorSpace {ColorSpace::OKLCH};
  };

  template <typename T>
  struct ColorSpaceMapping<ProPhotoRGB<T>>
  {
    constexpr static auto colorSpace {ColorSpace::ProPhotoRGB};
  };

  template <typename T>
  struct ColorSpaceMapping<Rec2020<T>>
  {
    constexpr static auto colorSpace {ColorSpace::Rec2020};
  };

  template <typename T>
  struct ColorSpaceMapping<SRGBA<T>>
  {
    constexpr static auto colorSpace {ColorSpace::SRGB};
  };

  template <typename T>
  struct ColorSpaceMapping<XYZA<T, WhitePoint::D50>>
  {
    constexpr static auto colorSpace {ColorSpace::XYZ_D50};
  };

  template <typename T>
  struct ColorSpaceMapping<XYZA<T, WhitePoint::D65>>
  {
    constexpr static auto colorSpace {ColorSpace::XYZ_D65};
  };

  template <typename ColorType>
  constexpr ColorSpace ColorSpaceFor = ColorSpaceMapping<CanonicalColorType<ColorType>>::colorSpace;

  template <typename T, typename Functor>
  KRYS_NODISCARD constexpr decltype(auto) CallWithColorType(ColorSpace colorSpace, Functor &&functor) noexcept
  {
    switch (colorSpace)
    {
      case ColorSpace::A98RGB:
      {
        return functor.template operator()<A98RGB<T>>();
      }
      case ColorSpace::DisplayP3:
      {
        return functor.template operator()<DisplayP3<T>>();
      }
      case ColorSpace::ExtendedA98RGB:
      {
        return functor.template operator()<ExtendedA98RGB<T>>();
      }
      case ColorSpace::ExtendedDisplayP3:
      {
        return functor.template operator()<ExtendedDisplayP3<T>>();
      }
      case ColorSpace::ExtendedLinearDisplayP3:
      {
        return functor.template operator()<ExtendedLinearDisplayP3<T>>();
      }
      case ColorSpace::ExtendedLinearSRGB:
      {
        return functor.template operator()<ExtendedLinearSRGBA<T>>();
      }
      case ColorSpace::ExtendedProPhotoRGB:
      {
        return functor.template operator()<ExtendedProPhotoRGB<T>>();
      }
      case ColorSpace::ExtendedRec2020:
      {
        return functor.template operator()<ExtendedRec2020<T>>();
      }
      case ColorSpace::ExtendedSRGB:
      {
        return functor.template operator()<ExtendedSRGBA<T>>();
      }
      case ColorSpace::HSL:
      {
        return functor.template operator()<HSLA<T>>();
      }
      case ColorSpace::HWB:
      {
        return functor.template operator()<HWBA<T>>();
      }
      case ColorSpace::LCH:
      {
        return functor.template operator()<LCHA<T>>();
      }
      case ColorSpace::Lab:
      {
        return functor.template operator()<Lab<T>>();
      }
      case ColorSpace::LinearDisplayP3:
      {
        return functor.template operator()<LinearDisplayP3<T>>();
      }
      case ColorSpace::LinearSRGB:
      {
        return functor.template operator()<LinearSRGBA<T>>();
      }
      case ColorSpace::OKLCH:
      {
        return functor.template operator()<OKLCHA<T>>();
      }
      case ColorSpace::OKLab:
      {
        return functor.template operator()<OKLab<T>>();
      }
      case ColorSpace::ProPhotoRGB:
      {
        return functor.template operator()<ProPhotoRGB<T>>();
      }
      case ColorSpace::Rec2020:
      {
        return functor.template operator()<Rec2020<T>>();
      }
      case ColorSpace::SRGB:
      {
        return functor.template operator()<SRGBA<T>>();
      }
      case ColorSpace::XYZ_D50:
      {
        return functor.template operator()<XYZA<T, WhitePoint::D50>>();
      }
      case ColorSpace::XYZ_D65:
      {
        return functor.template operator()<XYZA<T, WhitePoint::D65>>();
      }
    }

    assert(false);
    return functor.template operator()<SRGBA<T>>();
  }

  template <typename T, typename Functor>
  KRYS_NODISCARD constexpr decltype(auto) CallWithColorType(const ColorComponents<T, 4> &components,
                                                            ColorSpace colorSpace, Functor &&functor) noexcept
  {
    return CallWithColorType<T>(
      colorSpace, [&]<typename ColorType>()
      { return std::invoke(std::forward<Functor>(functor), CreateFromComponents<ColorType>(components)); });
  }
}