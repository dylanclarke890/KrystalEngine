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

  namespace
  {
    constexpr auto none = std::numeric_limits<float>::quiet_NaN();

    void CheckHueFixup(HueInterpolationMethod method, float component1, float component2, float expected1,
                       float expected2)
    {
      auto [fixed1, fixed2] = FixupHueComponentsPriorToInterpolation(method, component1, component2);
      CHECK(fixed1 == Approx(expected1));
      CHECK(fixed2 == Approx(expected2));
    }
  }

  TEST_CASE("ColorInterpolation: hue fixup", "[Graphics][Color][Interpolation]")
  {
    CheckHueFixup(HueInterpolationMethod::Shorter, 350, 10, 350, 370);
    CheckHueFixup(HueInterpolationMethod::Shorter, 10, 350, 370, 350);
    CheckHueFixup(HueInterpolationMethod::Shorter, 10, 20, 10, 20);
    CheckHueFixup(HueInterpolationMethod::Longer, 10, 20, 370, 20);
    CheckHueFixup(HueInterpolationMethod::Longer, 20, 10, 20, 370);
    CheckHueFixup(HueInterpolationMethod::Longer, 10, 350, 10, 350);
    CheckHueFixup(HueInterpolationMethod::Increasing, 20, 10, 20, 370);
    CheckHueFixup(HueInterpolationMethod::Increasing, 10, 20, 10, 20);
    CheckHueFixup(HueInterpolationMethod::Decreasing, 10, 20, 370, 20);
    CheckHueFixup(HueInterpolationMethod::Decreasing, 20, 10, 20, 10);

    // Hues are normalised to [0, 360) first.
    CheckHueFixup(HueInterpolationMethod::Shorter, -10, 370, 350, 370);
  }

  TEST_CASE("ColorInterpolation: component helpers", "[Graphics][Color][Interpolation]")
  {
    CHECK(InterpolateComponentWithoutAccountingForNaN(10, 0.25, 20, 0.75) == 17.5f);
    CHECK(InterpolateComponentAccountingForNaN(none, 0.5, 20, 0.5) == 20.0f);
    CHECK(InterpolateComponentAccountingForNaN(10, 0.5, none, 0.5) == 10.0f);
    CHECK(std::isnan(InterpolateComponentAccountingForNaN(none, 0.5, none, 0.5)));

    auto bothMissing = InterpolateAlphaPremulitplied(none, 0.5, none, 0.5);
    CHECK(bothMissing.AlphaForPremultiplicationOfColor1 == 1.0f);
    CHECK(std::isnan(bothMissing.ResultAlpha));

    auto oneMissing = InterpolateAlphaPremulitplied(none, 0.5, 0.25f, 0.5);
    CHECK(oneMissing.AlphaForPremultiplicationOfColor1 == 0.25f);
    CHECK(oneMissing.ResultAlpha == 0.25f);

    auto neither = InterpolateAlphaPremulitplied(1.0f, 0.5, 0.0f, 0.5);
    CHECK(neither.AlphaForPremultiplicationOfColor1 == 1.0f);
    CHECK(neither.AlphaForPremultiplicationOfColor2 == 0.0f);
    CHECK(neither.ResultAlpha == 0.5f);
  }

  TEST_CASE("ColorInterpolation: InterpolateColorComponents", "[Graphics][Color][Interpolation]")
  {
    using SRGB = ColorInterpolationMethod::SRGB;

    auto premultiplied = InterpolateColorComponents<AlphaPremultiplication::Premultiplied>(
      SRGB {}, ExtendedSRGBA<float> {1, 0, 0, 1}, 0.5, ExtendedSRGBA<float> {0, 0, 1, 0}, 0.5);
    auto [r, g, b, a] = premultiplied.Resolved();
    // The transparent color contributes nothing to the premultiplied components.
    CHECK(r == Approx(1.0f));
    CHECK(g == 0.0f);
    CHECK(b == 0.0f);
    CHECK(a == 0.5f);

    auto unpremultiplied = InterpolateColorComponents<AlphaPremultiplication::Unpremultiplied>(
      SRGB {}, ExtendedSRGBA<float> {1, 0, 0, 1}, 0.5, ExtendedSRGBA<float> {0, 0, 1, 0}, 0.5);
    auto [ur, ug, ub, ua] = unpremultiplied.Resolved();
    CHECK(ur == 0.5f);
    CHECK(ug == 0.0f);
    CHECK(ub == 0.5f);
    CHECK(ua == 0.5f);

    // Missing components take the other color's value.
    auto missing = InterpolateColorComponents<AlphaPremultiplication::Premultiplied>(
      SRGB {}, ExtendedSRGBA<float> {none, 0.5f, 0, 1}, 0.5, ExtendedSRGBA<float> {1, 0.5f, none, 1}, 0.5);
    auto [mr, mg, mb, ma] = missing.Unresolved();
    CHECK(mr == 1.0f);
    CHECK(mg == 0.5f);
    CHECK(mb == 0.0f);
    CHECK(ma == 1.0f);

    // Hues interpolate along the chosen arc.
    auto hsl = InterpolateColorComponents<AlphaPremultiplication::Premultiplied>(
      ColorInterpolationMethod::HSL {HueInterpolationMethod::Shorter}, HSLA<float> {350, 100, 50, 1}, 0.5,
      HSLA<float> {10, 100, 50, 1}, 0.5);
    CHECK(hsl.Resolved().Hue == Approx(0.0f).margin(0.0001));

    auto hslLonger = InterpolateColorComponents<AlphaPremultiplication::Premultiplied>(
      ColorInterpolationMethod::HSL {HueInterpolationMethod::Longer}, HSLA<float> {350, 100, 50, 1}, 0.5,
      HSLA<float> {10, 100, 50, 1}, 0.5);
    CHECK(hslLonger.Resolved().Hue == Approx(180.0f));

    auto added = AddColorComponents<AlphaPremultiplication::Unpremultiplied>(
      SRGB {}, ExtendedSRGBA<float> {0.25f, 0, 0, 0.5f}, ExtendedSRGBA<float> {0.5f, 0, 0, 0.75f});
    CHECK(added.Resolved().Red == 0.75f);
    CHECK(added.Resolved().Alpha == 1.0f);
  }

  TEST_CASE("ColorInterpolation: InterpolateColors", "[Graphics][Color][Interpolation]")
  {
    ColorInterpolationMethod srgb {ColorInterpolationMethod::SRGB {}, AlphaPremultiplication::Premultiplied};
    auto mixed = InterpolateColors(srgb, Color::red, 0.5, Color::blue, 0.5);
    CHECK(mixed.Space() == ColorSpace::ExtendedSRGB);
    CHECK(SerialisationForCSS(mixed) == u8"color(srgb 0.5 0 0.5)");

    ColorInterpolationMethod oklab {ColorInterpolationMethod::OKLab {},
                                    AlphaPremultiplication::Unpremultiplied};
    auto oklabMixed = InterpolateColors(oklab, Color::white, 1.0, Color::black, 0.0);
    CHECK(oklabMixed.Space() == ColorSpace::OKLab);
    CHECK(oklabMixed.ToColorTypeLossy<OKLab<float>>().Resolved().Lightness == Approx(1.0f).margin(0.0001));
  }

  TEST_CASE("ColorInterpolationMethod: serialisation", "[Graphics][Color][Interpolation]")
  {
    STATIC_CHECK(SerialisationForCSS(ColorInterpolationColorSpace::SRGBLinear) == u8"srgb-linear");
    STATIC_CHECK(SerialisationForCSS(ColorInterpolationColorSpace::XYZD65) == u8"xyz-d65");

    auto serialise = [](ColorInterpolationMethod method)
    {
      return SerialisationForCSS(method);
    };

    CHECK(serialise({ColorInterpolationMethod::OKLab {}, AlphaPremultiplication::Premultiplied})
          == u8"oklab");
    CHECK(serialise({ColorInterpolationMethod::HSL {}, AlphaPremultiplication::Premultiplied}) == u8"hsl");
    CHECK(serialise({ColorInterpolationMethod::HSL {HueInterpolationMethod::Longer},
                     AlphaPremultiplication::Premultiplied})
          == u8"hsl longer hue");
    CHECK(serialise({ColorInterpolationMethod::OKLCH {HueInterpolationMethod::Increasing},
                     AlphaPremultiplication::Unpremultiplied})
          == u8"oklch increasing hue");
    CHECK(serialise({ColorInterpolationMethod::LCH {HueInterpolationMethod::Decreasing},
                     AlphaPremultiplication::Premultiplied})
          == u8"lch decreasing hue");
    CHECK(serialise({ColorInterpolationMethod::DisplayP3Linear {}, AlphaPremultiplication::Premultiplied})
          == u8"display-p3-linear");
  }

  TEST_CASE("ColorInterpolationMethod: equality and hash", "[Graphics][Color][Interpolation]")
  {
    ColorInterpolationMethod a {ColorInterpolationMethod::HSL {HueInterpolationMethod::Longer},
                                AlphaPremultiplication::Premultiplied};
    ColorInterpolationMethod b {ColorInterpolationMethod::HSL {HueInterpolationMethod::Longer},
                                AlphaPremultiplication::Premultiplied};
    ColorInterpolationMethod c {ColorInterpolationMethod::HSL {HueInterpolationMethod::Shorter},
                                AlphaPremultiplication::Premultiplied};
    ColorInterpolationMethod d {ColorInterpolationMethod::HSL {HueInterpolationMethod::Longer},
                                AlphaPremultiplication::Unpremultiplied};

    CHECK(a == b);
    CHECK(a != c);
    CHECK(a != d);

    auto hash = [](const ColorInterpolationMethod &method)
    {
      return DefaultHash<ColorInterpolationMethod>::Hash(method);
    };
    CHECK(hash(a) == hash(b));
    CHECK(hash(a) != hash(c));
    CHECK(hash(a) != hash(d));
    CHECK(hash({ColorInterpolationMethod::SRGB {}, AlphaPremultiplication::Premultiplied})
          != hash({ColorInterpolationMethod::OKLab {}, AlphaPremultiplication::Premultiplied}));
  }

  TEST_CASE("ColorLuminance", "[Graphics][Color][Interpolation]")
  {
    CHECK(RelativeLuminance(Color {Color::white}) == Approx(1.0));
    CHECK(RelativeLuminance(Color {Color::black}) == Approx(0.0));
    CHECK(RelativeLuminance(SRGBA<float> {1, 0, 0, 1}) == Approx(0.2126).margin(0.0001));

    CHECK(ContrastRatio(1.0, 0.0) == Approx(21.0));
    CHECK(ContrastRatio(0.0, 1.0) == Approx(21.0));
    CHECK(ContrastRatio(Color {Color::white}, Color {Color::black}) == Approx(21.0).margin(0.001));
    CHECK(ContrastRatio(Color {Color::red}, Color {Color::red}) == Approx(1.0));
  }
}
