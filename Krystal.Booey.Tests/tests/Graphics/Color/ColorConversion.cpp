#include "Krystal.Booey/Graphics/Color/ColorConversion.hpp"
#include "catch2/catch_approx.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/Graphics/Color/ColorSpace.hpp"
#include <cmath>
#include <limits>

namespace krys::boo::tests
{
  using Catch::Approx;

  namespace
  {
    constexpr auto none = std::numeric_limits<float>::quiet_NaN();

    template <typename ColorType>
    void CheckComponents(const ColorType &color, float c1, float c2, float c3, float alpha, double margin)
    {
      auto [r1, r2, r3, ra] = color.Unresolved();
      CHECK(r1 == Approx(c1).margin(margin));
      CHECK(r2 == Approx(c2).margin(margin));
      CHECK(r3 == Approx(c3).margin(margin));
      CHECK(ra == Approx(alpha).margin(margin));
    }
  }

  TEST_CASE("ColorConversion: sRGB to Lab, LCH, OKLab and OKLCH", "[Graphics][Color][Conversion]")
  {
    // Reference values from CSS Color 4 / color.js.
    SRGBA<float> red {1, 0, 0, 1};
    CheckComponents(ConvertColor<Lab<float>>(red), 54.29f, 80.82f, 69.91f, 1, 0.05);
    CheckComponents(ConvertColor<LCHA<float>>(red), 54.29f, 106.84f, 40.86f, 1, 0.05);
    CheckComponents(ConvertColor<OKLab<float>>(red), 0.62796f, 0.22486f, 0.12585f, 1, 0.0005);
    CheckComponents(ConvertColor<OKLCHA<float>>(red), 0.62796f, 0.25768f, 29.234f, 1, 0.005);

    // XYZ D65 of white is the D65 white point.
    CheckComponents(ConvertColor<XYZA<float, WhitePoint::D65>>(SRGBA<float> {1, 1, 1, 1}), D65WhitePoint[0],
                    D65WhitePoint[1], D65WhitePoint[2], 1, 0.0005);
    CheckComponents(ConvertColor<XYZA<float, WhitePoint::D50>>(SRGBA<float> {1, 1, 1, 1}), D50WhitePoint[0],
                    D50WhitePoint[1], D50WhitePoint[2], 1, 0.0005);
  }

  TEST_CASE("ColorConversion: round trips", "[Graphics][Color][Conversion]")
  {
    SRGBA<float> color {0.25f, 0.5f, 0.75f, 0.5f};

    CheckComponents(ConvertColor<SRGBA<float>>(ConvertColor<Lab<float>>(color)), 0.25f, 0.5f, 0.75f, 0.5f,
                    0.0005);
    CheckComponents(ConvertColor<SRGBA<float>>(ConvertColor<OKLCHA<float>>(color)), 0.25f, 0.5f, 0.75f, 0.5f,
                    0.0005);
    CheckComponents(ConvertColor<SRGBA<float>>(ConvertColor<HWBA<float>>(color)), 0.25f, 0.5f, 0.75f, 0.5f,
                    0.0005);
    CheckComponents(ConvertColor<SRGBA<float>>(ConvertColor<DisplayP3<float>>(color)), 0.25f, 0.5f, 0.75f,
                    0.5f, 0.0005);
    CheckComponents(ConvertColor<SRGBA<float>>(ConvertColor<Rec2020<float>>(color)), 0.25f, 0.5f, 0.75f, 0.5f,
                    0.0005);
    CheckComponents(ConvertColor<SRGBA<float>>(ConvertColor<ProPhotoRGB<float>>(color)), 0.25f, 0.5f, 0.75f,
                    0.5f, 0.0005);
    CheckComponents(ConvertColor<SRGBA<float>>(ConvertColor<A98RGB<float>>(color)), 0.25f, 0.5f, 0.75f, 0.5f,
                    0.0005);
    CheckComponents(ConvertColor<SRGBA<float>>(ConvertColor<XYZA<float, WhitePoint::D50>>(color)), 0.25f,
                    0.5f, 0.75f, 0.5f, 0.0005);
  }

  TEST_CASE("ColorConversion: bytes", "[Graphics][Color][Conversion]")
  {
    auto bytes = ConvertColor<SRGBA<uint8>>(SRGBA<float> {1, 0.5f, 0, 0.25f}).Resolved();
    CHECK(bytes.Red == 255);
    CHECK(bytes.Green == 128);
    CHECK(bytes.Blue == 0);
    CHECK(bytes.Alpha == 64);

    CheckComponents(ConvertColor<SRGBA<float>>(SRGBA<uint8> {255, 51, 0, 255}), 1, 0.2f, 0, 1, 0.000001);

    // Out of gamut values are clipped on the way to bytes.
    auto clipped = ConvertColor<SRGBA<uint8>>(ExtendedSRGBA<float> {1.5f, -0.5f, 0.5f, 1}).Resolved();
    CHECK(clipped.Red == 255);
    CHECK(clipped.Green == 0);
    CHECK(clipped.Blue == 128);
  }

