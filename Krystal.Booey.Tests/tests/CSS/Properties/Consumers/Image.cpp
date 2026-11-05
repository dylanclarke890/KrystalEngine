#include "Krystal.Booey/CSS/Properties/Consumers/Image.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ConsumerTestUtils.hpp"

namespace krys::boo::css::PropertyParserHelpers::tests
{
  using css::tests::ConsumeToText;

  namespace
  {
    constexpr auto Image = [](TokenRange &tokens, PropertyParserState &state)
    {
      return ConsumeImage(tokens, state);
    };

    constexpr auto ImageOrNone = [](TokenRange &tokens, PropertyParserState &state)
    {
      return ConsumeImageOrNone(tokens, state);
    };

    template <AllowedImageType Allowed>
    constexpr auto ImageAllowing = [](TokenRange &tokens, PropertyParserState &state)
    {
      return ConsumeImage(tokens, state, Allowed);
    };

    ParserContext Context()
    {
      return ParserContext {ParserMode::HTMLStandard, url::URL(u8"https://example.com/dir/")};
    }
  }

  TEST_CASE("PropertyParserHelpers(ConsumeImage linear-gradient)", "[CSS][Properties][Consumers]")
  {
    CHECK(ConsumeToText(u8"linear-gradient(red, blue)", Image) == "linear-gradient(red, blue)");
    CHECK(ConsumeToText(u8"linear-gradient(red)", Image) == "linear-gradient(red)");
    CHECK(ConsumeToText(u8"linear-gradient(90deg, red, blue)", Image) == "linear-gradient(90deg, red, blue)");
    CHECK(ConsumeToText(u8"repeating-linear-gradient(45deg, red, blue 10px)", Image)
          == "repeating-linear-gradient(45deg, red, blue 10px)");

    // The default direction is dropped; unitless zero is an angle here.
    CHECK(ConsumeToText(u8"linear-gradient(to bottom, red, blue)", Image) == "linear-gradient(red, blue)");
    CHECK(ConsumeToText(u8"linear-gradient(180deg, red, blue)", Image) == "linear-gradient(red, blue)");
    CHECK(ConsumeToText(u8"linear-gradient(0, red, blue)", Image) == "linear-gradient(0deg, red, blue)");

    // Sides and corners, in either order.
    CHECK(ConsumeToText(u8"linear-gradient(to left, red, blue)", Image)
          == "linear-gradient(to left, red, blue)");
    CHECK(ConsumeToText(u8"linear-gradient(to top right, red, blue)", Image)
          == "linear-gradient(to right top, red, blue)");
    CHECK(ConsumeToText(u8"linear-gradient(to right top, red, blue)", Image)
          == "linear-gradient(to right top, red, blue)");

    // Colour stops: hints, and two positions sharing a colour.
    CHECK(ConsumeToText(u8"linear-gradient(red 10%, 50%, blue)", Image)
          == "linear-gradient(red 10%, 50%, blue)");
    CHECK(ConsumeToText(u8"linear-gradient(red 10% 20%, blue)", Image)
          == "linear-gradient(red 10%, red 20%, blue)");

    CHECK(ConsumeToText(u8"linear-gradient()", Image) == "<null>");
    CHECK(ConsumeToText(u8"linear-gradient(10%, red)", Image) == "<null>");
    CHECK(ConsumeToText(u8"linear-gradient(red, 10%)", Image) == "<null>");
    CHECK(ConsumeToText(u8"linear-gradient(red, 10%, 20%, blue)", Image) == "<null>");
    CHECK(ConsumeToText(u8"linear-gradient(90deg red, blue)", Image) == "<null>");
    CHECK(ConsumeToText(u8"linear-gradient(to, red, blue)", Image) == "<null>");
    CHECK(ConsumeToText(u8"linear-gradient(to left left, red, blue)", Image) == "<null>");
    CHECK(ConsumeToText(u8"linear-gradient(red, blue,)", Image) == "<null>");
  }

