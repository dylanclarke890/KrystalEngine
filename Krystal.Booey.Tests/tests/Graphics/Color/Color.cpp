#include "Krystal.Booey/Graphics/Color/Color.hpp"
#include "catch2/catch_test_macros.hpp"
#include "catch2/matchers/catch_matchers_floating_point.hpp"
#include "Krystal.Booey/Graphics/Color/ColorHash.hpp"
#include "Krystal.Booey/Graphics/Color/ColorSerialisation.hpp"

// Ported from WebKit's Tools/TestWebKitAPI/Tests/WebCore/ColorTests.cpp, plus regression tests.

namespace krys::boo::tests
{
  using Catch::Matchers::WithinULP;

  namespace
  {
    void CheckRGBToHSL(const Color &color, float hue, float saturation, float lightness)
    {
      auto hslaColor = color.ToColorTypeLossy<HSLA<float>>().Resolved();

      CHECK_THAT(hslaColor.Hue, WithinULP(hue, 4));
      CHECK_THAT(hslaColor.Saturation, WithinULP(saturation, 4));
      CHECK_THAT(hslaColor.Lightness, WithinULP(lightness, 4));

      CHECK_THAT(static_cast<float>(color.Lightness() * 100), WithinULP(hslaColor.Lightness, 4));

      auto roundTrippedColor = ConvertColor<SRGBA<uint8>>(hslaColor);
      CHECK(color == Color {roundTrippedColor});
    }

    void CheckDisplayP3Components(const Color &color)
    {
      auto [colorSpace, components] = color.ColorSpaceAndResolvedColorComponents();
      auto [r, g, b, alpha] = components;

      CHECK(colorSpace == ColorSpace::DisplayP3);
      CHECK_THAT(r, WithinULP(1.0f, 4));
      CHECK_THAT(g, WithinULP(0.5f, 4));
      CHECK_THAT(b, WithinULP(0.25f, 4));
      CHECK_THAT(alpha, WithinULP(1.0f, 4));
      CHECK(SerialisationForCSS(color) == u8"color(display-p3 1 0.5 0.25)");
    }

    Color MakeColor()
    {
      return Color {DisplayP3<float> {1.0f, 0.5f, 0.25f, 1.0f}};
    }

    template <typename T>
    void SelfMove(T &a, T &&b)
    {
      a = krys::move(b);
    }
  }

  TEST_CASE("Color: RGB to HSL", "[Graphics][Color]")
  {
    CheckRGBToHSL(Color::white, 0, 0, 100);
    CheckRGBToHSL(Color::black, 0, 0, 0);
    CheckRGBToHSL(Color::red, 0, 100, 50);
    CheckRGBToHSL(Color::green, 120, 100, 50);
    CheckRGBToHSL(Color::blue, 240, 100, 50);
    CheckRGBToHSL(Color::darkGray, 0, 0, 50.196083f);
    CheckRGBToHSL(Color::gray, 0, 0, 62.745102f);
    CheckRGBToHSL(Color::lightGray, 0, 0, 75.294121f);
  }

  TEST_CASE("Color: validity", "[Graphics][Color]")
  {
    Color invalidColor;
    CHECK_FALSE(invalidColor.IsValid());

    Color otherInvalidColor = invalidColor;
    CHECK_FALSE(otherInvalidColor.IsValid());

    Color validColor = Color::red;
    CHECK(validColor.IsValid());

    Color otherValidColor = validColor;
    CHECK(otherValidColor.IsValid());

    validColor = SRGBA<uint8> {1, 2, 3, 4};
    CHECK(validColor.IsValid());
    auto validColorComponents = validColor.ToColorTypeLossy<SRGBA<uint8>>().Resolved();
    CHECK(validColorComponents.Red == 1);
    CHECK(validColorComponents.Green == 2);
    CHECK(validColorComponents.Blue == 3);
    CHECK(validColorComponents.Alpha == 4);

    Color yetAnotherValidColor(krys::move(validColor));
    CHECK(yetAnotherValidColor.IsValid());
    auto yetAnotherValidColorComponents = yetAnotherValidColor.ToColorTypeLossy<SRGBA<uint8>>().Resolved();
    CHECK(yetAnotherValidColorComponents.Red == 1);
    CHECK(yetAnotherValidColorComponents.Green == 2);
    CHECK(yetAnotherValidColorComponents.Blue == 3);
    CHECK(yetAnotherValidColorComponents.Alpha == 4);

    otherValidColor = krys::move(yetAnotherValidColor);
    CHECK(otherValidColor.IsValid());
    auto otherValidColorComponents = otherValidColor.ToColorTypeLossy<SRGBA<uint8>>().Resolved();
    CHECK(otherValidColorComponents.Red == 1);
    CHECK(otherValidColorComponents.Green == 2);
    CHECK(otherValidColorComponents.Blue == 3);
    CHECK(otherValidColorComponents.Alpha == 4);
  }

