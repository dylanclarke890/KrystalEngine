#pragma once

#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Colour/ColourTypes.hpp"

namespace krys
{
  enum class ColourSpace : uint8
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
  struct ColourSpaceMapping;

  template <typename T>
  struct ColourSpaceMapping<A98RGB<T>>
  {
    constexpr static auto ColourSpace {krys::ColourSpace::A98RGB};
  };

  template <typename T>
  struct ColourSpaceMapping<DisplayP3<T>>
  {
    constexpr static auto ColourSpace {krys::ColourSpace::DisplayP3};
  };

  template <typename T>
  struct ColourSpaceMapping<ExtendedA98RGB<T>>
  {
    constexpr static auto ColourSpace {krys::ColourSpace::ExtendedA98RGB};
  };

  template <typename T>
  struct ColourSpaceMapping<ExtendedDisplayP3<T>>
  {
    constexpr static auto ColourSpace {krys::ColourSpace::ExtendedDisplayP3};
  };

  template <typename T>
  struct ColourSpaceMapping<ExtendedLinearDisplayP3<T>>
  {
    constexpr static auto ColourSpace {krys::ColourSpace::ExtendedLinearDisplayP3};
  };

  template <typename T>
  struct ColourSpaceMapping<ExtendedLinearSRGBA<T>>
  {
    constexpr static auto ColourSpace {krys::ColourSpace::ExtendedLinearSRGB};
  };

  template <typename T>
  struct ColourSpaceMapping<ExtendedProPhotoRGB<T>>
  {
    constexpr static auto ColourSpace {krys::ColourSpace::ExtendedProPhotoRGB};
  };

  template <typename T>
  struct ColourSpaceMapping<ExtendedRec2020<T>>
  {
    constexpr static auto ColourSpace {krys::ColourSpace::ExtendedRec2020};
  };

  template <typename T>
  struct ColourSpaceMapping<ExtendedSRGBA<T>>
  {
    constexpr static auto ColourSpace {krys::ColourSpace::ExtendedSRGB};
  };

  template <typename T>
  struct ColourSpaceMapping<HSLA<T>>
  {
    constexpr static auto ColourSpace {krys::ColourSpace::HSL};
  };

  template <typename T>
  struct ColourSpaceMapping<HWBA<T>>
  {
    constexpr static auto ColourSpace {krys::ColourSpace::HWB};
  };

  template <typename T>
  struct ColourSpaceMapping<LCHA<T>>
  {
    constexpr static auto ColourSpace {krys::ColourSpace::LCH};
  };

  template <typename T>
  struct ColourSpaceMapping<Lab<T>>
  {
    constexpr static auto ColourSpace {krys::ColourSpace::Lab};
  };

  template <typename T>
  struct ColourSpaceMapping<LinearDisplayP3<T>>
  {
    constexpr static auto ColourSpace {krys::ColourSpace::LinearDisplayP3};
  };

  template <typename T>
  struct ColourSpaceMapping<LinearSRGBA<T>>
  {
    constexpr static auto ColourSpace {krys::ColourSpace::LinearSRGB};
  };

  template <typename T>
  struct ColourSpaceMapping<OKLab<T>>
  {
    constexpr static auto ColourSpace {krys::ColourSpace::OKLab};
  };

  template <typename T>
  struct ColourSpaceMapping<OKLCHA<T>>
  {
    constexpr static auto ColourSpace {krys::ColourSpace::OKLCH};
  };

  template <typename T>
  struct ColourSpaceMapping<ProPhotoRGB<T>>
  {
    constexpr static auto ColourSpace {krys::ColourSpace::ProPhotoRGB};
  };

  template <typename T>
  struct ColourSpaceMapping<Rec2020<T>>
  {
    constexpr static auto ColourSpace {krys::ColourSpace::Rec2020};
  };

  template <typename T>
  struct ColourSpaceMapping<SRGBA<T>>
  {
    constexpr static auto ColourSpace {krys::ColourSpace::SRGB};
  };

