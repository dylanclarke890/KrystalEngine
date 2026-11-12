#include "Krystal.Booey/Rendering/Style/StyleCrossfadeImage.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Loader/PageInFrameFixture.hpp"
#include "Krystal.Booey.Tests/Loader/TestLoader.hpp"
#include "Krystal.Booey.Tests/Platform/ImageDecoders/TestImages.hpp"
#include "Krystal.Booey/Graphics/Animation/AnimationUtilities.hpp"
#include "Krystal.Booey/Rendering/GeneratedContent/RenderListItem.hpp"
#include "Krystal.Booey/Rendering/GeneratedContent/RenderListMarker.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"

namespace krys::boo::tests
{
  TEST_CASE("StyleCrossfadeImage::FixedSize: the input images' sizes, interpolated",
            "[Rendering][StyleCrossfadeImage]")
  {
    LoaderFixture loader;
    loader.Strategy.Resources[u8"file:///a.png"] = images::PNG;
    loader.Strategy.Resources[u8"file:///b.jpg"] = images::JPEG;

    PageInFrameFixture fixture(
      u8"<div id=mixed style='background-image: cross-fade(url(file:///a.png), url(file:///b.jpg), "
      u8"25%)'></div>"
      u8"<div id=same style='background-image: cross-fade(url(file:///a.png), url(file:///a.png), "
      u8"25%)'></div>"
      u8"<div id=none style='background-image: cross-fade(none, url(file:///a.png), 25%)'></div>");
    RunNetworkingTasks();

    auto imageSize = [&](dom::DOMStringView id)
    {
      auto &renderer = *fixture.ById(id).Renderer();
      RefPtr image = renderer.Style().GetBackgroundLayers().UsedValues().begin()->GetImage().TryStyleImage();
      REQUIRE(image);
      CHECK(Is<StyleCrossfadeImage>(*image));
      return image->ImageSize(&renderer, 1);
    };

    // A quarter of the way from the 3x2 PNG to the 16x8 JPEG.
    CHECK(imageSize(u8"mixed") == FloatSize(6.25f, 3.5f));
    // Images of the same size keep it exactly.
    CHECK(imageSize(u8"same") == FloatSize(3, 2));
    // Without both images, the size is empty.
    CHECK(imageSize(u8"none") == FloatSize());
  }

  TEST_CASE("StyleCrossfadeImage::Blend: the percentages, once the input images have loaded",
            "[Rendering][StyleCrossfadeImage]")
  {
    LoaderFixture loader;
    loader.Strategy.Resources[u8"file:///a.png"] = images::PNG;
    loader.Strategy.Resources[u8"file:///b.jpg"] = images::JPEG;

    PageInFrameFixture fixture(
      u8"<div id=from style='background-image: cross-fade(url(file:///a.png), url(file:///b.jpg), "
      u8"20%)'></div>"
      u8"<div id=to style='background-image: cross-fade(url(file:///a.png), url(file:///b.jpg), "
      u8"60%)'></div>");
    RunNetworkingTasks();

    auto crossfade = [&](dom::DOMStringView id) -> StyleCrossfadeImage &
    {
      auto &renderer = *fixture.ById(id).Renderer();
      return Downcast<StyleCrossfadeImage>(
        *renderer.Style().GetBackgroundLayers().UsedValues().begin()->GetImage().TryStyleImage());
    };

    auto blended = crossfade(u8"to").Blend(crossfade(u8"from"), BlendingContext(0.5));
    REQUIRE(blended);
    CHECK(blended->Percentage() == 0.4);
    CHECK(blended->EqualInputImages(crossfade(u8"from")));
  }

  TEST_CASE("StyleCrossfadeImage: its renderers hear when an input image loads",
            "[Rendering][StyleCrossfadeImage]")
  {
    LoaderFixture loader;
    loader.Strategy.Resources[u8"file:///b.jpg"] = images::JPEG;
    loader.Strategy.Defer = true;

    PageInFrameFixture fixture(u8"<ul><li id=item style='list-style-image: cross-fade(url(file:///b.jpg), "
                               u8"url(file:///b.jpg), 50%)'>x</li></ul>");
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

  TEST_CASE("StyleCrossfadeImage: draws both input images", "[Rendering][StyleCrossfadeImage]")
  {
    LoaderFixture loader;
    loader.Strategy.Resources[u8"file:///a.png"] = images::PNG;
    loader.Strategy.Resources[u8"file:///b.jpg"] = images::JPEG;

    PageInFrameFixture fixture(
      u8"<style>body{margin:0}</style><div style='width:16px;height:8px;background-repeat:no-repeat;"
      u8"background-image: cross-fade(url(file:///a.png), url(file:///b.jpg), 25%)'></div>");
    RunNetworkingTasks();

    auto painted = fixture.Painted();
    INFO(css::tests::ToNarrow(painted));
    auto count = [&](utf8_stringview text)
    {
      size_t found = 0;
      for (auto position = painted.find(text); position != utf8_string::npos;
           position = painted.find(text, position + 1))
      {
        ++found;
      }
      return found;
    };
    CHECK(count(u8"(draw-native-image") == 2);
    CHECK(count(u8"(begin-transparency-layer") >= 1);
  }
}
