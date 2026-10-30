#include "Krystal.Booey/Rendering/ImageQualityController.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Loader/PageInFrameFixture.hpp"
#include "Krystal.Booey.Tests/Loader/TestLoader.hpp"
#include "Krystal.Booey.Tests/Platform/ImageDecoders/TestImages.hpp"
#include "Krystal.Booey/Graphics/DisplayLists/DisplayListRecorderImpl.hpp"
#include "Krystal.Booey/Loader/Cache/CachedImage.hpp"
#include "Krystal.Booey/Rendering/RenderImage.hpp"
#include "Krystal.Booey/Rendering/RenderLayer.hpp"

namespace krys::boo::tests
{
  namespace
  {
    KRYS_NODISCARD size_t Count(const utf8_string &text, utf8_stringview part) noexcept
    {
      size_t count = 0;
      for (auto position = text.find(part); position != utf8_string::npos;
           position = text.find(part, position + 1))
      {
        ++count;
      }
      return count;
    }
  }

  TEST_CASE("ImageQualityController: image-rendering chooses the interpolation", "[Rendering][ImageQuality]")
  {
    LoaderFixture loader;
    loader.Strategy.Resources[u8"file:///a.png"] = images::PNG;

    PageInFrameFixture fixture(
      u8"<img id=auto src='file:///a.png'>"
      u8"<img id=pixelated src='file:///a.png' style='image-rendering:pixelated'>"
      u8"<img id=crisp src='file:///a.png' style='image-rendering:crisp-edges'>"
      u8"<img id=speed src='file:///a.png' style='image-rendering:optimizeSpeed'>"
      u8"<img id=quality src='file:///a.png' style='image-rendering:optimizeQuality'>");
    RunNetworkingTasks();
    (void)fixture.Document->UpdateLayout();

    display_list::RecorderImpl recorder(FloatSize(200, 100));
    auto quality = [&](dom::DOMStringView id)
    {
      auto *renderer = DynamicDowncast<RenderImage>(fixture.ById(id).Renderer());
      REQUIRE(renderer);
      auto *image = renderer->GetCachedImage()->ImageForRenderer(renderer);
      REQUIRE(image);
      CHECK(ImageQualityController::ChooseInterpolationQualityForSVG(recorder, *renderer, *image)
            == renderer->ChooseInterpolationQuality(recorder, *image, nullptr, LayoutSize(6, 4)));
      return renderer->ChooseInterpolationQuality(recorder, *image, nullptr, LayoutSize(6, 4));
    };

    CHECK(quality(u8"auto") == InterpolationQuality::Default);
    CHECK(quality(u8"pixelated") == InterpolationQuality::DoNotInterpolate);
    CHECK(quality(u8"crisp") == InterpolationQuality::DoNotInterpolate);
    CHECK(quality(u8"speed") == InterpolationQuality::Low);
    CHECK(quality(u8"quality") == InterpolationQuality::Default);

    // Only `auto` leaves the choice to the controller.
    auto &autoStyle = fixture.ById(u8"auto").Renderer()->Style();
    CHECK_FALSE(ImageQualityController::InterpolationQualityFromStyle(autoStyle).has_value());
    auto &qualityStyle = fixture.ById(u8"quality").Renderer()->Style();
    CHECK(ImageQualityController::InterpolationQualityFromStyle(qualityStyle)
          == InterpolationQuality::Default);
  }

  TEST_CASE("ImageQualityController: a bitmap image's opacity is an alpha", "[Rendering][ImageQuality]")
  {
    LoaderFixture loader;
    loader.Strategy.Resources[u8"file:///a.png"] = images::PNG;

    PageInFrameFixture fixture(u8"<img id=faded src='file:///a.png' style='opacity:0.5'>"
                               u8"<img id=bordered src='file:///a.png' style='opacity:0.5;border:1px solid'>"
                               u8"<img id=missing src='file:///none.png' style='opacity:0.5;width:4px'>");
    RunNetworkingTasks();
    (void)fixture.Document->UpdateLayout();

    auto layerOf = [&](dom::DOMStringView id)
    {
      auto *renderer = DynamicDowncast<RenderImage>(fixture.ById(id).Renderer());
      REQUIRE(renderer);
      REQUIRE(renderer->Layer());
      return renderer->Layer();
    };
    CHECK(layerOf(u8"faded")->IsBitmapOnly());
    CHECK(layerOf(u8"faded")->CanPaintTransparencyWithSetOpacity());
    CHECK_FALSE(layerOf(u8"bordered")->IsBitmapOnly());
    CHECK_FALSE(layerOf(u8"missing")->IsBitmapOnly());

    // Only the bordered and the missing images are drawn in transparency layers.
    auto painted = fixture.Painted();
    INFO(css::tests::ToNarrow(painted));
    CHECK(Count(painted, u8"begin-transparency-layer") == 2u);
    CHECK(Count(painted, u8"draw-native-image") == 2u);
  }
}
