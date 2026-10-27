#include "Krystal.Booey/SVG/Graphics/SVGImage.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Loader/PageInFrameFixture.hpp"
#include "Krystal.Booey.Tests/Loader/TestLoader.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorCheckerTestFunctions.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/HTML/HTMLImageElement.hpp"
#include "Krystal.Booey/Loader/Cache/CachedImage.hpp"
#include "Krystal.Booey/Rendering/RenderBox.hpp"
#include "Krystal.Booey/SVG/Graphics/SVGImageForContainer.hpp"
#include "Krystal.Booey/SVG/SVGSvgElement.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo::svg::tests
{
  using boo::tests::LoaderFixture;
  using boo::tests::PageInFrameFixture;
  using boo::tests::RunNetworkingTasks;

  namespace
  {
    constexpr utf8_stringview GreenRect =
      u8R"svg(<svg xmlns="http://www.w3.org/2000/svg" width="40" height="20">)svg"
      u8R"svg(<rect width="40" height="20" fill="green"/></svg>)svg";

    KRYS_NODISCARD bool Contains(const utf8_string &text, utf8_stringview part) noexcept
    {
      return text.find(part) != utf8_string::npos;
    }

    KRYS_NODISCARD html::HTMLImageElement &ImageById(PageInFrameFixture &fixture,
                                                     dom::DOMStringView id) noexcept
    {
      return Downcast<html::HTMLImageElement>(Downcast<html::HTMLElement>(fixture.ById(id)));
    }

    KRYS_NODISCARD SVGImage *SVGImageOf(html::HTMLImageElement &image) noexcept
    {
      auto *cachedImage = image.GetCachedImage();
      return cachedImage ? DynamicDowncast<SVGImage>(cachedImage->GetImage()) : nullptr;
    }
  }

  TEST_CASE("SVGImage: an SVG file in an <img> draws its document", "[SVG][SVGImage]")
  {
    LoaderFixture loader;
    loader.Strategy.AddText(u8"file:///a.svg", GreenRect, u8"image/svg+xml");
    PageInFrameFixture fixture(u8R"html(<body style="margin: 0"><img id=i src="file:///a.svg">)html"
                               u8R"html(<img id=wide src="file:///a.svg" width=80></body>)html");
    RunNetworkingTasks();

    auto &image = ImageById(fixture, u8"i");
    auto *svgImage = SVGImageOf(image);
    REQUIRE(svgImage);
    CHECK(svgImage->Size() == FloatSize(40, 20));
    CHECK(image.NaturalWidth() == 40);
    CHECK(image.NaturalHeight() == 20);
    auto root = svgImage->RootElement();
    REQUIRE(root);
    CHECK(root->NodeDocument().ContentType() == u8"image/svg+xml");

    // Each image is drawn at its own size: its own container, with the document laid out at that size.
    (void)fixture.Document->UpdateLayout();
    auto *renderer = DynamicDowncast<RenderBox>(image.Renderer());
    REQUIRE(renderer);
    CHECK(renderer->Size() == LayoutSize(40, 20));
    auto *forContainer = image.GetCachedImage()->ImageForRenderer(renderer);
    CHECK(Is<SVGImageForContainer>(*forContainer));
    CHECK(forContainer->Size() == FloatSize(40, 20));

    auto &wide = ImageById(fixture, u8"wide");
    auto *wideRenderer = DynamicDowncast<RenderBox>(wide.Renderer());
    REQUIRE(wideRenderer);
    CHECK(wideRenderer->Size() == LayoutSize(80, 40));
    CHECK(wide.GetCachedImage()->ImageForRenderer(wideRenderer)->Size() == FloatSize(80, 40));

    auto painted = fixture.Painted();
    CHECK(Contains(painted, u8"(color #008000)"));
    CHECK(Contains(painted, u8"(rect at (40,0) size 80x40)"));
  }

  TEST_CASE("SVGImage: an SVG image without a size takes its aspect ratio from its view box",
            "[SVG][SVGImage]")
  {
    LoaderFixture loader;
    loader.Strategy.AddText(u8"file:///ratio.svg",
                            u8R"svg(<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 10 5"/>)svg",
                            u8"image/svg+xml");
    PageInFrameFixture fixture(
      u8R"html(<body style="margin: 0"><img id=i src="file:///ratio.svg" width=60>)html", {400, 300});
    RunNetworkingTasks();

    auto &image = ImageById(fixture, u8"i");
    auto *svgImage = SVGImageOf(image);
    REQUIRE(svgImage);
    // Without a width or height, the image's own size is its view box's.
    CHECK(svgImage->Size() == FloatSize(10, 5));
    (void)fixture.Document->UpdateLayout();
    auto *renderer = DynamicDowncast<RenderBox>(image.Renderer());
    REQUIRE(renderer);
    CHECK(renderer->Size() == LayoutSize(60, 30));
  }

  TEST_CASE("SVGImage: a sized SVG image without a view box synthesises one", "[SVG][SVGImage]")
  {
    LoaderFixture loader;
    loader.Strategy.AddText(u8"file:///a.svg", GreenRect, u8"image/svg+xml");
    PageInFrameFixture fixture(u8R"html(<img id=i src="file:///a.svg">)html");
    RunNetworkingTasks();

    auto *svgImage = SVGImageOf(ImageById(fixture, u8"i"));
    REQUIRE(svgImage);
    auto root = svgImage->RootElement();
    REQUIRE(root);
    CHECK(root->ViewBox().IsEmpty());
    CHECK(root->CurrentViewBoxRect() == FloatRect(0, 0, 40, 20));
    CHECK(IsInSVGImage(root.get()));
    CHECK_FALSE(IsInSVGImage(&ImageById(fixture, u8"i")));
  }

  TEST_CASE("SVGImage: an SVG image loads nothing but data: URLs and has no links", "[SVG][SVGImage]")
  {
    LoaderFixture loader;
    loader.Strategy.AddText(
      u8"file:///links.svg",
      u8R"svg(<svg xmlns="http://www.w3.org/2000/svg" width="10" height="10"><a id="a" href="file:///x">)svg"
      u8R"svg(<image id="external" href="file:///b.png" width="5" height="5"/></a></svg>)svg",
      u8"image/svg+xml");
    PageInFrameFixture fixture(u8R"html(<img id=i src="file:///links.svg">)html");
    RunNetworkingTasks();

    auto *svgImage = SVGImageOf(ImageById(fixture, u8"i"));
    REQUIRE(svgImage);
    CHECK(loader.Strategy.Requests == List<utf8_string> {u8"file:///links.svg"});

    auto &document = svgImage->RootElement()->NodeDocument();
    auto link = document.GetElementById(u8"a");
    REQUIRE(link);
    CHECK_FALSE(css::IsLink(*link));
  }

  TEST_CASE("SVGImage: an SVG background image draws", "[SVG][SVGImage]")
  {
    LoaderFixture loader;
    loader.Strategy.AddText(u8"file:///a.svg", GreenRect, u8"image/svg+xml");
    PageInFrameFixture fixture(
      u8R"html(<body style="margin: 0"><div style="width: 40px; height: 20px; )html"
      u8R"html(background-image: url(file:///a.svg); background-repeat: no-repeat"></div></body>)html");
    RunNetworkingTasks();

    auto painted = fixture.Painted();
    CHECK(Contains(painted, u8"(color #008000)"));
  }
}
