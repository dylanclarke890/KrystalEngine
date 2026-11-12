#include "Krystal.Booey/SVG/Graphics/SVGImage.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Loader/PageInFrameFixture.hpp"
#include "Krystal.Booey.Tests/Loader/TestLoader.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorCheckerTestFunctions.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLImageElement.hpp"
#include "Krystal.Booey/Loader/Cache/CachedImage.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/Page.hpp"
#include "Krystal.Booey/Page/Settings.hpp"
#include "Krystal.Booey/Rendering/RenderBox.hpp"
#include "Krystal.Booey/SVG/Animation/SMILTimeContainer.hpp"
#include "Krystal.Booey/SVG/Elements/SVGRectElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGSvgElement.hpp"
#include "Krystal.Booey/SVG/Graphics/SVGImageForContainer.hpp"
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

  TEST_CASE("SVGImage: an SVG image's animations run in the rendering update of the page showing it",
            "[SVG][SVGImage]")
  {
    LoaderFixture loader;
    loader.Strategy.AddText(
      u8"file:///moving.svg",
      u8R"svg(<svg xmlns="http://www.w3.org/2000/svg" width="40" height="20"><rect id="r" width="10")svg"
      u8R"svg( height="10"><animate attributeName="x" from="0" to="100" dur="1s"/></rect></svg>)svg",
      u8"image/svg+xml");
    auto testPage = Page::Create();
    PageInFrameFixture fixture(*testPage, u8R"html(<img id=i src="file:///moving.svg">)html");
    RunNetworkingTasks();

    auto *svgImage = SVGImageOf(ImageById(fixture, u8"i"));
    REQUIRE(svgImage);
    auto root = svgImage->RootElement();
    REQUIRE(root);
    auto *page = fixture.Frame->GetPage();
    REQUIRE(page);
    (void)fixture.Document->UpdateLayout();
    page->UpdateRendering();
    svgImage->StartAnimation();
    html::EventLoop::Current().RunUntilIdle();
    REQUIRE(root->TimeContainer().IsActive());
    auto rectElement = root->NodeDocument().GetElementById(u8"r");
    REQUIRE(rectElement);
    auto &rect = Downcast<SVGRectElement>(Downcast<SVGElement>(*rectElement));

    // The image's page asks the page showing it for its rendering updates, and is updated in them.
    for (int i = 0; i < 2; ++i)
    {
      CHECK(page->IsRenderingUpdateScheduled());
      auto x = rect.X().ValueInSpecifiedUnits();
      page->UpdateRendering();
      CHECK(rect.X().ValueInSpecifiedUnits() > x);
    }

    // Held by the page showing it, the image's animations pause, and play again after.
    page->SetImageAnimationEnabled(false);
    CHECK(root->AnimationsPaused());
    CHECK_FALSE(SVGImageOf(ImageById(fixture, u8"i"))->GetImageObserver()->AllowsAnimation(*svgImage));
    page->SetImageAnimationEnabled(true);
    CHECK_FALSE(root->AnimationsPaused());
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
  TEST_CASE("SVGImage: an SVG image takes its loader's settings and the dark appearance of what shows it",
            "[SVG][SVGImage]")
  {
    LoaderFixture loader;
    loader.Strategy.AddText(
      u8"file:///scheme.svg",
      u8R"svg(<svg xmlns="http://www.w3.org/2000/svg" width="40" height="20"><style>rect { fill: red })svg"
      u8R"svg( @media (prefers-color-scheme: dark) { rect { fill: lime } }</style>)svg"
      u8R"svg(<rect width="40" height="20"/></svg>)svg",
      u8"image/svg+xml");
    auto page = Page::Create();
    auto environment = page->MainFrame().GetMediaEnvironment();
    environment.UseDarkAppearance = true;
    page->MainFrame().SetMediaEnvironment(environment);
    PageInFrameFixture fixture(*page, u8R"html(<body style="margin: 0"></body>)html");

    auto settings = Settings::Create();
    fixture.Document->SetSettings(Ref<Settings>(settings));
    auto image = fixture.Document->CreateElement(u8"img").ReleaseValue();
    REQUIRE_FALSE(image->SetAttribute(u8"id", u8"i").HasException());
    REQUIRE_FALSE(image->SetAttribute(u8"src", u8"file:///scheme.svg").HasException());
    REQUIRE_FALSE(fixture.Document->Body()->AppendChild(*image).HasException());
    (void)fixture.Document->UpdateLayout();
    RunNetworkingTasks();

    auto *svgImage = SVGImageOf(ImageById(fixture, u8"i"));
    REQUIRE(svgImage);
    auto root = svgImage->RootElement();
    REQUIRE(root);
    CHECK(&root->NodeDocument().GetSettings() == settings.get());
    auto painted = fixture.Painted();
    CHECK(Contains(painted, u8"(color #00FF00)"));
    CHECK_FALSE(Contains(painted, u8"(color #FF0000)"));
  }
}