  TEST_CASE("Color: luminance, lightened and darkened", "[Graphics][Color]")
  {
    CHECK_THAT(static_cast<float>(Color(Color::black).Luminance()), WithinULP(0.0f, 4));
    CHECK_THAT(static_cast<float>(Color(Color::white).Luminance()), WithinULP(1.0f, 4));

    auto cComponents = SRGBA<uint8> {85, 90, 160}.Resolved();
    CHECK_THAT(static_cast<float>(Color(cComponents).Luminance()), WithinULP(0.11781455f, 4));

    CHECK(cComponents.Red == 85);
    CHECK(cComponents.Green == 90);
    CHECK(cComponents.Blue == 160);

    auto cLightened = Color(cComponents).Lightened().ToColorTypeLossy<SRGBA<uint8>>().Resolved();
    CHECK_THAT(static_cast<float>(Color(cLightened).Luminance()), WithinULP(0.291682f, 4));
    CHECK(cLightened.Red == 130);
    CHECK(cLightened.Green == 137);
    CHECK(cLightened.Blue == 244);

    auto cDarkened = Color(cComponents).Darkened().ToColorTypeLossy<SRGBA<uint8>>().Resolved();
    CHECK_THAT(static_cast<float>(Color(cDarkened).Luminance()), WithinULP(0.027006242f, 4));
    CHECK(cDarkened.Red == 40);
    CHECK(cDarkened.Green == 43);
    CHECK(cDarkened.Blue == 76);
  }

  TEST_CASE("Color: construction, copy and assignment", "[Graphics][Color]")
  {
    Color c1 {DisplayP3<float> {1.0f, 0.5f, 0.25f, 1.0f}};
    CheckDisplayP3Components(c1);

    Color c2(c1);
    CheckDisplayP3Components(c2);

    Color c3 = c1;
    CheckDisplayP3Components(c3);

    CheckDisplayP3Components(MakeColor());
  }

  TEST_CASE("Color: equality", "[Graphics][Color]")
  {
    {
      Color c1 {DisplayP3<float> {1.0f, 0.5f, 0.25f, 1.0f}};
      Color c2 {DisplayP3<float> {1.0f, 0.5f, 0.25f, 1.0f}};
      CHECK(c1 == c2);
    }

    {
      Color c1 {DisplayP3<float> {1.0f, 0.5f, 0.25f, 1.0f}};
      Color c2 {SRGBA<float> {1.0f, 0.5f, 0.25f, 1.0f}};
      CHECK(c1 != c2);
    }

    auto componentBytes = SRGBA<uint8> {255, 128, 63, 127};
    Color rgb1 {ConvertColor<SRGBA<float>>(componentBytes)};
    Color rgb2 {componentBytes};
    CHECK(rgb1 != rgb2);
    CHECK(rgb2 != rgb1);
  }

  TEST_CASE("Color: hash", "[Graphics][Color]")
  {
    auto hash = [](const Color &color)
    {
      return DefaultHash<Color>::Hash(color);
    };

    {
      Color c1 {DisplayP3<float> {1.0f, 0.5f, 0.25f, 1.0f}};
      Color c2 {DisplayP3<float> {1.0f, 0.5f, 0.25f, 1.0f}};
      CHECK(hash(c1) == hash(c2));
    }

    {
      Color c1 {DisplayP3<float> {1.0f, 0.5f, 0.25f, 1.0f}};
      Color c2 {SRGBA<float> {1.0f, 0.5f, 0.25f, 1.0f}};
      CHECK(hash(c1) != hash(c2));
    }

    auto componentBytes = SRGBA<uint8> {255, 128, 63, 127};
    Color rgb1 {ConvertColor<SRGBA<float>>(componentBytes)};
    Color rgb2 {componentBytes};
    CHECK(hash(rgb1) != hash(rgb2));
  }

  TEST_CASE("Color: hash traits", "[Graphics][Color]")
  {
    auto empty = HashTraits<Color>::EmptyValue();
    CHECK(HashTraits<Color>::IsEmptyValue(empty));
    CHECK_FALSE(empty.IsValid());
    CHECK_FALSE(HashTraits<Color>::IsDeletedValue(empty));

    Color deleted;
    HashTraits<Color>::ConstructDeletedValue(deleted);
    CHECK(HashTraits<Color>::IsDeletedValue(deleted));
    CHECK_FALSE(HashTraits<Color>::IsEmptyValue(deleted));

    CHECK_FALSE(HashTraits<Color>::IsEmptyValue(Color {Color::red}));
    CHECK_FALSE(HashTraits<Color>::IsDeletedValue(Color {Color::red}));
  }

