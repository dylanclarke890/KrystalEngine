#include "Krystal.Core/Colour/ColourInterpolation.hpp"
#include "catch2/catch_approx.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Core/Colour/ColourLuminance.hpp"
#include <cmath>
#include <limits>

namespace krys::tests
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

  TEST_CASE("ColourInterpolation: hue fixup", "[Colour][Interpolation]")
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

  TEST_CASE("ColourInterpolation: component helpers", "[Colour][Interpolation]")
  {
    CHECK(InterpolateComponentWithoutAccountingForNaN(10, 0.25, 20, 0.75) == 17.5f);
    CHECK(InterpolateComponentAccountingForNaN(none, 0.5, 20, 0.5) == 20.0f);
    CHECK(InterpolateComponentAccountingForNaN(10, 0.5, none, 0.5) == 10.0f);
    CHECK(std::isnan(InterpolateComponentAccountingForNaN(none, 0.5, none, 0.5)));

    auto bothMissing = InterpolateAlphaPremulitplied(none, 0.5, none, 0.5);
    CHECK(bothMissing.AlphaForPremultiplicationOfColour1 == 1.0f);
    CHECK(std::isnan(bothMissing.ResultAlpha));

    auto oneMissing = InterpolateAlphaPremulitplied(none, 0.5, 0.25f, 0.5);
    CHECK(oneMissing.AlphaForPremultiplicationOfColour1 == 0.25f);
    CHECK(oneMissing.ResultAlpha == 0.25f);

    auto neither = InterpolateAlphaPremulitplied(1.0f, 0.5, 0.0f, 0.5);
    CHECK(neither.AlphaForPremultiplicationOfColour1 == 1.0f);
    CHECK(neither.AlphaForPremultiplicationOfColour2 == 0.0f);
    CHECK(neither.ResultAlpha == 0.5f);
  }

  TEST_CASE("ColourInterpolation: InterpolateColourComponents", "[Colour][Interpolation]")
  {
    using SRGB = ColourInterpolationMethod::SRGB;

    auto premultiplied = InterpolateColourComponents<AlphaPremultiplication::Premultiplied>(
      SRGB {}, ExtendedSRGBA<float> {1, 0, 0, 1}, 0.5, ExtendedSRGBA<float> {0, 0, 1, 0}, 0.5);
    auto [r, g, b, a] = premultiplied.Resolved();
    // The transparent color contributes nothing to the premultiplied components.
    CHECK(r == Approx(1.0f));
    CHECK(g == 0.0f);
    CHECK(b == 0.0f);
    CHECK(a == 0.5f);

    auto unpremultiplied = InterpolateColourComponents<AlphaPremultiplication::Unpremultiplied>(
      SRGB {}, ExtendedSRGBA<float> {1, 0, 0, 1}, 0.5, ExtendedSRGBA<float> {0, 0, 1, 0}, 0.5);
    auto [ur, ug, ub, ua] = unpremultiplied.Resolved();
    CHECK(ur == 0.5f);
    CHECK(ug == 0.0f);
    CHECK(ub == 0.5f);
    CHECK(ua == 0.5f);

    // Missing components take the other color's value.
    auto missing = InterpolateColourComponents<AlphaPremultiplication::Premultiplied>(
      SRGB {}, ExtendedSRGBA<float> {none, 0.5f, 0, 1}, 0.5, ExtendedSRGBA<float> {1, 0.5f, none, 1}, 0.5);
    auto [mr, mg, mb, ma] = missing.Unresolved();
    CHECK(mr == 1.0f);
    CHECK(mg == 0.5f);
    CHECK(mb == 0.0f);
    CHECK(ma == 1.0f);

    // Hues interpolate along the chosen arc.
    auto hsl = InterpolateColourComponents<AlphaPremultiplication::Premultiplied>(
      ColourInterpolationMethod::HSL {HueInterpolationMethod::Shorter}, HSLA<float> {350, 100, 50, 1}, 0.5,
      HSLA<float> {10, 100, 50, 1}, 0.5);
    CHECK(hsl.Resolved().Hue == Approx(0.0f).margin(0.0001));

    auto hslLonger = InterpolateColourComponents<AlphaPremultiplication::Premultiplied>(
      ColourInterpolationMethod::HSL {HueInterpolationMethod::Longer}, HSLA<float> {350, 100, 50, 1}, 0.5,
      HSLA<float> {10, 100, 50, 1}, 0.5);
    CHECK(hslLonger.Resolved().Hue == Approx(180.0f));

    auto added = AddColourComponents<AlphaPremultiplication::Unpremultiplied>(
      SRGB {}, ExtendedSRGBA<float> {0.25f, 0, 0, 0.5f}, ExtendedSRGBA<float> {0.5f, 0, 0, 0.75f});
    CHECK(added.Resolved().Red == 0.75f);
    CHECK(added.Resolved().Alpha == 1.0f);
  }

  TEST_CASE("ColourInterpolationMethod: equality and hash", "[Colour][Interpolation]")
  {
    ColourInterpolationMethod a {ColourInterpolationMethod::HSL {HueInterpolationMethod::Longer},
                                 AlphaPremultiplication::Premultiplied};
    ColourInterpolationMethod b {ColourInterpolationMethod::HSL {HueInterpolationMethod::Longer},
                                 AlphaPremultiplication::Premultiplied};
    ColourInterpolationMethod c {ColourInterpolationMethod::HSL {HueInterpolationMethod::Shorter},
                                 AlphaPremultiplication::Premultiplied};
    ColourInterpolationMethod d {ColourInterpolationMethod::HSL {HueInterpolationMethod::Longer},
                                 AlphaPremultiplication::Unpremultiplied};

    CHECK(a == b);
    CHECK(a != c);
    CHECK(a != d);

    auto hash = [](const ColourInterpolationMethod &method)
    {
      return DefaultHash<ColourInterpolationMethod>::Hash(method);
    };
    CHECK(hash(a) == hash(b));
    CHECK(hash(a) != hash(c));
    CHECK(hash(a) != hash(d));
    CHECK(hash({ColourInterpolationMethod::SRGB {}, AlphaPremultiplication::Premultiplied})
          != hash({ColourInterpolationMethod::OKLab {}, AlphaPremultiplication::Premultiplied}));
  }

  TEST_CASE("ColourLuminance", "[Colour][Interpolation]")
  {
    CHECK(RelativeLuminance(SRGBA<float> {1, 1, 1, 1}) == Approx(1.0));
    CHECK(RelativeLuminance(SRGBA<float> {0, 0, 0, 1}) == Approx(0.0));
    CHECK(RelativeLuminance(SRGBA<float> {1, 0, 0, 1}) == Approx(0.2126).margin(0.0001));

    CHECK(ContrastRatio(1.0, 0.0) == Approx(21.0));
    CHECK(ContrastRatio(0.0, 1.0) == Approx(21.0));
    CHECK(ContrastRatio(SRGBA<float> {1, 1, 1, 1}, SRGBA<float> {0, 0, 0, 1}) == Approx(21.0).margin(0.001));
  }
}
