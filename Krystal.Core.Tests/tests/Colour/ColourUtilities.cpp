#include "Krystal.Core/Colour/ColourUtilities.hpp"
#include "catch2/catch_approx.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Core/Colour/ColourMatrix.hpp"
#include "Krystal.Core/Colour/ColourNormalization.hpp"
#include "Krystal.Core/Colour/ColourTransferFunctions.hpp"
#include <cmath>

namespace krys::tests
{
  using Catch::Approx;

  TEST_CASE("ColourUtilities: alpha conversions", "[Colour][Utilities]")
  {
    STATIC_CHECK(ConvertByteAlphaTo<uint8>(128) == 128);
    STATIC_CHECK(ConvertByteAlphaTo<float>(255) == 1.0f);
    STATIC_CHECK(ConvertByteAlphaTo<float>(0) == 0.0f);
    CHECK(ConvertByteAlphaTo<float>(51) == 0.2f);

    CHECK(ConvertFloatAlphaTo<uint8>(1.0f) == 255);
    CHECK(ConvertFloatAlphaTo<uint8>(0.5f) == 128);
    CHECK(ConvertFloatAlphaTo<uint8>(2.0f) == 255);
    CHECK(ConvertFloatAlphaTo<uint8>(-1.0f) == 0);
    CHECK(ConvertFloatAlphaTo<float>(0.25f) == 0.25f);
    CHECK(ConvertFloatAlphaTo<float>(1.5f) == 1.0f);
    CHECK(ConvertFloatAlphaTo<float>(-0.5f) == 0.0f);

    CHECK(ConvertPrescaledSRGBAFloatToSRGBAByte(127.5f) == 128);
    CHECK(ConvertPrescaledSRGBAFloatToSRGBAByte(300.0f) == 255);
  }

  TEST_CASE("ColourUtilities: overridden alpha", "[Colour][Utilities]")
  {
    // ColourWithOverriddenAlpha returns the unresolved color type; slice it back to compare.
    STATIC_CHECK(SRGBA<uint8> {ColourWithOverriddenAlpha(SRGBA<uint8> {1, 2, 3, 4}, uint8 {200})}
                 == SRGBA<uint8> {1, 2, 3, 200});
    CHECK(SRGBA<uint8> {ColourWithOverriddenAlpha(SRGBA<uint8> {1, 2, 3, 4}, 1.0f)}
          == SRGBA<uint8> {1, 2, 3, 255});
    CHECK(SRGBA<float> {ColourWithOverriddenAlpha(SRGBA<float> {1, 0, 0, 1}, uint8 {51})}
          == SRGBA<float> {1, 0, 0, 0.2f});
    CHECK(SRGBA<float> {ColourWithOverriddenAlpha(SRGBA<float> {1, 0, 0, 1}, 0.5f)}
          == SRGBA<float> {1, 0, 0, 0.5f});

    STATIC_CHECK(InvertedColourWithOverriddenAlpha(SRGBA<uint8> {255, 0, 55, 4}, uint8 {255})
                 == SRGBA<uint8> {0, 255, 200, 255});
    CHECK(InvertedColourWithOverriddenAlpha(SRGBA<float> {1, 0, 0.25f, 1}, 0.5f)
          == SRGBA<float> {0, 1, 0.75f, 0.5f});

    STATIC_CHECK(SRGBA<uint8> {1, 2, 3}.ColourWithAlphaByte(9) == SRGBA<uint8> {1, 2, 3, 9});

    auto doubled = ColourByModifyingEachNonAlphaComponent(SRGBA<float> {0.25f, 0.5f, 0, 0.5f},
                                                          [](float c) { return c * 2; });
    CHECK(doubled == SRGBA<float> {0.5f, 1, 0, 0.5f});
  }

