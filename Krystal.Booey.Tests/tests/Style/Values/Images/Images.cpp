#include "catch2/catch_approx.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Style/BuilderTestUtils.hpp"
#include "Krystal.Booey/Graphics/Animation/AnimationUtilities.hpp"
#include "Krystal.Booey/Graphics/Gradient.hpp"
#include "Krystal.Booey/Graphics/ImageOrientation.hpp"
#include "Krystal.Booey/Rendering/Style/StyleCachedImage.hpp"
#include "Krystal.Booey/Rendering/Style/StyleCrossfadeImage.hpp"
#include "Krystal.Booey/Rendering/Style/StyleFilterImage.hpp"
#include "Krystal.Booey/Rendering/Style/StyleGradientImage.hpp"
#include "Krystal.Booey/Rendering/Style/StyleImageSet.hpp"
#include "Krystal.Booey/Rendering/Style/StyleInvalidImage.hpp"
#include "Krystal.Booey/Style/Values/CSSValueConversion.hpp"
#include "Krystal.Booey/Style/Values/Images/Gradient.hpp"
#include "Krystal.Booey/Style/Values/Images/ImageOrientation.hpp"
#include "Krystal.Booey/Style/Values/Images/ImageOrNone.hpp"
#include "Krystal.Booey/Style/Values/Images/ObjectPosition.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordCSSValueConversion.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordCSSValueCreation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordSerialisation.hpp"

namespace krys::boo::style::tests
{
  using Catch::Approx;
  using css::PropertyId;

  namespace
  {
    KRYS_NODISCARD RefPtr<StyleImage> ImageOf(const ImageOrNone &value) noexcept
    {
      return value.TryStyleImage();
    }

    KRYS_NODISCARD GradientLinearColorStop Stop(boo::Color color,
                                                Maybe<float> percentage = std::nullopt) noexcept
    {
      GradientLinearColorStop stop {Markable<style::Color> {style::Color {color}}, std::nullopt};
      if (percentage)
      {
        stop.Position = LengthPercentage<> {LengthPercentage<>::Percentage {*percentage}};
      }
      return stop;
    }

    KRYS_NODISCARD GradientColorInterpolationMethod SRGBInterpolation() noexcept
    {
      return GradientColorInterpolationMethod::LegacyMethod(AlphaPremultiplication::Premultiplied);
    }
  }