  TEST_CASE("ColorConversion: HSL and HWB", "[Graphics][Color][Conversion]")
  {
    CheckComponents(ConvertColor<ExtendedSRGBA<float>>(HSLA<float> {120, 100, 25, 1}), 0, 0.5f, 0, 1,
                    0.000001);
    CheckComponents(ConvertColor<ExtendedSRGBA<float>>(HWBA<float> {0, 50, 50, 1}), 0.5f, 0.5f, 0.5f, 1,
                    0.000001);
    CheckComponents(ConvertColor<ExtendedSRGBA<float>>(HWBA<float> {0, 25, 25, 1}), 0.75f, 0.25f, 0.25f, 1,
                    0.000001);

    // Achromatic colors have a missing hue.
    auto grey = ConvertColor<HSLA<float>>(SRGBA<float> {0.5f, 0.5f, 0.5f, 1});
    CHECK(std::isnan(grey.Unresolved().Hue));
    CHECK(std::isnan(ConvertColor<HWBA<float>>(SRGBA<float> {0.5f, 0.5f, 0.5f, 1}).Unresolved().Hue));
    CHECK(std::isnan(ConvertColor<LCHA<float>>(Lab<float> {50, 0, 0, 1}).Unresolved().Hue));
    CheckComponents(ConvertColor<Lab<float>>(LCHA<float> {50, 10, none, 1}), 50, 0, 0, 1, 0.000001);
  }

  TEST_CASE("ColorConversion: AnalogousComponentIndex", "[Graphics][Color][Conversion]")
  {
    // The hue of HSLA (0) is the hue of LCHA (2).
    STATIC_CHECK(AnalogousComponentIndex<LCHA<float>, HSLA<float>, 0>() == 2u);
    // Saturation (colorfulness) is LCH chroma.
    STATIC_CHECK(AnalogousComponentIndex<LCHA<float>, HSLA<float>, 1>() == 1u);
    // HWB has no colorfulness component.
    STATIC_CHECK(!AnalogousComponentIndex<HWBA<float>, HSLA<float>, 1>());
    // Alpha always carries forward.
    STATIC_CHECK(AnalogousComponentIndex<XYZA<float, WhitePoint::D50>, HSLA<float>, 3>() == 3u);
    // Red is analogous to x.
    STATIC_CHECK(AnalogousComponentIndex<XYZA<float, WhitePoint::D50>, SRGBA<float>, 0>() == 0u);
  }

  TEST_CASE("ColorConversion: carrying forward missing components", "[Graphics][Color][Conversion]")
  {
    auto lch = ConvertColorCarryingForwardMissing<LCHA<float>>(HSLA<float> {none, 50, 50, none});
    CHECK(std::isnan(lch.Unresolved().Hue));
    CHECK(std::isnan(lch.Unresolved().Alpha));
    CHECK_FALSE(std::isnan(lch.Unresolved().Lightness));

    // Plain conversion resolves missing components.
    CHECK_FALSE(std::isnan(ConvertColor<LCHA<float>>(HSLA<float> {none, 50, 50, none}).Unresolved().Alpha));

    // Same type and byte conversions don't carry forward.
    auto same = ConvertColorCarryingForwardMissing<HSLA<float>>(HSLA<float> {none, 50, 50, 1});
    CHECK(std::isnan(same.Unresolved().Hue));
  }

  TEST_CASE("ColorConversion: ConvertAndResolveColorComponents", "[Graphics][Color][Conversion]")
  {
    auto [r, g, b, a] =
      ConvertAndResolveColorComponents(ColorSpace::LinearSRGB, {1.0f, 0.0f, 0.0f, 1.0f}, ColorSpace::SRGB);
    CHECK(r == Approx(1.0f).margin(0.000001));
    CHECK(g == 0.0f);
    CHECK(b == 0.0f);
    CHECK(a == 1.0f);

    auto resolved =
      ConvertAndResolveColorComponents(ColorSpace::SRGB, {none, 0.5f, 0.0f, none}, ColorSpace::SRGB);
    CHECK(resolved == ColorComponents<float, 4> {0.0f, 0.5f, 0.0f, 0.0f});
  }

  TEST_CASE("ColorConversion: ComputeDeltaEOK", "[Graphics][Color][Conversion]")
  {
    CHECK(ComputeDeltaEOK(SRGBA<float> {1, 0, 0, 1}, SRGBA<float> {1, 0, 0, 1}) == 0.0f);
    CHECK(ComputeDeltaEOK(SRGBA<float> {1, 0, 0, 1}, SRGBA<float> {0, 0, 1, 1}) > 0.0f);
  }

  TEST_CASE("ColorConversion: ComputeDeltaEOK of black and white is 1", "[Graphics][Color][Conversion]")
  {
    CHECK(ComputeDeltaEOK(SRGBA<float> {1, 1, 1, 1}, SRGBA<float> {0, 0, 0, 1})
          == Approx(1.0).margin(0.0005));
  }

  TEST_CASE("ColorSpace: ColorSpaceFor and CallWithColorType", "[Graphics][Color][Conversion]")
  {
    STATIC_CHECK(ColorSpaceFor<SRGBA<uint8>> == ColorSpace::SRGB);
    STATIC_CHECK(ColorSpaceFor<ExtendedLinearSRGBA<float>> == ColorSpace::ExtendedLinearSRGB);
    STATIC_CHECK(ColorSpaceFor<XYZA<float, WhitePoint::D65>> == ColorSpace::XYZ_D65);
    STATIC_CHECK(ColorSpaceFor<OKLCHA<float>> == ColorSpace::OKLCH);

    for (uint8 i = 0; i <= static_cast<uint8>(ColorSpace::XYZ_D65); ++i)
    {
      INFO(static_cast<uint32>(i));
      auto colorSpace = static_cast<ColorSpace>(i);
      auto roundTripped =
        CallWithColorType<float>(colorSpace, []<typename ColorType>() { return ColorSpaceFor<ColorType>; });
      CHECK(roundTripped == colorSpace);
    }
  }
}