  TEST_CASE("PropertyParserHelpers(ConsumeImage gradient colour interpolation)",
            "[CSS][Properties][Consumers]")
  {
    // Legacy (8-bit sRGB) stops default to sRGB, others to OKLab; the default isn't serialised.
    CHECK(ConsumeToText(u8"linear-gradient(in srgb, red, blue)", Image) == "linear-gradient(red, blue)");
    CHECK(ConsumeToText(u8"linear-gradient(in oklab, red, blue)", Image)
          == "linear-gradient(in oklab, red, blue)");
    CHECK(ConsumeToText(u8"linear-gradient(lab(50 0 0), blue)", Image)
          == "linear-gradient(lab(50 0 0), blue)");
    CHECK(ConsumeToText(u8"linear-gradient(in srgb, lab(50 0 0), blue)", Image)
          == "linear-gradient(in srgb, lab(50 0 0), blue)");

    // The method may come before or after the direction.
    CHECK(ConsumeToText(u8"linear-gradient(90deg in hsl longer hue, red, blue)", Image)
          == "linear-gradient(90deg in hsl longer hue, red, blue)");
    CHECK(ConsumeToText(u8"linear-gradient(in hsl longer hue 90deg, red, blue)", Image)
          == "linear-gradient(90deg in hsl longer hue, red, blue)");

    CHECK(ConsumeToText(u8"linear-gradient(in, red, blue)", Image) == "<null>");
    CHECK(ConsumeToText(u8"linear-gradient(in srgb 90deg in srgb, red, blue)", Image) == "<null>");
  }

  TEST_CASE("PropertyParserHelpers(ConsumeImage radial-gradient)", "[CSS][Properties][Consumers]")
  {
    CHECK(ConsumeToText(u8"radial-gradient(red, blue)", Image) == "radial-gradient(red, blue)");
    CHECK(ConsumeToText(u8"radial-gradient(ellipse farthest-corner, red, blue)", Image)
          == "radial-gradient(red, blue)");
    CHECK(ConsumeToText(u8"radial-gradient(circle, red, blue)", Image)
          == "radial-gradient(circle, red, blue)");
    CHECK(ConsumeToText(u8"radial-gradient(closest-side, red, blue)", Image)
          == "radial-gradient(closest-side, red, blue)");
    CHECK(ConsumeToText(u8"radial-gradient(closest-side circle at left top, red, blue)", Image)
          == "radial-gradient(circle closest-side at left top, red, blue)");
    CHECK(ConsumeToText(u8"radial-gradient(at left top, red, blue)", Image)
          == "radial-gradient(at left top, red, blue)");
    CHECK(ConsumeToText(u8"repeating-radial-gradient(circle in srgb, red, blue)", Image)
          == "repeating-radial-gradient(circle, red, blue)");

    // A single length is a circle; two length-percentages are an ellipse.
    CHECK(ConsumeToText(u8"radial-gradient(10px, red, blue)", Image) == "radial-gradient(10px, red, blue)");
    CHECK(ConsumeToText(u8"radial-gradient(circle 10px, red, blue)", Image)
          == "radial-gradient(10px, red, blue)");
    CHECK(ConsumeToText(u8"radial-gradient(10px 20%, red, blue)", Image)
          == "radial-gradient(10px 20%, red, blue)");
    CHECK(ConsumeToText(u8"radial-gradient(ellipse 10px 20%, red, blue)", Image)
          == "radial-gradient(10px 20%, red, blue)");

    CHECK(ConsumeToText(u8"radial-gradient(circle 10px 20px, red, blue)", Image) == "<null>");
    CHECK(ConsumeToText(u8"radial-gradient(ellipse 10px, red, blue)", Image) == "<null>");
    CHECK(ConsumeToText(u8"radial-gradient(10%, red, blue)", Image) == "<null>");
    CHECK(ConsumeToText(u8"radial-gradient(-10px, red, blue)", Image) == "<null>");
    CHECK(ConsumeToText(u8"radial-gradient(circle circle, red, blue)", Image) == "<null>");
    CHECK(ConsumeToText(u8"radial-gradient(closest-side 10px, red, blue)", Image) == "<null>");
    CHECK(ConsumeToText(u8"radial-gradient(contain, red, blue)", Image) == "<null>");
    CHECK(ConsumeToText(u8"radial-gradient(at, red, blue)", Image) == "<null>");
  }

  TEST_CASE("PropertyParserHelpers(ConsumeImage conic-gradient)", "[CSS][Properties][Consumers]")
  {
    CHECK(ConsumeToText(u8"conic-gradient(red, blue)", Image) == "conic-gradient(red, blue)");
    CHECK(ConsumeToText(u8"conic-gradient(from 90deg, red, blue)", Image)
          == "conic-gradient(from 90deg, red, blue)");
    CHECK(ConsumeToText(u8"conic-gradient(from 0, red, blue)", Image) == "conic-gradient(red, blue)");
    CHECK(ConsumeToText(u8"conic-gradient(from 90deg at left top in srgb, red, blue)", Image)
          == "conic-gradient(from 90deg at left top, red, blue)");
    CHECK(ConsumeToText(u8"repeating-conic-gradient(red 0, blue 50%)", Image)
          == "repeating-conic-gradient(red 0deg, blue 50%)");

    CHECK(ConsumeToText(u8"conic-gradient(red 10px, blue)", Image) == "<null>");
    CHECK(ConsumeToText(u8"conic-gradient(from, red, blue)", Image) == "<null>");
    CHECK(ConsumeToText(u8"conic-gradient(at left top from 90deg, red, blue)", Image) == "<null>");
  }

