#include "Krystal.Booey/Graphics/Color/ColorInterpolation.hpp"
#include "catch2/catch_approx.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/Graphics/Color/Color.hpp"
#include "Krystal.Booey/Graphics/Color/ColorInterpolationMethod.hpp"
#include "Krystal.Booey/Graphics/Color/ColorLuminance.hpp"
#include "Krystal.Booey/Graphics/Color/ColorSerialisation.hpp"
#include <cmath>
#include <limits>

namespace krys::boo::tests
{
  using Catch::Approx;

  TEST_CASE("ColorInterpolation: InterpolateColors", "[Graphics][Color][Interpolation]")
  {
    ColourInterpolationMethod srgb {ColourInterpolationMethod::SRGB {},
                                    AlphaPremultiplication::Premultiplied};
    auto mixed = InterpolateColors(srgb, Color::red, 0.5, Color::blue, 0.5);
    CHECK(mixed.Space() == ColourSpace::ExtendedSRGB);
    CHECK(SerialisationForCSS(mixed) == u8"color(srgb 0.5 0 0.5)");

    ColourInterpolationMethod oklab {ColourInterpolationMethod::OKLab {},
                                     AlphaPremultiplication::Unpremultiplied};
    auto oklabMixed = InterpolateColors(oklab, Color::white, 1.0, Color::black, 0.0);
    CHECK(oklabMixed.Space() == ColourSpace::OKLab);
    CHECK(oklabMixed.ToColorTypeLossy<OKLab<float>>().Resolved().Lightness == Approx(1.0f).margin(0.0001));
  }

  TEST_CASE("ColourInterpolationMethod: serialisation", "[Graphics][Color][Interpolation]")
  {
    STATIC_CHECK(SerialisationForCSS(ColourInterpolationColourSpace::SRGBLinear) == u8"srgb-linear");
    STATIC_CHECK(SerialisationForCSS(ColourInterpolationColourSpace::XYZD65) == u8"xyz-d65");

    auto serialise = [](ColourInterpolationMethod method)
    {
      return SerialisationForCSS(method);
    };

    CHECK(serialise({ColourInterpolationMethod::OKLab {}, AlphaPremultiplication::Premultiplied})
          == u8"oklab");
    CHECK(serialise({ColourInterpolationMethod::HSL {}, AlphaPremultiplication::Premultiplied}) == u8"hsl");
    CHECK(serialise({ColourInterpolationMethod::HSL {HueInterpolationMethod::Longer},
                     AlphaPremultiplication::Premultiplied})
          == u8"hsl longer hue");
    CHECK(serialise({ColourInterpolationMethod::OKLCH {HueInterpolationMethod::Increasing},
                     AlphaPremultiplication::Unpremultiplied})
          == u8"oklch increasing hue");
    CHECK(serialise({ColourInterpolationMethod::LCH {HueInterpolationMethod::Decreasing},
                     AlphaPremultiplication::Premultiplied})
          == u8"lch decreasing hue");
    CHECK(serialise({ColourInterpolationMethod::DisplayP3Linear {}, AlphaPremultiplication::Premultiplied})
          == u8"display-p3-linear");
  }

  TEST_CASE("ColorLuminance", "[Graphics][Color][Interpolation]")
  {
    CHECK(RelativeLuminance(Color {Color::white}) == Approx(1.0));
    CHECK(RelativeLuminance(Color {Color::black}) == Approx(0.0));

    CHECK(ContrastRatio(Color {Color::white}, Color {Color::black}) == Approx(21.0).margin(0.001));
    CHECK(ContrastRatio(Color {Color::red}, Color {Color::red}) == Approx(1.0));
  }
}
