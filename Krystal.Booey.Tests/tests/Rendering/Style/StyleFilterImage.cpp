#include "Krystal.Booey/Rendering/Style/StyleFilterImage.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Loader/PageInFrameFixture.hpp"
#include "Krystal.Booey.Tests/Loader/TestLoader.hpp"
#include "Krystal.Booey.Tests/Platform/ImageDecoders/TestImages.hpp"
#include "Krystal.Booey/Page/Page.hpp"
#include "Krystal.Booey/Rendering/GeneratedContent/RenderListItem.hpp"
#include "Krystal.Booey/Rendering/GeneratedContent/RenderListMarker.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"

namespace krys::boo::tests
{
  TEST_CASE("StyleFilterImage::FixedSize: the input image's size", "[Rendering][StyleFilterImage]")
  {
    LoaderFixture loader;
    loader.Strategy.Resources[u8"file:///b.jpg"] = images::JPEG;

    PageInFrameFixture fixture(
      u8"<div id=image style='background-image: filter(url(file:///b.jpg), sepia())'></div>"
      u8"<div id=none style='background-image: filter(none, sepia())'></div>");
    RunNetworkingTasks();

    auto imageSize = [&](dom::DOMStringView id)
    {
      auto &renderer = *fixture.ById(id).Renderer();
      RefPtr image = renderer.Style().GetBackgroundLayers().UsedValues().begin()->GetImage().TryStyleImage();
      REQUIRE(image);
      CHECK(Is<StyleFilterImage>(*image));
      return image->ImageSize(&renderer, 1);
    };

    CHECK(imageSize(u8"image") == FloatSize(16, 8));
    CHECK(imageSize(u8"none") == FloatSize());
  }

  TEST_CASE("StyleFilterImage: its renderers hear when the input image loads",
            "[Rendering][StyleFilterImage]")
  {
    LoaderFixture loader;
    loader.Strategy.Resources[u8"file:///b.jpg"] = images::JPEG;
    loader.Strategy.Defer = true;

    PageInFrameFixture fixture(
      u8"<ul><li id=item style='list-style-image: filter(url(file:///b.jpg), sepia())'>x</li></ul>");
    RunNetworkingTasks();

    auto markerWidth = [&]
    {
      (void)fixture.Document->UpdateLayout();
      auto *marker = Downcast<RenderListItem>(*fixture.ById(u8"item").Renderer()).MarkerRenderer();
      REQUIRE(marker);
      return marker->Width();
    };

    auto widthBeforeLoad = markerWidth();
    loader.Strategy.CompleteDeferred();
    RunNetworkingTasks();

    // The marker is laid out again with the 16x8 image.
    CHECK(markerWidth() > widthBeforeLoad + 10);
  }

  TEST_CASE("StyleFilterImage: draws the input image through the filter", "[Rendering][StyleFilterImage]")
  {
    LoaderFixture loader;
    loader.Strategy.Resources[u8"file:///b.jpg"] = images::JPEG;

    // Filters need the page's rendering modes.
    auto page = Page::Create();
    PageInFrameFixture fixture(*page, u8"<style>body{margin:0}</style><div style='width:16px;height:8px;"
                                      u8"background-image: filter(url(file:///b.jpg), sepia())'></div>");
    RunNetworkingTasks();

    auto painted = fixture.Painted();
    INFO(css::tests::ToNarrow(painted));
    // The image is drawn once, inside the layer the filter applies to.
    auto layer = painted.find(u8"(begin-transparency-layer");
    auto image = painted.find(u8"(draw-native-image");
    REQUIRE(layer != utf8_string::npos);
    REQUIRE(image != utf8_string::npos);
    CHECK(layer < image);
    CHECK(painted.find(u8"(draw-native-image", image + 1) == utf8_string::npos);
  }
}
