#include "Krystal.Booey/Graphics/Color/ColorSerialisation.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/Graphics/Color/Color.hpp"
#include <limits>

namespace krys::boo::tests
{
  namespace
  {
    constexpr auto none = std::numeric_limits<float>::quiet_NaN();
    constexpr auto infinity = std::numeric_limits<float>::infinity();
  }

  TEST_CASE("ColorSerialisation: sRGB bytes for CSS", "[Graphics][Color][Serialisation]")
  {
    CHECK(SerialisationForCSS(Color::red) == u8"rgb(255, 0, 0)");
    CHECK(SerialisationForCSS(SRGBA<uint8> {1, 2, 3, 0}) == u8"rgba(1, 2, 3, 0)");
    CHECK(SerialisationForCSS(SRGBA<uint8> {255, 0, 0, 128}) == u8"rgba(255, 0, 0, 0.5)");
    CHECK(SerialisationForCSS(SRGBA<uint8> {255, 0, 0, 127}) == u8"rgba(255, 0, 0, 0.498)");
    CHECK(SerialisationForCSS(SRGBA<uint8> {255, 0, 0, 64}) == u8"rgba(255, 0, 0, 0.25)");
    CHECK(SerialisationForCSS(SRGBA<uint8> {255, 0, 0, 204}) == u8"rgba(255, 0, 0, 0.8)");
    CHECK(SerialisationForCSS(SRGBA<uint8> {255, 0, 0, 1}) == u8"rgba(255, 0, 0, 0.004)");
    CHECK(SerialisationForCSS(SRGBA<uint8> {255, 0, 0, 254}) == u8"rgba(255, 0, 0, 0.996)");
  }

  TEST_CASE("ColorSerialisation: sRGB alpha fractions round trip", "[Graphics][Color][Serialisation]")
  {
    // The shortest fraction that parses back to the same byte is chosen.
    for (uint32 alpha = 1; alpha < 255; ++alpha)
    {
      INFO(alpha);
      auto serialised = SerialisationForCSS(SRGBA<uint8> {0, 0, 0, static_cast<uint8>(alpha)});
      auto start = serialised.rfind(u8", ") + 2;
      auto fraction =
        std::stod(std::string(serialised.begin() + static_cast<ptrdiff_t>(start), serialised.end() - 1));
      CHECK(std::lround(fraction * 255) == static_cast<long>(alpha));
    }
  }

  TEST_CASE("ColorSerialisation: sRGB bytes for HTML and render tree", "[Graphics][Color][Serialisation]")
  {
    CHECK(SerialisationForHTML(SRGBA<uint8> {255, 0, 171}) == u8"#ff00ab");
    CHECK(SerialisationForHTML(SRGBA<uint8> {255, 0, 171, 128}) == u8"rgba(255, 0, 171, 0.5)");

    CHECK(SerialisationForRenderTreeAsText(SRGBA<uint8> {255, 0, 171}) == u8"#FF00AB");
    CHECK(SerialisationForRenderTreeAsText(SRGBA<uint8> {255, 0, 171, 128}) == u8"#FF00AB80");
  }

  TEST_CASE("ColorSerialisation: invalid color", "[Graphics][Color][Serialisation]")
  {
    CHECK(SerialisationForCSS(Color {}) == u8"rgba(0, 0, 0, 0)");
  }

  TEST_CASE("ColorSerialisation: color() function", "[Graphics][Color][Serialisation]")
  {
    CHECK(SerialisationForCSS(DisplayP3<float> {1, 0.5f, 0.25f, 1}) == u8"color(display-p3 1 0.5 0.25)");
    CHECK(SerialisationForCSS(DisplayP3<float> {1, 0.5f, 0.25f, 0.5f})
          == u8"color(display-p3 1 0.5 0.25 / 0.5)");
    CHECK(SerialisationForCSS(LinearSRGBA<float> {0.1f, 0.2f, 0.3f, 1})
          == u8"color(srgb-linear 0.1 0.2 0.3)");
    CHECK(SerialisationForCSS(A98RGB<float> {0, 0, 0, 1}) == u8"color(a98-rgb 0 0 0)");
    CHECK(SerialisationForCSS(ProPhotoRGB<float> {0, 0, 0, 1}) == u8"color(prophoto-rgb 0 0 0)");
    CHECK(SerialisationForCSS(Rec2020<float> {0, 0, 0, 1}) == u8"color(rec2020 0 0 0)");
    CHECK(SerialisationForCSS(LinearDisplayP3<float> {0, 0, 0, 1}) == u8"color(display-p3-linear 0 0 0)");
    CHECK(SerialisationForCSS(XYZA<float, WhitePoint::D50> {0.1f, 0.2f, 0.3f, 1})
          == u8"color(xyz-d50 0.1 0.2 0.3)");
    CHECK(SerialisationForCSS(XYZA<float, WhitePoint::D65> {0.1f, 0.2f, 0.3f, 1})
          == u8"color(xyz-d65 0.1 0.2 0.3)");

    // Extended types share their bounded counterpart's name.
    CHECK(SerialisationForCSS(ExtendedSRGBA<float> {1.5f, -0.5f, 0, 1}) == u8"color(srgb 1.5 -0.5 0)");
    CHECK(SerialisationForCSS(ExtendedDisplayP3<float> {0, 0, 0, 1}) == u8"color(display-p3 0 0 0)");

    // Missing and non-finite components.
    CHECK(SerialisationForCSS(ExtendedSRGBA<float> {none, 0, 0, none}) == u8"color(srgb none 0 0 / none)");
    CHECK(SerialisationForCSS(ExtendedSRGBA<float> {infinity, -infinity, 0, 1})
          == u8"color(srgb calc(infinity) calc(-infinity) 0)");

    // These are used in every context.
    CHECK(SerialisationForHTML(DisplayP3<float> {1, 0, 0, 1}) == u8"color(display-p3 1 0 0)");
    CHECK(SerialisationForRenderTreeAsText(DisplayP3<float> {1, 0, 0, 1}) == u8"color(display-p3 1 0 0)");
  }