  template <typename T>
  struct ColourSpaceMapping<XYZA<T, WhitePoint::D50>>
  {
    constexpr static auto ColourSpace {krys::ColourSpace::XYZ_D50};
  };

  template <typename T>
  struct ColourSpaceMapping<XYZA<T, WhitePoint::D65>>
  {
    constexpr static auto ColourSpace {krys::ColourSpace::XYZ_D65};
  };

  template <typename ColourType>
  constexpr ColourSpace ColourSpaceFor = ColourSpaceMapping<CanonicalColourType<ColourType>>::ColourSpace;

  template <typename T, typename Functor>
  KRYS_NODISCARD constexpr decltype(auto) CallWithColourType(ColourSpace colorSpace,
                                                             Functor &&functor) noexcept
  {
    switch (colorSpace)
    {
      case ColourSpace::A98RGB:
      {
        return functor.template operator()<A98RGB<T>>();
      }
      case ColourSpace::DisplayP3:
      {
        return functor.template operator()<DisplayP3<T>>();
      }
      case ColourSpace::ExtendedA98RGB:
      {
        return functor.template operator()<ExtendedA98RGB<T>>();
      }
      case ColourSpace::ExtendedDisplayP3:
      {
        return functor.template operator()<ExtendedDisplayP3<T>>();
      }
      case ColourSpace::ExtendedLinearDisplayP3:
      {
        return functor.template operator()<ExtendedLinearDisplayP3<T>>();
      }
      case ColourSpace::ExtendedLinearSRGB:
      {
        return functor.template operator()<ExtendedLinearSRGBA<T>>();
      }
      case ColourSpace::ExtendedProPhotoRGB:
      {
        return functor.template operator()<ExtendedProPhotoRGB<T>>();
      }
      case ColourSpace::ExtendedRec2020:
      {
        return functor.template operator()<ExtendedRec2020<T>>();
      }
      case ColourSpace::ExtendedSRGB:
      {
        return functor.template operator()<ExtendedSRGBA<T>>();
      }
      case ColourSpace::HSL:
      {
        return functor.template operator()<HSLA<T>>();
      }
      case ColourSpace::HWB:
      {
        return functor.template operator()<HWBA<T>>();
      }
      case ColourSpace::LCH:
      {
        return functor.template operator()<LCHA<T>>();
      }
      case ColourSpace::Lab:
      {
        return functor.template operator()<Lab<T>>();
      }
      case ColourSpace::LinearDisplayP3:
      {
        return functor.template operator()<LinearDisplayP3<T>>();
      }
      case ColourSpace::LinearSRGB:
      {
        return functor.template operator()<LinearSRGBA<T>>();
      }
      case ColourSpace::OKLCH:
      {
        return functor.template operator()<OKLCHA<T>>();
      }
      case ColourSpace::OKLab:
      {
        return functor.template operator()<OKLab<T>>();
      }
      case ColourSpace::ProPhotoRGB:
      {
        return functor.template operator()<ProPhotoRGB<T>>();
      }
      case ColourSpace::Rec2020:
      {
        return functor.template operator()<Rec2020<T>>();
      }
      case ColourSpace::SRGB:
      {
        return functor.template operator()<SRGBA<T>>();
      }
      case ColourSpace::XYZ_D50:
      {
        return functor.template operator()<XYZA<T, WhitePoint::D50>>();
      }
      case ColourSpace::XYZ_D65:
      {
        return functor.template operator()<XYZA<T, WhitePoint::D65>>();
      }
    }

    krys_unreachable();
  }

  template <typename T, typename Functor>
  KRYS_NODISCARD constexpr decltype(auto) CallWithColourType(const ColourComponents<T, 4> &components,
                                                             ColourSpace colorSpace,
                                                             Functor &&functor) noexcept
  {
    return CallWithColourType<T>(
      colorSpace, [&]<typename ColourType>()
      { return std::invoke(std::forward<Functor>(functor), CreateFromComponents<ColourType>(components)); });
  }
}