  TEST_CASE("ColourUtilities: premultiplication", "[Colour][Utilities]")
  {
    CHECK(Premultiplied(SRGBA<float> {1, 0.5f, 0, 0.5f}) == SRGBA<float> {0.5f, 0.25f, 0, 0.5f});
    CHECK(Unpremultiplied(SRGBA<float> {0.5f, 0.25f, 0, 0.5f}) == SRGBA<float> {1, 0.5f, 0, 0.5f});
    CHECK(Unpremultiplied(SRGBA<float> {0.5f, 0.25f, 0, 0}) == SRGBA<float> {0.5f, 0.25f, 0, 0});

    STATIC_CHECK(PremultipliedFlooring(SRGBA<uint8> {255, 128, 0, 128}) == SRGBA<uint8> {128, 64, 0, 128});
    STATIC_CHECK(PremultipliedCeiling(SRGBA<uint8> {255, 128, 0, 128}) == SRGBA<uint8> {128, 65, 0, 128});
    STATIC_CHECK(PremultipliedFlooring(SRGBA<uint8> {255, 128, 0, 0}) == SRGBA<uint8> {0, 0, 0, 0});
    STATIC_CHECK(PremultipliedCeiling(SRGBA<uint8> {255, 128, 0, 255}) == SRGBA<uint8> {255, 128, 0, 255});
    STATIC_CHECK(Unpremultiplied(SRGBA<uint8> {128, 64, 0, 128}) == SRGBA<uint8> {255, 128, 0, 128});
    STATIC_CHECK(Unpremultiplied(SRGBA<uint8> {1, 2, 3, 0}) == SRGBA<uint8> {1, 2, 3, 0});
  }

  TEST_CASE("ColourUtilities: black and white", "[Colour][Utilities]")
  {
    STATIC_CHECK(IsBlack(SRGBA<uint8> {0, 0, 0}));
    STATIC_CHECK_FALSE(IsBlack(SRGBA<uint8> {0, 0, 0, 254}));
    STATIC_CHECK(IsWhite(SRGBA<uint8> {255, 255, 255}));
    STATIC_CHECK(IsWhite(SRGBA<float> {1, 1, 1, 1}));
    STATIC_CHECK(IsBlack(LCHA<float> {0, 50, 120, 1}));
    STATIC_CHECK(IsWhite(LCHA<float> {100, 0, 0, 1}));
    STATIC_CHECK(IsWhite(OKLCHA<float> {1, 0, 0, 1}));
    STATIC_CHECK(IsBlack(HSLA<float> {0, 0, 0, 1}));
    STATIC_CHECK(IsWhite(HWBA<float> {0, 100, 0, 1}));
    STATIC_CHECK(IsBlack(XYZA<float, WhitePoint::D50> {0, 0, 0, 1}));
    STATIC_CHECK(IsWhite(XYZA<float, WhitePoint::D65> {0.95f, 1, 1.09f, 1}));
  }

  TEST_CASE("ColorNormalization", "[Colour][Utilities]")
  {
    CHECK(NormalizeHue(0.0f) == 0.0f);
    CHECK(NormalizeHue(360.0f) == 0.0f);
    CHECK(NormalizeHue(400.0f) == 40.0f);
    CHECK(NormalizeHue(-90.0f) == 270.0f);
    CHECK(std::isnan(NormalizeHue(std::numeric_limits<float>::quiet_NaN())));

    auto hsl = CreateColourTypeByNormalizingComponents<HSLA<float>>({-30.0f, -10.0f, 50.0f, 2.0f});
    CHECK(hsl == HSLA<float> {330, 0, 50, 1});
  }

  TEST_CASE("ColorTransferFunctions: sRGB", "[Colour][Utilities]")
  {
    using Clamped = SRGBTransferFunction<float, TransferFunctionMode::Clamped>;
    using Unclamped = SRGBTransferFunction<float, TransferFunctionMode::Unclamped>;

    CHECK(Clamped::ToLinear(0.0f) == 0.0f);
    CHECK(Clamped::ToLinear(0.5f) == Approx(0.214041).margin(0.000001));
    CHECK(Clamped::ToGammaEncoded(0.214041f) == Approx(0.5).margin(0.000001));
    CHECK(Clamped::ToLinear(2.0f) == 1.0f);
    CHECK(Clamped::ToGammaEncoded(-0.5f) == 0.0f);

    CHECK(Unclamped::ToLinear(-0.5f) == Approx(-0.214041).margin(0.000001));
    CHECK(Unclamped::ToLinear(2.0f) > 1.0f);
  }