  TEST_CASE_METHOD(BuilderFixture, "Style ImageOrNone", "[Style][Values][Images]")
  {
    CHECK(Convert<ImageOrNone>(PropertyId::ListStyleImage, u8"none").IsNone());
    CHECK(Serialised(Convert<ImageOrNone>(PropertyId::ListStyleImage, u8"none")) == "none");

    auto url = Convert<ImageOrNone>(PropertyId::ListStyleImage, u8"url(https://example.com/a.png)");
    REQUIRE(url.IsImage());
    auto cached = ImageOf(url);
    CHECK(cached->IsCachedImage());
    CHECK(cached->HasCachedImage());
    // Nothing loads the image yet.
    CHECK(cached->IsPending());
    CHECK_FALSE(cached->UsesDataProtocol());
    CHECK(cached->ImageScaleFactor() == 1);
    CHECK(Serialised(url) == "url(\"https://example.com/a.png\")");
    CHECK(Created(url) == "url(\"https://example.com/a.png\")");

    CHECK(ImageOf(Convert<ImageOrNone>(PropertyId::ListStyleImage, u8"url(data:image/png;base64,AAAA)"))
            ->UsesDataProtocol());

    // Equal URLs make equal images.
    CHECK((url == Convert<ImageOrNone>(PropertyId::ListStyleImage, u8"url(https://example.com/a.png)")));
    CHECK_FALSE(
      (url == Convert<ImageOrNone>(PropertyId::ListStyleImage, u8"url(https://example.com/b.png)")));

    SECTION("generated images")
    {
      auto gradient = Convert<ImageOrNone>(PropertyId::ListStyleImage, u8"linear-gradient(red, blue)");
      REQUIRE(gradient.IsImage());
      CHECK(ImageOf(gradient)->IsGradientImage());
      CHECK(ImageOf(gradient)->IsGeneratedImage());
      CHECK_FALSE(ImageOf(gradient)->IsPending());
      CHECK(ImageOf(gradient)->ImageHasRelativeWidth());
      CHECK(ImageOf(gradient)->Data() == ImageOf(gradient).get());
      CHECK(Serialised(gradient) == "linear-gradient(rgb(255, 0, 0), rgb(0, 0, 255))");

      auto crossfade = Convert<ImageOrNone>(PropertyId::ListStyleImage,
                                            u8"cross-fade(url(https://example.com/a.png), none, 25%)");
      REQUIRE(crossfade.IsImage());
      CHECK(ImageOf(crossfade)->IsCrossfadeImage());
      // The input image is pending.
      CHECK(ImageOf(crossfade)->IsPending());
      CHECK_FALSE(ImageOf(crossfade)->ImageHasRelativeWidth());
      CHECK(Serialised(crossfade) == "cross-fade(url(\"https://example.com/a.png\"), none, 0.25)");

      auto filter = Convert<ImageOrNone>(PropertyId::ListStyleImage,
                                         u8"filter(url(https://example.com/a.png), blur(2px))");
      REQUIRE(filter.IsImage());
      CHECK(ImageOf(filter)->IsFilterImage());
      CHECK(Serialised(filter) == "filter(url(\"https://example.com/a.png\"), blur(2px))");

      auto canvas = Convert<ImageOrNone>(PropertyId::ListStyleImage, u8"-webkit-canvas(drawing)");
      REQUIRE(canvas.IsImage());
      CHECK(ImageOf(canvas)->IsCanvasImage());
      CHECK(Serialised(canvas) == "-webkit-canvas(drawing)");

      auto named =
        Convert<ImageOrNone>(PropertyId::ListStyleImage, u8"-webkit-named-image(apple-pay-logo-white)");
      REQUIRE(named.IsImage());
      CHECK(ImageOf(named)->IsNamedImage());
      CHECK(Serialised(named) == "-webkit-named-image(apple-pay-logo-white)");
    }

    SECTION("image sets")
    {
      auto set = Convert<ImageOrNone>(PropertyId::ListStyleImage,
                                      u8"image-set(url(https://example.com/b.png) 2x, "
                                      u8"url(https://example.com/a.png) 1x type(\"image/png\"))");
      REQUIRE(set.IsImage());
      auto image = ImageOf(set);
      REQUIRE(image->IsImageSet());
      CHECK(Serialised(set)
            == "image-set(url(\"https://example.com/b.png\") 2dppx, url(\"https://example.com/a.png\") 1dppx "
               "type(\"image/png\"))");

      // Nothing is selected until the set loads.
      CHECK(image->IsPending());
      CHECK(image->SelectedImage() == nullptr);

      // The lowest resolution at or above the device scale factor (1) wins.
      auto best = Downcast<StyleImageSet>(*image).SelectBestFitImage(*Document);
      CHECK(best.ScaleFactor == 1);
      CHECK(best.Image->Url().Resolved.String() == u8"https://example.com/a.png");

      // Unsupported types are skipped.
      auto unsupported = ImageOf(Convert<ImageOrNone>(
        PropertyId::ListStyleImage, u8"image-set(url(https://example.com/a.png) 1x type(\"image/unknown\"), "
                                    u8"url(https://example.com/b.png) 3x)"));
      auto bestSupported = Downcast<StyleImageSet>(*unsupported).SelectBestFitImage(*Document);
      CHECK(bestSupported.ScaleFactor == 3);
    }

    SECTION("caching")
    {
      // A gradient that doesn't depend on the style is created once per CSS value.
      auto value = Parse(PropertyId::ListStyleImage, u8"linear-gradient(red, blue)");
      CHECK(State->CreateStyleImage(*value) == State->CreateStyleImage(*value));

      auto dependent = Parse(PropertyId::ListStyleImage, u8"linear-gradient(currentcolor, blue)");
      CHECK(State->CreateStyleImage(*dependent) != State->CreateStyleImage(*dependent));
    }

    SECTION("blending")
    {
      auto a = Convert<ImageOrNone>(PropertyId::ListStyleImage, u8"url(https://example.com/a.png)");
      auto b = Convert<ImageOrNone>(PropertyId::ListStyleImage, u8"url(https://example.com/b.png)");
      CHECK(Blending<ImageOrNone> {}.CanBlend(a, b));
      CHECK_FALSE(Blending<ImageOrNone> {}.CanBlend(a, ImageOrNone {css::keywords::None {}}));
      CHECK((Blending<ImageOrNone> {}.Blend(a, b, BlendingContext {0}) == a));
      CHECK((Blending<ImageOrNone> {}.Blend(a, b, BlendingContext {1}) == b));
      // Cross-fading needs both images loaded; until then, the destination image is used.
      CHECK((Blending<ImageOrNone> {}.Blend(a, b, BlendingContext {0.25}) == b));
      CHECK((Blending<ImageOrNone> {}.Blend(a, b, BlendingContext {0.75}) == b));
    }
  }

