#include "Krystal.Booey/Rendering/Style/StyleFilterImage.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Loader/PageInFrameFixture.hpp"
#include "Krystal.Booey.Tests/Loader/TestLoader.hpp"
#include "Krystal.Booey.Tests/Platform/ImageDecoders/TestImages.hpp"
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
}