  TEST_CASE("ColorTransferFunctions: unclamped functions are odd", "[Colour][Utilities]")
  {
    // Regression: the unclamped ProPhotoRGB and Rec2020 functions passed negative values to pow(), giving
    // NaN.
    auto checkOdd = [](auto function)
    {
      for (float c : {0.001f, 0.02f, 0.25f, 0.5f, 1.0f, 1.5f})
      {
        INFO(c);
        auto positive = function(c);
        auto negative = function(-c);
        CHECK_FALSE(std::isnan(negative));
        CHECK(negative == Approx(-positive).margin(0.000001));
      }
    };

    checkOdd([](float c)
             { return ProPhotoRGBTransferFunction<float, TransferFunctionMode::Unclamped>::ToLinear(c); });
    checkOdd(
      [](float c)
      { return ProPhotoRGBTransferFunction<float, TransferFunctionMode::Unclamped>::ToGammaEncoded(c); });
    checkOdd([](float c)
             { return Rec2020TransferFunction<float, TransferFunctionMode::Unclamped>::ToLinear(c); });
    checkOdd([](float c)
             { return Rec2020TransferFunction<float, TransferFunctionMode::Unclamped>::ToGammaEncoded(c); });
    checkOdd([](float c)
             { return A98RGBTransferFunction<float, TransferFunctionMode::Unclamped>::ToLinear(c); });
    checkOdd([](float c)
             { return SRGBTransferFunction<float, TransferFunctionMode::Unclamped>::ToLinear(c); });
  }

  TEST_CASE("ColorTransferFunctions: round trips", "[Colour][Utilities]")
  {
    for (float c : {0.0f, 0.01f, 0.25f, 0.5f, 0.75f, 1.0f})
    {
      INFO(c);
      using A98 = A98RGBTransferFunction<float, TransferFunctionMode::Clamped>;
      using ProPhoto = ProPhotoRGBTransferFunction<float, TransferFunctionMode::Clamped>;
      using Rec2020 = Rec2020TransferFunction<float, TransferFunctionMode::Clamped>;
      using SRGB = SRGBTransferFunction<float, TransferFunctionMode::Clamped>;

      CHECK(A98::ToGammaEncoded(A98::ToLinear(c)) == Approx(c).margin(0.00001));
      CHECK(ProPhoto::ToGammaEncoded(ProPhoto::ToLinear(c)) == Approx(c).margin(0.00001));
      CHECK(Rec2020::ToGammaEncoded(Rec2020::ToLinear(c)) == Approx(c).margin(0.00001));
      CHECK(SRGB::ToGammaEncoded(SRGB::ToLinear(c)) == Approx(c).margin(0.00001));
    }
  }

