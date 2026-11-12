#include "Krystal.Booey/Rendering/Style/StyleCrossfadeImage.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Loader/PageInFrameFixture.hpp"
#include "Krystal.Booey.Tests/Loader/TestLoader.hpp"
#include "Krystal.Booey.Tests/Platform/ImageDecoders/TestImages.hpp"
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
}
