#include "Krystal.Booey/SVG/SVGImageElement.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Loader/PageInFrameFixture.hpp"
#include "Krystal.Booey.Tests/Loader/TestLoader.hpp"
#include "Krystal.Booey.Tests/Platform/ImageDecoders/TestImages.hpp"
#include "Krystal.Booey/Rendering/RenderImageResource.hpp"
#include "Krystal.Booey/Rendering/RenderTreeAsText.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGImage.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo::svg::tests
{
  using boo::tests::LoaderFixture;
  using boo::tests::LoadEvents;
  using boo::tests::PageInFrameFixture;
  using boo::tests::RunNetworkingTasks;
  namespace images = boo::tests::images;

  namespace
  {
    KRYS_NODISCARD SVGImageElement &ImageById(PageInFrameFixture &fixture, dom::DOMStringView id) noexcept
    {
      return Downcast<SVGImageElement>(Downcast<SVGElement>(fixture.ById(id)));
    }

    KRYS_NODISCARD bool Contains(const utf8_string &text, utf8_stringview part) noexcept
    {
      return text.find(part) != utf8_string::npos;
    }
  }

  TEST_CASE("SVGImageElement: an image loads, fires load and paints", "[SVG][SVGImageElement]")
  {
    LoaderFixture loader;
    loader.Strategy.Resources[u8"file:///a.png"] = images::PNG;
    loader.Strategy.Defer = true;

    PageInFrameFixture fixture(
      u8R"html(<body style="margin: 0"><svg style="display: block" width=100 height=100>)html"
      u8R"html(<image id=fit href="file:///a.png" x=10 y=20 width=30 height=40 />)html"
      u8R"html(<image id=none href="file:///a.png" x=50 width=30 height=40 preserveAspectRatio=none />)html"
      u8R"html(<image id=auto href="file:///a.png" y=60 height=4 /></svg></body>)html");
    auto &image = ImageById(fixture, u8"fit");
    auto events = LoadEvents::ListenTo(image);

    // Each image loads its href.
    REQUIRE(loader.Strategy.Requests == List<utf8_string> {u8"file:///a.png"});
    CHECK_FALSE(Contains(fixture.Painted(), u8"draw-native-image"));

    loader.Strategy.CompleteDeferred();
    RunNetworkingTasks();

    CHECK(events->Loads == 1);
    CHECK(events->Errors == 0);
    auto *renderer = DynamicDowncast<LegacyRenderSVGImage>(image.Renderer());
    REQUIRE(renderer);
    CHECK(renderer->ImageResource().GetCachedImage() == image.GetCachedImage());

    // The image is fitted into its rectangle by preserveAspectRatio (centred, keeping its aspect ratio, by
    // default); an image without a width or height takes them from its natural size.
    auto painted = fixture.Painted();
    CHECK(Contains(painted, u8"(dest-rect at (10,30) size 30x20)"));
    CHECK(Contains(painted, u8"(dest-rect at (50,0) size 30x40)"));
    CHECK(Contains(painted, u8"(dest-rect at (0,60) size 6x4)"));

    CHECK(ExternalRepresentation(fixture.Frame.get()) == u8R"(layer at (0,0) size 200x100
  RenderView at (0,0) size 200x100
layer at (0,0) size 200x100
  RenderBlock {HTML} at (0,0) size 200x100
    RenderBody {BODY} at (0,0) size 200x100
      RenderSVGRoot {svg} at (0,0) size 80x64
        RenderSVGImage {image} at (10,20) size 30x40
        RenderSVGImage {image} at (50,0) size 30x40
        RenderSVGImage {image} at (0,60) size 6x4
)");

    // The image is hit in its rectangle, unless pointer-events says otherwise.
    CHECK(fixture.Document->ElementFromPoint(12, 22).get() == &image);
    CHECK(fixture.Document->ElementFromPoint(45, 22).get() != &image);
    (void)image.SetAttribute(u8"pointer-events", u8"none");
    CHECK(fixture.Document->ElementFromPoint(12, 22).get() != &image);

    // Changing its geometry updates its renderer.
    (void)image.SetAttribute(u8"x", u8"0");
    (void)image.SetAttribute(u8"preserveAspectRatio", u8"none");
    painted = fixture.Painted();
    CHECK(Contains(painted, u8"(dest-rect at (0,20) size 30x40)"));
  }

  TEST_CASE("SVGImageElement: a failed load fires error", "[SVG][SVGImageElement]")
  {
    LoaderFixture loader;
    PageInFrameFixture fixture(u8R"html(<svg><image id=i href="file:///missing.png" /></svg>)html");
    auto &image = ImageById(fixture, u8"i");
    auto events = LoadEvents::ListenTo(image);
    RunNetworkingTasks();

    CHECK(events->Loads == 0);
    CHECK(events->Errors == 1);
    CHECK_FALSE(Contains(fixture.Painted(), u8"draw-native-image"));

    // crossOrigin reflects the CORS setting.
    CHECK_FALSE(image.CrossOrigin());
    image.CrossOrigin(dom::DOMString(u8"bogus"));
    CHECK(image.CrossOrigin() == dom::DOMString(u8"anonymous"));
    (void)image.SetAttribute(u8"crossorigin", u8"USE-CREDENTIALS");
    CHECK(image.CrossOrigin() == dom::DOMString(u8"use-credentials"));
  }
}
