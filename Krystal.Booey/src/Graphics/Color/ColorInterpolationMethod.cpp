#include "Krystal.Booey/Graphics/Color/ColorInterpolationMethod.hpp"

namespace krys::boo
{
  void SerialisationForCSS(utf8_string &builder,
                           ColorInterpolationColorSpace interpolationColorSpace) noexcept
  {
    builder.append(SerialisationForCSS(interpolationColorSpace));
  }

  void SerialisationForCSS(utf8_string &builder, HueInterpolationMethod hueInterpolationMethod) noexcept
  {
    switch (hueInterpolationMethod)
    {
      case HueInterpolationMethod::Shorter:
      {
        break;
      }
      case HueInterpolationMethod::Longer:
      {
        builder.append(u8" longer hue");
        break;
      }
      case HueInterpolationMethod::Increasing:
      {
        builder.append(u8" increasing hue");
        break;
      }
      case HueInterpolationMethod::Decreasing:
      {
        builder.append(u8" decreasing hue");
        break;
      }
    }
  }

  void SerialisationForCSS(utf8_string &builder, const ColorInterpolationMethod &method) noexcept
  {
    krys::SwitchOn(method.ColorSpace,
                   [&]<typename MethodColorSpace>(const MethodColorSpace &type)
                   {
                     SerialisationForCSS(builder, type.InterpolationColorSpace);
                     if constexpr (hasHueInterpolationMethod<MethodColorSpace>)
                     {
                       SerialisationForCSS(builder, type.HueInterpolationMethod);
                     }
                   });
  }

  utf8_string SerialisationForCSS(const ColorInterpolationMethod &method) noexcept
  {
    utf8_string builder;
    SerialisationForCSS(builder, method);
    return builder;
  }
}