  TEST_CASE_METHOD(BuilderFixture, "Style Gradient", "[Style][Values][Images]")
  {
    auto styleRed = boo::Color {SRGBA<uint8> {255, 0, 0}};
    auto styleBlue = boo::Color {SRGBA<uint8> {0, 0, 255}};

    SECTION("linear")
    {
      LinearGradient linear {SRGBInterpolation(), Vertical {css::keywords::Bottom {}},
                             GradientLinearColorStopList {{Stop(styleRed), Stop(styleBlue)}}};
      Gradient gradient {css::FunctionNotation<css::ValueId::LinearGradient, LinearGradient> {linear}};
      CHECK(StopsAreCacheable(gradient));
      CHECK(IsOpaque(gradient, Style));

      auto platform = CreatePlatformGradient(gradient, {100, 50}, Style);
      const auto &data = std::get<boo::Gradient::LinearData>(platform->GetData());
      CHECK(data.Point0 == FloatPoint {0, 0});
      CHECK(data.Point1 == FloatPoint {0, 50});
      REQUIRE(platform->Stops().size() == 2);
      CHECK(platform->Stops().Stops()[0].Offset == 0);
      CHECK(platform->Stops().Stops()[0].Color == styleRed);
      CHECK(platform->Stops().Stops()[1].Offset == 1);
      CHECK(platform->SpreadMethod() == GradientSpreadMethod::Pad);
    }

    SECTION("stop positions")
    {
      // Unpositioned stops are spread evenly between their neighbours, and stops never go backwards.
      LinearGradient linear {SRGBInterpolation(), Vertical {css::keywords::Bottom {}},
                             GradientLinearColorStopList {{Stop(styleRed), Stop(styleBlue),
                                                           Stop(styleRed, 80.0f), Stop(styleBlue, 40.0f)}}};
      Gradient gradient {css::FunctionNotation<css::ValueId::LinearGradient, LinearGradient> {linear}};
      auto platform = CreatePlatformGradient(gradient, {100, 100}, Style);
      REQUIRE(platform->Stops().size() == 4);
      CHECK(platform->Stops().Stops()[0].Offset == 0);
      CHECK(platform->Stops().Stops()[1].Offset == Approx(0.4));
      CHECK(platform->Stops().Stops()[2].Offset == Approx(0.8));
      CHECK(platform->Stops().Stops()[3].Offset == Approx(0.8));
    }

    SECTION("repeating")
    {
      LinearGradient linear {SRGBInterpolation(), Vertical {css::keywords::Bottom {}},
                             GradientLinearColorStopList {{Stop(styleRed, 0.0f), Stop(styleBlue, 50.0f)}}};
      Gradient gradient {
        css::FunctionNotation<css::ValueId::RepeatingLinearGradient, LinearGradient> {linear}};
      auto platform = CreatePlatformGradient(gradient, {100, 100}, Style);
      // The stops repeat to fill the gradient line.
      REQUIRE(platform->Stops().size() > 2);
      CHECK(platform->Stops().Stops().back().Offset >= 1);
    }

    SECTION("current colour")
    {
      LinearGradient linear {
        SRGBInterpolation(), Vertical {css::keywords::Bottom {}},
        GradientLinearColorStopList {
          {GradientLinearColorStop {Markable<style::Color> {style::Color {css::keywords::Currentcolor {}}},
                                    std::nullopt},
           Stop(styleBlue)}}};
      Gradient gradient {css::FunctionNotation<css::ValueId::LinearGradient, LinearGradient> {linear}};
      CHECK_FALSE(StopsAreCacheable(gradient));
      Style.SetColor(styleRed);
      auto platform = CreatePlatformGradient(gradient, {10, 10}, Style);
      CHECK(platform->Stops().Stops()[0].Color == styleRed);
    }
  }

  TEST_CASE_METHOD(BuilderFixture, "Style ImageOrientation", "[Style][Values][Images]")
  {
    CHECK(Convert<ImageOrientation>(PropertyId::ImageOrientation, u8"none") == ImageOrientation::None);
    CHECK(Convert<ImageOrientation>(PropertyId::ImageOrientation, u8"from-image")
          == ImageOrientation::FromImage);
    CHECK(Serialised(ImageOrientation::FromImage) == "from-image");
    CHECK(ConvertToPlatform(ImageOrientation::None) == boo::ImageOrientation::Orientation::None);
    CHECK(ConvertToPlatform(ImageOrientation::FromImage) == boo::ImageOrientation::Orientation::FromImage);
  }

  TEST_CASE("Style images", "[Style][Values][Images]")
  {
    auto invalid = StyleInvalidImage::Create();
    CHECK(invalid->IsInvalidImage());
    // An invalid image is never equal, even to itself.
    CHECK_FALSE((*invalid == *invalid));
  }
}
