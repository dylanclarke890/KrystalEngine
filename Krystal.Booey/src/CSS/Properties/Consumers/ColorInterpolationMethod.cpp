#include "Krystal.Booey/CSS/Properties/Consumers/ColorInterpolationMethod.hpp"
#include "Krystal.Booey/CSS/Parser/TokenRange.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Ident.hpp"
#include "Krystal.Booey/CSS/Values/ValueId.hpp"
#include "Krystal.Booey/Graphics/Color/ColorInterpolationMethod.hpp"

namespace krys::boo::css::PropertyParserHelpers
{
  // MARK: - color-mix()

  KRYS_NODISCARD static Maybe<HueInterpolationMethod>
    ConsumeHueInterpolationMethod(TokenRange &tokens) noexcept
  {
    auto method = [&]() -> Maybe<HueInterpolationMethod>
    {
      switch (tokens.Peek().ValueId())
      {
        case ValueId::Shorter:
        {
          return HueInterpolationMethod::Shorter;
        }
        case ValueId::Longer:
        {
          return HueInterpolationMethod::Longer;
        }
        case ValueId::Increasing:
        {
          return HueInterpolationMethod::Increasing;
        }
        case ValueId::Decreasing:
        {
          return HueInterpolationMethod::Decreasing;
        }
        default:
        {
          return null;
        }
      }
    }();

    if (method)
    {
      tokens.Discard();
      tokens.DiscardWhitespace();
    }

    return method;
  }

  Maybe<ColorInterpolationMethod> ConsumeColorInterpolationMethod(TokenRange &args,
                                                                  PropertyParserState &) noexcept
  {
    // <rectangular-color-space> = srgb | srgb-linear | display-p3 | a98-rgb | prophoto-rgb | rec2020 | lab |
    // oklab | xyz | xyz-d50 | xyz-d65
    // <polar-color-space> = hsl | hwb | lch | oklch
    // <hue-interpolation-method> = [ shorter | longer | increasing | decreasing ] hue
    // <color-interpolation-method> = in [ <rectangular-color-space> | <polar-color-space>
    // <hue-interpolation-method>? ]

    krys_debug_assert(args.Peek().ValueId() == ValueId::In);
    args.Discard();
    args.DiscardWhitespace();

    auto consumePolarColorSpace = [](TokenRange &args,
                                     auto colorInterpolationMethod) -> Maybe<ColorInterpolationMethod>
    {
      // Consume the color space identifier.
      args.Discard();
      args.DiscardWhitespace();

      // <hue-interpolation-method> is optional, so if it is not provided, we just use the default value
      // specified in the passed in 'colorInterpolationMethod' parameter.
      auto hueInterpolationMethod = ConsumeHueInterpolationMethod(args);
      if (!hueInterpolationMethod)
      {
        return ColorInterpolationMethod {colorInterpolationMethod, AlphaPremultiplication::Premultiplied};
      }

      // If the hue-interpolation-method was provided it must be followed immediately by the 'hue' identifier.
      if (!ConsumeIdentRaw<ValueId::Hue>(args))
      {
        return null;
      }

      colorInterpolationMethod.HueInterpolationMethod = *hueInterpolationMethod;

      return ColorInterpolationMethod {colorInterpolationMethod, AlphaPremultiplication::Premultiplied};
    };

    auto consumeRectangularColorSpace = [](TokenRange &args,
                                           auto colorInterpolationMethod) -> Maybe<ColorInterpolationMethod>
    {
      // Consume the color space identifier.
      args.Discard();
      args.DiscardWhitespace();

      return ColorInterpolationMethod {colorInterpolationMethod, AlphaPremultiplication::Premultiplied};
    };

    switch (args.Peek().ValueId())
    {
      case ValueId::Hsl:
      {
        return consumePolarColorSpace(args, ColorInterpolationMethod::HSL {});
      }
      case ValueId::Hwb:
      {
        return consumePolarColorSpace(args, ColorInterpolationMethod::HWB {});
      }
      case ValueId::Lch:
      {
        return consumePolarColorSpace(args, ColorInterpolationMethod::LCH {});
      }
      case ValueId::Lab:
      {
        return consumeRectangularColorSpace(args, ColorInterpolationMethod::Lab {});
      }
      case ValueId::Oklch:
      {
        return consumePolarColorSpace(args, ColorInterpolationMethod::OKLCH {});
      }
      case ValueId::Oklab:
      {
        return consumeRectangularColorSpace(args, ColorInterpolationMethod::OKLab {});
      }
      case ValueId::SRGB:
      {
        return consumeRectangularColorSpace(args, ColorInterpolationMethod::SRGB {});
      }
      case ValueId::SrgbLinear:
      {
        return consumeRectangularColorSpace(args, ColorInterpolationMethod::SRGBLinear {});
      }
      case ValueId::DisplayP3:
      {
        return consumeRectangularColorSpace(args, ColorInterpolationMethod::DisplayP3 {});
      }
      case ValueId::DisplayP3Linear:
      {
        return consumeRectangularColorSpace(args, ColorInterpolationMethod::DisplayP3Linear {});
      }
      case ValueId::A98Rgb:
      {
        return consumeRectangularColorSpace(args, ColorInterpolationMethod::A98RGB {});
      }
      case ValueId::ProphotoRgb:
      {
        return consumeRectangularColorSpace(args, ColorInterpolationMethod::ProPhotoRGB {});
      }
      case ValueId::Rec2020:
      {
        return consumeRectangularColorSpace(args, ColorInterpolationMethod::Rec2020 {});
      }
      case ValueId::XyzD50:
      {
        return consumeRectangularColorSpace(args, ColorInterpolationMethod::XYZD50 {});
      }
      case ValueId::Xyz:
      case ValueId::XyzD65:
      {
        return consumeRectangularColorSpace(args, ColorInterpolationMethod::XYZD65 {});
      }
      default:
      {
        return null;
      }
    }
  }
}