  TEST_CASE("PropertyParserHelpers(ConsumeImage url and image-set)", "[CSS][Properties][Consumers]")
  {
    auto context = Context();

    CHECK(ConsumeToText(u8"url(a.png)", Image, context) == "url(\"a.png\")");
    CHECK(ConsumeToText(u8"\"a.png\"", Image, context) == "<null>");
    CHECK(ConsumeToText(u8"none", Image, context) == "<null>");
    CHECK(ConsumeToText(u8"none", ImageOrNone, context) == "none");
    CHECK(ConsumeToText(u8"\"a.png\"", ImageAllowing<AllowedImageType::RawStringAsURL>, context)
          == "url(\"a.png\")");

    CHECK(ConsumeToText(u8"image-set(\"a.png\" 1x, url(b.png) 2x)", Image, context)
          == "image-set(url(\"a.png\") 1x, url(\"b.png\") 2x)");
    CHECK(ConsumeToText(u8"-webkit-image-set(url(a.png) 2dppx)", Image, context) == "<null>");
    CHECK(ConsumeToText(u8"image-set(url(a.png) type(\"image/png\") 2x)", Image, context)
          == "image-set(url(\"a.png\") 2x type(\"image/png\"))");
    CHECK(ConsumeToText(u8"image-set(linear-gradient(red, blue) 1x)", Image, context)
          == "image-set(linear-gradient(red, blue) 1x)");

    CHECK(ConsumeToText(u8"image-set(url(a.png) 1x 2x)", Image, context) == "<null>");
    CHECK(ConsumeToText(u8"image-set(url(a.png) type(\"a\") type(\"b\"))", Image, context) == "<null>");
    CHECK(ConsumeToText(u8"image-set(url(a.png) type(a))", Image, context) == "<null>");
    CHECK(ConsumeToText(u8"image-set(image-set(url(a.png)))", Image, context) == "<null>");
    CHECK(ConsumeToText(u8"image-set()", Image, context) == "<null>");
  }

  TEST_CASE("PropertyParserHelpers(ConsumeImage allowed types)", "[CSS][Properties][Consumers]")
  {
    auto context = Context();
    constexpr auto urlOnly = ImageAllowing<AllowedImageType::URLFunction>;

    CHECK(ConsumeToText(u8"url(a.png)", urlOnly, context) == "url(\"a.png\")");
    CHECK(ConsumeToText(u8"linear-gradient(red, blue)", urlOnly, context) == "<null>");
    CHECK(ConsumeToText(u8"image-set(url(a.png))", urlOnly, context) == "<null>");
    CHECK(ConsumeToText(u8"url(a.png)", ImageAllowing<AllowedImageType::GeneratedImage>, context)
          == "<null>");
  }

  TEST_CASE("PropertyParserHelpers(ConsumeImage generated images)", "[CSS][Properties][Consumers]")
  {
    auto context = Context();

    CHECK(ConsumeToText(u8"cross-fade(url(a.png), url(b.png), 50%)", Image, context)
          == "cross-fade(url(\"a.png\"), url(\"b.png\"), 0.5)");
    CHECK(ConsumeToText(u8"cross-fade(none, url(b.png), 2)", Image, context)
          == "cross-fade(none, url(\"b.png\"), 1)");
    CHECK(ConsumeToText(u8"cross-fade(url(a.png), url(b.png))", Image, context) == "<null>");

    // The prefixed generators are gone.
    CHECK(ConsumeToText(u8"-webkit-canvas(foo)", Image, context) == "<null>");
    CHECK(ConsumeToText(u8"-webkit-named-image(apple-pay-logo-black)", Image, context) == "<null>");

    CHECK(ConsumeToText(u8"filter(url(a.png), blur(2px))", Image, context)
          == "filter(url(\"a.png\"), blur(2px))");
    CHECK(ConsumeToText(u8"filter(none, sepia())", Image, context) == "filter(none, sepia())");
    CHECK(ConsumeToText(u8"filter(url(a.png))", Image, context) == "<null>");

    CHECK(ConsumeToText(u8"paint(foo, 1px)", Image, context) == "paint(foo)");
    CHECK(ConsumeToText(u8"paint(foo)", Image, context) == "paint(foo)");
    CHECK(ConsumeToText(u8"paint(foo 1px)", Image, context) == "<null>");
  }
}
