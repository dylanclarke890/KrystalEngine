#include "Krystal.Booey/Graphics/Color/ColorInterpolationMethod.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo
{
  void SerialisationForCSS(StringBuilder &builder,
                           ColorInterpolationColorSpace interpolationColorSpace) noexcept
  {
    builder.Append(SerialisationForCSS(interpolationColorSpace));
  }

  void SerialisationForCSS(StringBuilder &builder, HueInterpolationMethod hueInterpolationMethod) noexcept
  {
    switch (hueInterpolationMethod)
    {
      case HueInterpolationMethod::Shorter:
      {
        break;
      }
      case HueInterpolationMethod::Longer:
      {
        builder.Append(u8" longer hue");
        break;
      }
      case HueInterpolationMethod::Increasing:
      {
        builder.Append(u8" increasing hue");
        break;
      }
      case HueInterpolationMethod::Decreasing:
      {
        builder.Append(u8" decreasing hue");
        break;
      }
    }
  }

  void SerialisationForCSS(StringBuilder &builder, const ColorInterpolationMethod &method) noexcept
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
    StringBuilder builder;
    SerialisationForCSS(builder, method);
    return krys::move(builder).ToString();
  }
}