  TEST_CASE("Color: move construction", "[Graphics][Color]")
  {
    Color c1 {DisplayP3<float> {1.0f, 0.5f, 0.25f, 1.0f}};
    Color c2(krys::move(c1));

    // We should have moved the out of line color pointer into c2,
    // and set c1 to invalid so that it doesn't cause deletion.
    CHECK_FALSE(c1.IsValid()); // NOLINT(bugprone-use-after-move)
    CheckDisplayP3Components(c2);
  }

  TEST_CASE("Color: move assignment", "[Graphics][Color]")
  {
    Color c1 {DisplayP3<float> {1.0f, 0.5f, 0.25f, 1.0f}};
    Color c2;
    c2 = krys::move(c1);

    CHECK(c2.IsValid());

    // We should have moved the out of line color pointer into c2,
    // and set c1 to invalid so that it doesn't cause deletion.
    CHECK_FALSE(c1.IsValid()); // NOLINT(bugprone-use-after-move)
    CheckDisplayP3Components(c2);

    // The API contract is that move invalidates. We should mark the source as invalid even if the
    // colors are the same.
    Color c3 {DisplayP3<float> {1.0f, 0.5f, 0.25f, 1.0f}};
    CHECK(c2 == c3);
    c2 = krys::move(c3);
    CHECK_FALSE(c3.IsValid()); // NOLINT(bugprone-use-after-move)
    CHECK(c2.IsValid());
    CheckDisplayP3Components(c2);

    // Moving into itself works.
    SelfMove(c2, krys::move(c2));
    CHECK(c2.IsValid());
    CheckDisplayP3Components(c2);
    SelfMove(c1, krys::move(c1));
    CHECK_FALSE(c1.IsValid());
  }

  TEST_CASE("Color: P3 conversion to sRGB", "[Graphics][Color]")
  {
    Color p3Color {DisplayP3<float> {1.0f, 0.5f, 0.25f, 0.75f}};
    auto sRGBAColor = p3Color.ToColorTypeLossy<SRGBA<float>>().Resolved();
    CHECK_THAT(sRGBAColor.Red, WithinULP(1.0f, 4));
    CHECK_THAT(sRGBAColor.Green, WithinULP(0.46253288f, 4));
    CHECK_THAT(sRGBAColor.Blue, WithinULP(0.14912745f, 4));
    CHECK_THAT(sRGBAColor.Alpha, WithinULP(0.75f, 4));
  }

  TEST_CASE("Color: P3 conversion to extended sRGB", "[Graphics][Color]")
  {
    Color p3Color {DisplayP3<float> {1.0f, 0.5f, 0.25f, 0.75f}};
    auto sRGBAColor = p3Color.ToColorTypeLossy<ExtendedSRGBA<float>>().Resolved();
    CHECK_THAT(sRGBAColor.Red, WithinULP(1.0740442f, 4));
    CHECK_THAT(sRGBAColor.Green, WithinULP(0.46253282f, 4));
    CHECK_THAT(sRGBAColor.Blue, WithinULP(0.14912748f, 4));
    CHECK_THAT(sRGBAColor.Alpha, WithinULP(0.75f, 4));
  }

  TEST_CASE("Color: linear sRGB conversion to sRGB", "[Graphics][Color]")
  {
    Color linearSRGBAColor {LinearSRGBA<float> {1.0f, 0.5f, 0.25f, 0.75f}};
    auto sRGBAColor = linearSRGBAColor.ToColorTypeLossy<SRGBA<float>>().Resolved();
    CHECK_THAT(sRGBAColor.Red, WithinULP(1.0f, 4));
    CHECK_THAT(sRGBAColor.Green, WithinULP(0.735356927f, 4));
    CHECK_THAT(sRGBAColor.Blue, WithinULP(0.537098706f, 4));
    CHECK_THAT(sRGBAColor.Alpha, WithinULP(0.75f, 4));
  }

