#include "Krystal.Core/Colour/ColourTextStream.hpp"
#include "Krystal.Core/Text/TextStream.hpp"

namespace krys
{
  TextStream &operator<<(TextStream &ts, AlphaPremultiplication premultiplication) noexcept
  {
    switch (premultiplication)
    {
      case AlphaPremultiplication::Premultiplied:   ts << "premultiplied"; break;
      case AlphaPremultiplication::Unpremultiplied: ts << "unpremultiplied"; break;
    }
    return ts;
  }

  TextStream &operator<<(TextStream &ts, BlendMode blendMode) noexcept
  {
    return ts << BlendModeName(blendMode);
  }

  TextStream &operator<<(TextStream &ts, GradientSpreadMethod spreadMethod) noexcept
  {
    switch (spreadMethod)
    {
      case GradientSpreadMethod::Pad:     ts << "pad"; break;
      case GradientSpreadMethod::Reflect: ts << "reflect"; break;
      case GradientSpreadMethod::Repeat:  ts << "repeat"; break;
    }
    return ts;
  }

  TextStream &operator<<(TextStream &ts, ColourInterpolationColourSpace interpolationColorSpace) noexcept
  {
    switch (interpolationColorSpace)
    {
      case ColourInterpolationColourSpace::HSL:             ts << "HSL"; break;
      case ColourInterpolationColourSpace::HWB:             ts << "HWB"; break;
      case ColourInterpolationColourSpace::LCH:             ts << "LCH"; break;
      case ColourInterpolationColourSpace::Lab:             ts << "Lab"; break;
      case ColourInterpolationColourSpace::OKLCH:           ts << "OKLCH"; break;
      case ColourInterpolationColourSpace::OKLab:           ts << "OKLab"; break;
      case ColourInterpolationColourSpace::SRGB:            ts << "sRGB"; break;
      case ColourInterpolationColourSpace::SRGBLinear:      ts << "sRGB linear"; break;
      case ColourInterpolationColourSpace::DisplayP3:       ts << "Display P3"; break;
      case ColourInterpolationColourSpace::DisplayP3Linear: ts << "Display P3 Linear"; break;
      case ColourInterpolationColourSpace::A98RGB:          ts << "A98 RGB"; break;
      case ColourInterpolationColourSpace::ProPhotoRGB:     ts << "ProPhoto RGB"; break;
      case ColourInterpolationColourSpace::Rec2020:         ts << "Rec2020"; break;
      case ColourInterpolationColourSpace::XYZD50:          ts << "XYZ D50"; break;
      case ColourInterpolationColourSpace::XYZD65:          ts << "XYZ D65"; break;
    }
    return ts;
  }

  TextStream &operator<<(TextStream &ts, HueInterpolationMethod hueInterpolationMethod) noexcept
  {
    switch (hueInterpolationMethod)
    {
      case HueInterpolationMethod::Shorter:    ts << "shorter"; break;
      case HueInterpolationMethod::Longer:     ts << "longer"; break;
      case HueInterpolationMethod::Increasing: ts << "increasing"; break;
      case HueInterpolationMethod::Decreasing: ts << "decreasing"; break;
    }
    return ts;
  }

  TextStream &operator<<(TextStream &ts, const ColourInterpolationMethod &method) noexcept
  {
    std::visit(
      [&]<typename TColourSpace>(const TColourSpace &type)
      {
        ts << TColourSpace::InterpolationColourSpace;
        if constexpr (hasHueInterpolationMethod<TColourSpace>)
        {
          ts << ' ' << type.HueInterpolationMethod;
        }
        ts << ' ' << method.AlphaPremultiplication;
      },
      method.ColourSpace);
    return ts;
  }
}