  TEST_CASE("ColorSerialisation: sRGB floats", "[Graphics][Color][Serialisation]")
  {
    CHECK(SerialisationForCSS(SRGBA<float> {1, 0, 0, 1}) == u8"rgb(255, 0, 0)");
    CHECK(SerialisationForHTML(SRGBA<float> {1, 0, 0, 1}) == u8"#ff0000");
    CHECK(SerialisationForRenderTreeAsText(SRGBA<float> {1, 0, 0, 1}) == u8"rgb(255, 0, 0)");

    Color colorFunction {SRGBA<float> {1, 0, 0, 1}, ColorFlags::UseColorFunctionSerialization};
    CHECK(SerialisationForCSS(colorFunction) == u8"color(srgb 1 0 0)");
    CHECK(SerialisationForHTML(colorFunction) == u8"color(srgb 1 0 0)");
    CHECK(SerialisationForRenderTreeAsText(colorFunction) == u8"color(srgb 1 0 0)");

    Color bytesColorFunction {SRGBA<uint8> {255, 0, 0, 255}, ColorFlags::UseColorFunctionSerialization};
    CHECK(SerialisationForCSS(bytesColorFunction) == u8"color(srgb 1 0 0)");
  }

  TEST_CASE("ColorSerialisation: HSL and HWB", "[Graphics][Color][Serialisation]")
  {
    CHECK(SerialisationForCSS(HSLA<float> {120, 100, 50, 1}) == u8"rgb(0, 255, 0)");
    CHECK(SerialisationForCSS(HWBA<float> {240, 0, 0, 0.5f}) == u8"rgba(0, 0, 255, 0.5)");
    CHECK(SerialisationForHTML(HSLA<float> {0, 100, 50, 1}) == u8"#ff0000");
    CHECK(SerialisationForRenderTreeAsText(HSLA<float> {0, 100, 50, 1}) == u8"#FF0000");

    // With color() serialisation, missing components are carried forward into sRGB.
    Color hsl {HSLA<float> {0, 0, 100, 1}, ColorFlags::UseColorFunctionSerialization};
    CHECK(SerialisationForCSS(hsl) == u8"color(srgb 1 1 1)");
  }

  TEST_CASE("ColorSerialisation: Lab-like and LCH-like", "[Graphics][Color][Serialisation]")
  {
    CHECK(SerialisationForCSS(Lab<float> {50, 20, -30, 1}) == u8"lab(50 20 -30)");
    CHECK(SerialisationForCSS(Lab<float> {50, 20, -30, 0.5f}) == u8"lab(50 20 -30 / 0.5)");
    CHECK(SerialisationForCSS(OKLab<float> {0.5f, 0.25f, -0.125f, 1}) == u8"oklab(0.5 0.25 -0.125)");

    // Hues are normalised to [0, 360).
    CHECK(SerialisationForCSS(LCHA<float> {50, 30, 400, 1}) == u8"lch(50 30 40)");
    CHECK(SerialisationForCSS(OKLCHA<float> {0.5f, 0.125f, -90, 1}) == u8"oklch(0.5 0.125 270)");
    CHECK(SerialisationForCSS(OKLCHA<float> {0.5f, 0.125f, none, 1}) == u8"oklch(0.5 0.125 none)");

    // HTML and the render tree use the CSS serialisation.
    CHECK(SerialisationForHTML(Lab<float> {50, 20, -30, 1}) == u8"lab(50 20 -30)");
    CHECK(SerialisationForRenderTreeAsText(LCHA<float> {50, 30, 40, 1}) == u8"lch(50 30 40)");
  }

  TEST_CASE("ColorSerialisation: color space names", "[Graphics][Color][Serialisation]")
  {
    CHECK(Serialisation(ColorSpace::SRGB) == u8"srgb");
    CHECK(Serialisation(ColorSpace::ExtendedSRGB) == u8"srgb");
    CHECK(Serialisation(ColorSpace::LinearSRGB) == u8"srgb-linear");
    CHECK(Serialisation(ColorSpace::ExtendedLinearSRGB) == u8"srgb-linear");
    CHECK(Serialisation(ColorSpace::DisplayP3) == u8"display-p3");
    CHECK(Serialisation(ColorSpace::LinearDisplayP3) == u8"display-p3-linear");
    CHECK(Serialisation(ColorSpace::A98RGB) == u8"a98-rgb");
    CHECK(Serialisation(ColorSpace::ProPhotoRGB) == u8"prophoto-rgb");
    CHECK(Serialisation(ColorSpace::Rec2020) == u8"rec2020");
    CHECK(Serialisation(ColorSpace::HSL) == u8"hsl");
    CHECK(Serialisation(ColorSpace::HWB) == u8"hwb");
    CHECK(Serialisation(ColorSpace::Lab) == u8"lab");
    CHECK(Serialisation(ColorSpace::LCH) == u8"lch");
    CHECK(Serialisation(ColorSpace::OKLab) == u8"oklab");
    CHECK(Serialisation(ColorSpace::OKLCH) == u8"oklch");
    CHECK(Serialisation(ColorSpace::XYZ_D50) == u8"xyz-d50");
    CHECK(Serialisation(ColorSpace::XYZ_D65) == u8"xyz-d65");
  }
}