  TEST_CASE("Color: ColorWithAlphaMultipliedBy", "[Graphics][Color]")
  {
    Color color {SRGBA<float> {0.0f, 0.0f, 1.0f, 0.6f}};

    {
      Color colorWithAlphaMultipliedBy = color.ColorWithAlphaMultipliedBy(1.0f);
      CHECK(color == colorWithAlphaMultipliedBy);
    }

    for (auto [multiplier, expectedAlpha] : {std::pair {0.5f, 0.3f}, std::pair {0.0f, 0.0f}})
    {
      Color colorWithAlphaMultipliedBy = color.ColorWithAlphaMultipliedBy(multiplier);
      auto [colorSpace, components] = colorWithAlphaMultipliedBy.ColorSpaceAndResolvedColorComponents();
      CHECK(colorSpace == ColorSpace::SRGB);
      auto [r, g, b, a] = components;
      CHECK_THAT(r, WithinULP(0.0f, 4));
      CHECK_THAT(g, WithinULP(0.0f, 4));
      CHECK_THAT(b, WithinULP(1.0f, 4));
      CHECK_THAT(a, WithinULP(expectedAlpha, 4));
    }
  }

  TEST_CASE("Color: alpha of inline colors", "[Graphics][Color]")
  {
    // Regression: ConvertByteAlphaTo<float> returned the byte unscaled and ConvertFloatAlphaTo<uint8>
    // divided by 255, so inline alphas were reported as 0-255 and ColorWithAlpha zeroed them.
    CHECK(Color(Color::red).AlphaAsFloat() == 1.0f);
    CHECK(Color(Color::red).AlphaByte() == 255);
    CHECK(Color(SRGBA<uint8> {0, 0, 0, 51}).AlphaAsFloat() == 0.2f);

    auto halfRed = Color(Color::red).ColorWithAlpha(0.5f);
    CHECK(halfRed.TryGetAsSRGBABytes().has_value());
    CHECK(halfRed.AlphaByte() == 128);
    CHECK(SerialisationForCSS(halfRed) == u8"rgba(255, 0, 0, 0.5)");

    auto outOfLine = Color(SRGBA<float> {1, 0, 0, 1}).ColorWithAlpha(0.25f);
    CHECK(outOfLine.AlphaAsFloat() == 0.25f);
    CHECK(outOfLine.AlphaByte() == 64);

    CHECK(Color(SRGBA<float> {1, 0, 0, 1}).ColorWithAlpha(2.0f).AlphaAsFloat() == 1.0f);
    CHECK(Color(Color::red).OpaqueColor().IsOpaque());
    CHECK_FALSE(Color(Color::red).ColorWithAlpha(0.0f).IsVisible());
  }

  TEST_CASE("Color: semantic flag", "[Graphics][Color]")
  {
    Color inlineColor {Color::red};
    Color inlineSemantic = inlineColor.SemanticColor();
    CHECK(inlineSemantic.IsSemantic());
    CHECK(inlineSemantic != inlineColor);

    // Regression: the Semantic flag was subtracted arithmetically rather than masked out, so a semantic and a
    // non-semantic color never compared equal.
    CHECK(EqualIgnoringSemanticColor(inlineSemantic, inlineColor));
    CHECK(EqualIgnoringSemanticColor(inlineColor, inlineColor));
    CHECK_FALSE(
      EqualIgnoringSemanticColor(inlineColor, Color {Color::red, ColorFlags::UseColorFunctionSerialisation}));

    Color outOfLine {SRGBA<float> {1, 0, 0, 1}};
    Color outOfLineSemantic = outOfLine.SemanticColor();
    CHECK(outOfLineSemantic.IsSemantic());
    CHECK(outOfLineSemantic != outOfLine);
    CHECK(EqualIgnoringSemanticColor(outOfLineSemantic, outOfLine));
    CHECK(EqualIgnoringSemanticColor(outOfLine, outOfLine));
    CHECK(OutOfLineComponentsEqualIgnoringSemanticColor(outOfLineSemantic, outOfLine));
    CHECK_FALSE(EqualIgnoringSemanticColor(outOfLine, inlineColor));

    CHECK(outOfLineSemantic.ColorWithAlpha(0.5f).IsSemantic());
  }

