#include "Krystal.Booey/Graphics/Color/ColorInterpolationMethod.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo
{
  void SerialisationForCSS(StringBuilder &builder,
                           ColourInterpolationColourSpace interpolationColorSpace) noexcept
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

  void SerialisationForCSS(StringBuilder &builder, const ColourInterpolationMethod &method) noexcept
  {
    krys::SwitchOn(method.ColourSpace,
                   [&]<typename MethodColourSpace>(const MethodColourSpace &type)
                   {
                     SerialisationForCSS(builder, type.InterpolationColourSpace);
                     if constexpr (hasHueInterpolationMethod<MethodColourSpace>)
                     {
                       SerialisationForCSS(builder, type.HueInterpolationMethod);
                     }
                   });
  }

  utf8_string SerialisationForCSS(const ColourInterpolationMethod &method) noexcept
  {
    StringBuilder builder;
    SerialisationForCSS(builder, method);
    return krys::move(builder).ToString();
  }
}