  TEST_CASE("ColourMatrix", "[Colour][Utilities]")
  {
    constexpr ColourMatrix<3, 3> identity {1, 0, 0, 0, 1, 0, 0, 0, 1};
    STATIC_CHECK(identity.at(1, 1) == 1.0f);
    STATIC_CHECK(identity.at(1, 2) == 0.0f);
    STATIC_CHECK(identity == ColourMatrix<3, 3> {1, 0, 0, 0, 1, 0, 0, 0, 1});

    constexpr ColourComponents<float, 4> input {0.25f, 0.5f, 0.75f, 1.0f};
    STATIC_CHECK(identity.TransformedColourComponents(input) == input);

    // A 3x3 matrix converts to a 5x4 one that leaves alpha alone.
    constexpr ColourMatrix<5, 4> widened = identity;
    STATIC_CHECK(widened.at(3, 3) == 1.0f);
    STATIC_CHECK(widened.at(0, 4) == 0.0f);
    STATIC_CHECK(widened.TransformedColourComponents(input) == input);

    auto gray = GrayscaleColourMatrix(1).TransformedColourComponents(
      ColourComponents<float, 4> {1.0f, 0.0f, 0.0f, 1.0f});
    CHECK(gray[0] == Approx(0.2126f));
    CHECK(gray[1] == Approx(0.2126f));
    CHECK(gray[2] == Approx(0.2126f));
    CHECK(gray[3] == 1.0f);

    // 5x4 matrices add the last column as an offset.
    auto inverted = InvertColourMatrix(1).TransformedColourComponents(input);
    CHECK(inverted == ColourComponents<float, 4> {0.75f, 0.5f, 0.25f, 1.0f});

    auto transparent = OpacityColourMatrix(0.5f).TransformedColourComponents(input);
    CHECK(transparent == ColourComponents<float, 4> {0.25f, 0.5f, 0.75f, 0.5f});

    auto contrast = ContrastColourMatrix(0).TransformedColourComponents(input);
    CHECK(contrast == ColourComponents<float, 4> {0.5f, 0.5f, 0.5f, 1.0f});

    CHECK(BrightnessColourMatrix(2).TransformedColourComponents(input)
          == ColourComponents<float, 4> {0.5f, 1.0f, 1.5f, 1.0f});
    CHECK(SaturationColourMatrix(1).TransformedColourComponents(input)[0] == Approx(0.25f));
    CHECK(SepiaColourMatrix(0).TransformedColourComponents(input)[2] == Approx(0.75f));

    auto rotated = HueRotateColourMatrix(0).TransformedColourComponents(input);
    CHECK(rotated[0] == Approx(0.25f));
    CHECK(rotated[1] == Approx(0.5f));
    CHECK(rotated[2] == Approx(0.75f));

    auto applied =
      ApplyMatricesToColourComponents(input, BrightnessColourMatrix(2), BrightnessColourMatrix(0.5f));
    CHECK(applied == input);
  }

  TEST_CASE("ColourComponents", "[Colour][Utilities]")
  {
    constexpr ColourComponents<float, 4> a {1.0f, -2.0f, 3.0f, 4.0f};
    constexpr ColourComponents<float, 4> b {0.5f, 0.5f, 0.5f, 0.5f};

    STATIC_CHECK((a + 1.0f) == ColourComponents<float, 4> {2.0f, -1.0f, 4.0f, 5.0f});
    STATIC_CHECK((a * 2.0f) == ColourComponents<float, 4> {2.0f, -4.0f, 6.0f, 8.0f});
    STATIC_CHECK((a / 2.0f) == ColourComponents<float, 4> {0.5f, -1.0f, 1.5f, 2.0f});
    CHECK(a.Abs() == ColourComponents<float, 4> {1.0f, 2.0f, 3.0f, 4.0f});
    STATIC_CHECK(a.Subset<1, 3>() == ColourComponents<float, 2> {-2.0f, 3.0f});
    STATIC_CHECK(PerComponentMax(a, b) == ColourComponents<float, 4> {1.0f, 0.5f, 3.0f, 4.0f});
    STATIC_CHECK(PerComponentMin(a, b) == ColourComponents<float, 4> {0.5f, -2.0f, 0.5f, 0.5f});
    STATIC_CHECK(a.get<2>() == 3.0f);

    auto sum = a;
    sum += b;
    CHECK(sum == ColourComponents<float, 4> {1.5f, -1.5f, 3.5f, 4.5f});
    sum /= 0.5f;
    CHECK(sum == ColourComponents<float, 4> {3.0f, -3.0f, 7.0f, 9.0f});

    // NaNs compare equal to each other.
    constexpr auto none = std::numeric_limits<float>::quiet_NaN();
    CHECK(ColourComponents<float, 4> {none, 1.0f, 1.0f, 1.0f}
          == ColourComponents<float, 4> {none, 1.0f, 1.0f, 1.0f});
    CHECK_FALSE(ColourComponents<float, 4> {none, 1.0f, 1.0f, 1.0f}
                == ColourComponents<float, 4> {0.0f, 1.0f, 1.0f, 1.0f});

    auto [c1, c2, c3, c4] = a;
    CHECK(c1 + c2 + c3 + c4 == 6.0f);
  }
}