  TEST_CASE("Color: inline and out of line storage", "[Graphics][Color]")
  {
    Color inlineColor {SRGBA<uint8> {1, 2, 3, 4}};
    CHECK(inlineColor.Space() == ColorSpace::SRGB);
    REQUIRE(inlineColor.TryGetAsPackedInline().has_value());
    CHECK(inlineColor.TryGetAsPackedInline()->Value == 0x01'02'03'04u);

    Color outOfLine {Lab<float> {50, 10, 20, 1}};
    CHECK(outOfLine.Space() == ColorSpace::Lab);
    CHECK_FALSE(outOfLine.TryGetAsPackedInline().has_value());
    CHECK_FALSE(outOfLine.TryGetAsSRGBABytes().has_value());

    CHECK_FALSE(Color(Maybe<SRGBA<uint8>> {}).IsValid());
    CHECK(Color(Maybe<SRGBA<uint8>> {Color::red}).IsValid());
    CHECK_FALSE(Color(Maybe<Lab<float>> {}).IsValid());
    CHECK(Color(Maybe<Lab<float>> {Lab<float> {}}).IsValid());
  }

  TEST_CASE("Color: AnyComponentIsNone", "[Graphics][Color]")
  {
    constexpr auto none = std::numeric_limits<float>::quiet_NaN();

    CHECK_FALSE(Color(Color::red).AnyComponentIsNone());
    CHECK_FALSE(Color(SRGBA<float> {1, 0, 0, 1}).AnyComponentIsNone());
    CHECK(Color(SRGBA<float> {1, none, 0, 1}).AnyComponentIsNone());
    CHECK(Color(OKLCHA<float> {0.5f, 0.1f, none, 1}).AnyComponentIsNone());
  }

  TEST_CASE("Color: black and white", "[Graphics][Color]")
  {
    CHECK(Color::IsBlackColor(Color::black));
    CHECK_FALSE(Color::IsBlackColor(Color::white));
    CHECK(Color::IsBlackColor(Color {Lab<float> {0, 0, 0, 1}}));
    CHECK(Color::IsBlackColor(Color {HWBA<float> {0, 0, 100, 1}}));
    CHECK_FALSE(Color::IsBlackColor(Color {SRGBA<uint8> {0, 0, 0, 128}}));

    CHECK(Color::IsWhiteColor(Color::white));
    CHECK_FALSE(Color::IsWhiteColor(Color::black));
    CHECK(Color::IsWhiteColor(Color {OKLab<float> {1, 0, 0, 1}}));
    CHECK(Color::IsWhiteColor(Color {SRGBA<float> {1, 1, 1, 1}}));
  }

  TEST_CASE("Color: InvertedColorWithAlpha", "[Graphics][Color]")
  {
    CHECK(Color(Color::red).InvertedColorWithAlpha(1.0f) == Color {Color::cyan});

    auto inverted = Color(SRGBA<float> {1, 0.25f, 0, 1}).InvertedColorWithAlpha(0.5f);
    auto [r, g, b, a] = inverted.ToColorTypeLossy<SRGBA<float>>().Resolved();
    CHECK_THAT(r, WithinULP(0.0f, 4));
    CHECK_THAT(g, WithinULP(0.75f, 4));
    CHECK_THAT(b, WithinULP(1.0f, 4));
    CHECK_THAT(a, WithinULP(0.5f, 4));

    // Non-invertible types are inverted in sRGB.
    auto invertedLab = Color(Lab<float> {100, 0, 0, 1}).InvertedColorWithAlpha(Maybe<float> {1.0f});
    CHECK(invertedLab.Space() == ColorSpace::SRGB);
    CHECK(Color(Color::red).InvertedColorWithAlpha(Maybe<float> {}) == Color {Color::red});
  }

  TEST_CASE("Color: ToResolvedColorComponentsInColorSpace", "[Graphics][Color]")
  {
    auto [r, g, b, a] = Color(Color::white).ToResolvedColorComponentsInColorSpace(ColorSpace::LinearSRGB);
    CHECK_THAT(r, WithinULP(1.0f, 4));
    CHECK_THAT(g, WithinULP(1.0f, 4));
    CHECK_THAT(b, WithinULP(1.0f, 4));
    CHECK_THAT(a, WithinULP(1.0f, 4));

    constexpr auto none = std::numeric_limits<float>::quiet_NaN();
    auto [colorSpace, components] =
      Color(SRGBA<float> {none, 0.5f, 0, none}).ColorSpaceAndResolvedColorComponents();
    CHECK(colorSpace == ColorSpace::SRGB);
    CHECK(components == ColorComponents<float, 4> {0.0f, 0.5f, 0.0f, 0.0f});
  }

  TEST_CASE("Color: debug output", "[Graphics][Color]")
  {
    auto debug = Color(SRGBA<float> {1, 0, 0, 0.5f}).DebugRGBA();
    CHECK(debug.Red == 255);
    CHECK(debug.Green == 0);
    CHECK(debug.Blue == 0);
    CHECK(debug.Alpha == 128);

    CHECK(Color(SRGBA<uint8> {255, 0, 16, 128}).DebugDescription() == u8"#FF001080");
  }
}
