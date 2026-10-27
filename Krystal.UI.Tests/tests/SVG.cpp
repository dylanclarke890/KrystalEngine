#include "catch2/catch_approx.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/CSS/Values/SerialisationContext.hpp"
#include "Krystal.Booey/DOM/DOMRect.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/HTML/HTMLImageElement.hpp"
#include "Krystal.Booey/Loader/Cache/CachedImage.hpp"
#include "Krystal.Booey/Loader/Cache/MemoryCache.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Rendering/Style/StyleImage.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/SVGResources.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/SVGResourcesCache.hpp"
#include "Krystal.Booey/Style/Extractor.hpp"
#include "Krystal.Booey/SVG/Graphics/SVGImage.hpp"
#include "Krystal.Booey/SVG/SVGGeometryElement.hpp"
#include "Krystal.Booey/SVG/SVGGraphicsElement.hpp"
#include "Krystal.Booey/SVG/SVGSvgElement.hpp"
#include "Krystal.Booey/SVG/SVGTextContentElement.hpp"
#include "Krystal.Gfx.Tests/Golden.hpp"
#include "Krystal.UI.Tests/ViewTestUtils.hpp"
#include "Krystal.UI/View.hpp"

namespace krys::ui::tests
{
  using Catch::Approx;

  TEST_CASE("View: the editor page's SVG", "[UI]")
  {
    boo::MemoryCache::Singleton().EvictResources();
    View view({1'200, 600});
    REQUIRE(
      view.LoadFile(gfx::tests::GoldenDirectory() / ".." / ".." / "Krystal.Editor" / "pages" / "index.html")
        .has_value());
    WaitForLoad(view);
    // Its images load as networking tasks.
    for (int32 pass = 0; pass < 4; ++pass)
    {
      view.UpdateRendering();
      boo::html::EventLoop::Current().RunUntilIdle();
    }
    auto &document = view.Document();
    auto byId = [&](utf8_stringview id)
    {
      auto element = document.GetElementById(id);
      REQUIRE(element);
      return element;
    };
    auto svgById = [&](utf8_stringview id) -> boo::svg::SVGElement &
    {
      auto *element = DynamicDowncast<boo::svg::SVGElement>(byId(id).get());
      REQUIRE(element);
      return *element;
    };
    auto resourcesOf = [&](boo::svg::SVGElement &element)
    {
      auto *renderer = element.Renderer();
      REQUIRE(renderer);
      auto *resources = boo::SVGResourcesCache::CachedResourcesForRenderer(*renderer);
      REQUIRE(resources);
      return resources;
    };
    auto opacityOf = [&](boo::svg::SVGElement &element)
    {
      return boo::style::Extractor(&element).PropertyValueSerialisation(
        boo::css::PropertyId::Opacity, boo::css::DefaultSerialisationContext());
    };

    // The section in view.
    byId(u8"svg")->ScrollIntoView(true);
    view.UpdateRendering();

    SECTION("icons and shapes")
    {
      // Each icon is a symbol drawn at its <svg>'s size.
      auto small = byId(u8"icon-small")->GetBoundingClientRect();
      CHECK(small->Width() == 24);
      CHECK(small->Height() == 24);
      auto bounds = Downcast<boo::svg::SVGGraphicsElement>(svgById(u8"svg-shapes")).GetBBox();
      CHECK(bounds == boo::FloatRect(2, 5, 293, 60));
    }

    SECTION("SVG images")
    {
      auto *cup = DynamicDowncast<boo::html::HTMLImageElement>(byId(u8"svg-cup").get());
      REQUIRE(cup);
      CHECK(cup->Complete());
      CHECK(cup->NaturalWidth() == 48u);
      CHECK(cup->NaturalHeight() == 48u);
      REQUIRE(cup->GetCachedImage());
      CHECK(Is<boo::svg::SVGImage>(cup->GetCachedImage()->GetImage()));

      auto *tiles = byId(u8"svg-tiles")->Renderer();
      REQUIRE(tiles);
      auto background = tiles->Style().GetBackgroundLayers().UsedValues().begin()->GetImage().TryStyleImage();
      REQUIRE(background);
      CHECK(background->IsLoaded(tiles));
      REQUIRE(background->GetCachedImage());
      CHECK(Is<boo::svg::SVGImage>(background->GetCachedImage()->GetImage()));
    }

    SECTION("gradients, patterns, clipping, masking and filters")
    {
      // Each shape's paint server, clip, mask and filter resolves to its resource.
      auto &paint = svgById(u8"svg-paint");
      uint32 filled = 0;
      for (RefPtr child = paint.FirstElementChild(); child; child = child->NextElementSibling())
      {
        if (auto *shape = DynamicDowncast<boo::svg::SVGGeometryElement>(
              DynamicDowncast<boo::svg::SVGElement>(child.get()));
            shape && shape->Renderer())
        {
          auto *resources = resourcesOf(*shape);
          filled += resources->Fill() || resources->Stroke();
        }
      }
      CHECK(filled == 5u);

      auto &clipping = svgById(u8"svg-clipping");
      auto *clipped =
        DynamicDowncast<boo::svg::SVGElement>(clipping.LastElementChild()->PreviousElementSibling().get());
      auto *masked = DynamicDowncast<boo::svg::SVGElement>(clipping.LastElementChild().get());
      REQUIRE(clipped);
      REQUIRE(masked);
      CHECK(resourcesOf(*clipped)->Clipper());
      CHECK(resourcesOf(*masked)->Masker());
      CHECK(resourcesOf(svgById(u8"svg-cup-filtered"))->Filter());

      // The HTML box clipped to the star is hit only inside it: at its middle, not its corners.
      auto rect = byId(u8"svg-clipped")->GetBoundingClientRect();
      auto middle =
        document.ElementFromPoint(rect->Left() + rect->Width() / 2, rect->Top() + rect->Height() / 2);
      CHECK(middle.get() == byId(u8"svg-clipped").get());
      auto corner = document.ElementFromPoint(rect->Left() + 2, rect->Top() + 2);
      CHECK(corner.get() != byId(u8"svg-clipped").get());
    }

    SECTION("text on a path")
    {
      auto &svgText = svgById(u8"svg-text");
      auto *curved = DynamicDowncast<boo::svg::SVGTextContentElement>(
        DynamicDowncast<boo::svg::SVGElement>(svgText.FirstElementChild()->NextElementSibling().get()));
      REQUIRE(curved);
      // Its white space collapses.
      CHECK(curved->GetNumberOfChars() == 34u);
      CHECK(curved->GetComputedTextLength() > 0);
      // Its first glyphs climb the curve, and its middle ones are level at the curve's top.
      CHECK(curved->GetRotationOfChar(0).ReleaseValue() < -10);
      CHECK(curved->GetRotationOfChar(17).ReleaseValue() == Approx(0).margin(5));
    }

    SECTION("the animations")
    {
      auto &animated = Downcast<boo::svg::SVGSvgElement>(svgById(u8"svg-animated"));
      // They run from the page's load.
      CHECK_FALSE(animated.AnimationsPaused());

      auto &steam = Downcast<boo::svg::SVGGraphicsElement>(svgById(u8"svg-steam"));
      auto &spinning = Downcast<boo::svg::SVGGraphicsElement>(svgById(u8"svg-spinning"));
      auto &riding = Downcast<boo::svg::SVGGraphicsElement>(svgById(u8"svg-riding"));
      animated.PauseAnimations();

      // Five seconds in: the first steam is half way up and opaque, the bean has turned two thirds of the way
      // round its middle, and the riding bean is at the top of its track, level, on its way out.
      animated.SetCurrentTime(5);
      CHECK(opacityOf(steam) == u8"1");
      CHECK(steam.AnimatedLocalTransform().F() == Approx(2));
      auto turned = spinning.AnimatedLocalTransform();
      CHECK(turned.A() == Approx(-0.5));
      CHECK(turned.B() == Approx(-std::sqrt(3.) / 2));
      REQUIRE(riding.SupplementalTransform());
      CHECK(riding.SupplementalTransform()->E() == Approx(235));
      CHECK(riding.SupplementalTransform()->F() == Approx(27.5));
      CHECK(riding.SupplementalTransform()->B() == Approx(0).margin(0.01));

      // Six seconds in, the first steam has faded and started again; at eight, the riding bean is back at the
      // start of its track.
      animated.SetCurrentTime(6);
      CHECK(opacityOf(steam) == u8"0");
      animated.SetCurrentTime(8);
      CHECK(riding.SupplementalTransform()->E() == Approx(180));
      CHECK(riding.SupplementalTransform()->F() == Approx(80));
    }
  }
}
