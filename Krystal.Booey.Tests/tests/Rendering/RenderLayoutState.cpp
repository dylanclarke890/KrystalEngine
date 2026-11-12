#include "Krystal.Booey/Rendering/RenderLayoutState.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Page/LocalFrameViewLayoutContext.hpp"
#include "Krystal.Booey/Rendering/RenderBox.hpp"

namespace krys::boo::tests
{
  using style::tests::ResolverFixture;

  namespace
  {
    KRYS_NODISCARD RenderBox &LaidOutBoxOf(ResolverFixture &fixture, dom::DOMStringView id)
    {
      (void)fixture.Document->UpdateLayout();
      auto *box = DynamicDowncast<RenderBox>(fixture.ById(id).Renderer());
      REQUIRE(box);
      return *box;
    }
  }

  // The layout state caches the paint offset of the boxes being laid out, so mapping them to absolute
  // coordinates during layout agrees with mapping them through their containers.
  TEST_CASE_METHOD(css::tests::ValueFixture, "RenderLayoutState: the paint offset cache",
                   "[Rendering][RenderLayoutState]")
  {
    ResolverFixture fixture {u8R"html(<!DOCTYPE html><style>
      body { margin: 0; height: 2000px }
      #scroller { position: absolute; left: 10px; top: 20px; width: 100px; height: 100px; overflow: scroll }
      #content { height: 1000px }
      #child, #inner { width: 10px; height: 10px }
      #child { margin-left: 20px }
      #rel { position: relative; left: 5px; top: 7px; padding: 2px }
      #fixed { position: fixed; left: 3px; top: 4px; padding: 1px }
    </style><div id=scroller><div id=content><div id=child></div><div id=rel><div id=inner></div></div>
    <div id=fixed><div id=fixedInner></div></div></div></div><div id=other></div>)html"};
    fixture.ById(u8"scroller").SetScrollTop(30);
    // After the first layout every box repaints in full, and boxes push no layout states of their own.
    (void)fixture.Document->UpdateLayout();
    REQUIRE_FALSE(fixture.ById(u8"other").SetAttribute(u8"style", u8"height: 1px").HasException());
    (void)fixture.Document->UpdateLayout();
    REQUIRE_FALSE(fixture.Frame->View()->LayoutContext().NeedsFullRepaint());
    auto &scroller = LaidOutBoxOf(fixture, u8"scroller");
    auto &content = LaidOutBoxOf(fixture, u8"content");
    auto &child = LaidOutBoxOf(fixture, u8"child");
    auto &rel = LaidOutBoxOf(fixture, u8"rel");
    auto &inner = LaidOutBoxOf(fixture, u8"inner");
    auto &fixed = LaidOutBoxOf(fixture, u8"fixed");
    auto &fixedInner = LaidOutBoxOf(fixture, u8"fixedInner");
    auto &layoutContext = fixture.Frame->View()->LayoutContext();

    // Without a layout state, the boxes map through their containers.
    REQUIRE_FALSE(layoutContext.IsPaintOffsetCacheEnabled());
    auto contentPoint = content.LocalToAbsolute();
    auto childPoint = child.LocalToAbsolute();
    auto innerPoint = inner.LocalToAbsolute();
    auto fixedInnerPoint = fixedInner.LocalToAbsolute();
    CHECK(contentPoint == FloatPoint(10, -10));
    CHECK(childPoint == FloatPoint(30, -10));
    CHECK(innerPoint == FloatPoint(17, 9));
    CHECK(fixedInnerPoint == FloatPoint(4, 5));

    SECTION("A subtree root starts from its container's position, clipped and scrolled by it")
    {
      SubtreeLayoutStateMaintainer subtree(&content);
      REQUIRE(layoutContext.IsPaintOffsetCacheEnabled());
      const auto *layoutState = layoutContext.LayoutState();
      REQUIRE(layoutState);
      CHECK(layoutState->PaintOffset() == LayoutSize(10, -10));
      CHECK(layoutState->IsClipped());
      CHECK(layoutState->ClipRect() == LayoutRect(LayoutPoint(10, 20), scroller.CachedSizeForOverflowClip()));
      CHECK(content.LocalToAbsolute() == contentPoint);

      SECTION("Its children move by their own offsets")
      {
        LayoutStateMaintainer contentState(content, content.LocationOffset());
        CHECK(child.LocalToAbsolute() == childPoint);
      }

      SECTION("In-flow positioned boxes move their children by their position")
      {
        LayoutStateMaintainer contentState(content, content.LocationOffset());
        LayoutStateMaintainer relState(rel, rel.LocationOffset());
        CHECK(inner.LocalToAbsolute() == innerPoint);
      }

      SECTION("Fixed positioned boxes start from the view")
      {
        LayoutStateMaintainer contentState(content, content.LocationOffset());
        LayoutStateMaintainer fixedState(fixed, fixed.LocationOffset());
        CHECK(fixedInner.LocalToAbsolute() == fixedInnerPoint);
      }
    }

    SECTION("A box that clips its overflow scrolls its children and clips them")
    {
      SubtreeLayoutStateMaintainer subtree(&scroller);
      LayoutStateMaintainer scrollerState(scroller, scroller.LocationOffset());
      const auto *layoutState = layoutContext.LayoutState();
      REQUIRE(layoutState);
      CHECK(layoutState->PaintOffset() == LayoutSize(10, -10));
      CHECK(layoutState->IsClipped());
      CHECK(layoutState->ClipRect() == LayoutRect(LayoutPoint(10, 20), scroller.CachedSizeForOverflowClip()));
      CHECK(content.LocalToAbsolute() == contentPoint);
    }
  }
}
