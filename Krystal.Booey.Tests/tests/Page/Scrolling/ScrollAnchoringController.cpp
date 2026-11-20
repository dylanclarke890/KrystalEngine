#include "Krystal.Booey/Page/Scrolling/ScrollAnchoringController.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Animation/AnimationTestUtils.hpp"
#include "Krystal.Booey/DOM/Nodes/FocusOptions.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLElement.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Rendering/Layers/RenderLayerScrollableArea.hpp"
#include "Krystal.Booey/Rendering/RenderLayerModelObject.hpp"

namespace krys::boo::tests
{
  namespace
  {
    /// @brief A page of twenty 50 pixel blocks (`d0` to `d19`) after an empty one (`top`), all in `w`, in the
    /// 200x100 view, with @p style.
    KRYS_NODISCARD utf8_string AnchoringPage(utf8_stringview style = {}, utf8_stringview block7 = {})
    {
      utf8_string markup = u8"<!DOCTYPE html><style>body{margin:0} #w>*{display:block;height:50px;margin:0}"
                           u8" #top{height:0}";
      markup += style;
      markup += u8"</style><div id=w><div id=top></div>";
      for (int32 i = 0; i < 20; ++i)
      {
        auto id = std::to_string(i);
        if (i == 7 && !block7.empty())
        {
          markup += block7;
          continue;
        }
        markup += u8"<div id=d";
        markup.append(id.begin(), id.end());
        markup += u8"></div>";
      }
      markup += u8"</div>";
      return markup;
    }

    KRYS_NODISCARD LocalFrameView &View(AnimationPageFixture &fixture) noexcept
    {
      return *fixture.TestFrame->View();
    }

    KRYS_NODISCARD int32 ViewY(AnimationPageFixture &fixture) noexcept
    {
      return View(fixture).GetScrollPosition().Y();
    }

    void ScrollViewTo(AnimationPageFixture &fixture, int32 y) noexcept
    {
      View(fixture).SetScrollPosition({0, y});
      REQUIRE(ViewY(fixture) == y);
    }
  }

  TEST_CASE("ScrollAnchoringController: the view keeps its anchor where it was as layout moves it",
            "[Page][Scrolling][ScrollAnchoringController][css-scroll-anchoring:scroll-adjustment]"
            "[css-scroll-anchoring:anchor-node-selection]")
  {
    SECTION("content growing above the anchor scrolls the view as far")
    {
      AnimationPageFixture fixture {AnchoringPage()};
      ScrollViewTo(fixture, 300);
      REQUIRE(View(fixture).GetScrollAnchoringController()->AnchorElement() == &fixture.ById(u8"d6"));
      fixture.SetStyle(u8"top", u8"height:120px");
      fixture.Frame();
      CHECK(ViewY(fixture) == 420);

      // And again, from the adjusted position.
      fixture.SetStyle(u8"top", u8"height:20px");
      fixture.Frame();
      CHECK(ViewY(fixture) == 320);
    }

    SECTION("scrolling chooses the anchor again")
    {
      AnimationPageFixture fixture {AnchoringPage()};
      ScrollViewTo(fixture, 300);
      ScrollViewTo(fixture, 500);
      CHECK(View(fixture).GetScrollAnchoringController()->AnchorElement() == &fixture.ById(u8"d10"));
      fixture.SetStyle(u8"top", u8"height:120px");
      fixture.Frame();
      CHECK(ViewY(fixture) == 620);
    }

    SECTION("content growing below the anchor doesn't")
    {
      AnimationPageFixture fixture {AnchoringPage()};
      ScrollViewTo(fixture, 300);
      fixture.SetStyle(u8"d15", u8"height:200px");
      fixture.Frame();
      CHECK(ViewY(fixture) == 300);
    }

    SECTION("a view at the top has no anchor")
    {
      AnimationPageFixture fixture {AnchoringPage()};
      fixture.SetStyle(u8"top", u8"height:120px");
      fixture.Frame();
      CHECK(ViewY(fixture) == 0);
      CHECK_FALSE(View(fixture).GetScrollAnchoringController()->AnchorElement());
    }

    SECTION("an anchor that's removed is replaced")
    {
      AnimationPageFixture fixture {AnchoringPage()};
      ScrollViewTo(fixture, 300);
      fixture.ById(u8"d6").Remove();
      fixture.Frame();
      CHECK(ViewY(fixture) == 300);
      CHECK(View(fixture).GetScrollAnchoringController()->AnchorElement() == &fixture.ById(u8"d7"));
    }
  }

  TEST_CASE("ScrollAnchoringController: the anchor is the first element wholly in view",
            "[Page][Scrolling][ScrollAnchoringController][css-scroll-anchoring:candidate-examination]")
  {
    SECTION("rather than its descendants")
    {
      AnimationPageFixture fixture {
        AnchoringPage({}, u8"<div id=d7><p id=p style=height:10px;margin:0></p></div>")};
      ScrollViewTo(fixture, 350);
      CHECK(View(fixture).GetScrollAnchoringController()->AnchorElement() == &fixture.ById(u8"d7"));
    }

    SECTION("that has an area")
    {
      AnimationPageFixture fixture {AnchoringPage({}, u8"<div id=d7 style=height:0></div>")};
      ScrollViewTo(fixture, 350);
      CHECK(View(fixture).GetScrollAnchoringController()->AnchorElement() == &fixture.ById(u8"d8"));
    }
  }

  TEST_CASE("ScrollAnchoringController: overflow-anchor: none excludes a subtree",
            "[Page][Scrolling][ScrollAnchoringController][css-scroll-anchoring:candidate-examination]")
  {
    SECTION("on the root, the view doesn't anchor")
    {
      AnimationPageFixture fixture {AnchoringPage(u8"html{overflow-anchor:none}")};
      ScrollViewTo(fixture, 300);
      fixture.SetStyle(u8"top", u8"height:120px");
      fixture.Frame();
      CHECK(ViewY(fixture) == 300);
    }

    SECTION("on an element, the next one is the anchor")
    {
      AnimationPageFixture fixture {AnchoringPage(u8"#d6{overflow-anchor:none}")};
      ScrollViewTo(fixture, 300);
      CHECK(View(fixture).GetScrollAnchoringController()->AnchorElement() == &fixture.ById(u8"d7"));
      fixture.SetStyle(u8"top", u8"height:120px");
      fixture.Frame();
      CHECK(ViewY(fixture) == 420);
    }
  }

  TEST_CASE("ScrollAnchoringController: a suppression trigger on the anchor's ancestors stops the adjustment",
            "[Page][Scrolling][ScrollAnchoringController][css-scroll-anchoring:suppression-triggers]")
  {
    AnimationPageFixture fixture {AnchoringPage()};
    ScrollViewTo(fixture, 300);

    SECTION("a change to their padding")
    {
      fixture.SetStyle(u8"w", u8"padding-top:120px");
      fixture.Frame();
      CHECK(ViewY(fixture) == 300);
      // The next layout anchors again.
      fixture.SetStyle(u8"top", u8"height:10px");
      fixture.Frame();
      CHECK(ViewY(fixture) == 310);
    }

    SECTION("but not one to an element outside the anchor's chain")
    {
      fixture.SetStyle(u8"d2", u8"height:170px");
      fixture.Frame();
      CHECK(ViewY(fixture) == 420);
    }

    SECTION("something becoming out of flow")
    {
      fixture.SetStyle(u8"d2", u8"position:absolute");
      fixture.Frame();
      CHECK(ViewY(fixture) == 300);
    }
  }

  TEST_CASE("ScrollAnchoringController: a focused text control is a priority candidate",
            "[Page][Scrolling][ScrollAnchoringController][css-scroll-anchoring:priority-candidates]")
  {
    AnimationPageFixture fixture {AnchoringPage({}, u8"<textarea id=t></textarea>")};
    Downcast<HTMLElement>(fixture.ById(u8"t")).Focus(dom::FocusOptions {.PreventScroll = true});
    ScrollViewTo(fixture, 300);
    CHECK(View(fixture).GetScrollAnchoringController()->AnchorElement() == &fixture.ById(u8"t"));

    // The block above it grows, which moves the text control but not the first block in view.
    fixture.SetStyle(u8"d6", u8"height:80px");
    fixture.Frame();
    CHECK(ViewY(fixture) == 330);
  }

  TEST_CASE("ScrollAnchoringController: a focused element that isn't editable isn't a priority candidate",
            "[Page][Scrolling][ScrollAnchoringController][css-scroll-anchoring:priority-candidates]")
  {
    AnimationPageFixture fixture {AnchoringPage({}, u8"<div id=t tabindex=0></div>")};
    Downcast<HTMLElement>(fixture.ById(u8"t")).Focus(dom::FocusOptions {.PreventScroll = true});
    ScrollViewTo(fixture, 300);
    CHECK(View(fixture).GetScrollAnchoringController()->AnchorElement() == &fixture.ById(u8"d6"));
  }

  TEST_CASE("ScrollAnchoringController: a scroller keeps its anchor where it was",
            "[Page][Scrolling][ScrollAnchoringController][css-scroll-anchoring:scroll-adjustment]")
  {
    AnimationPageFixture fixture {
      utf8_string(u8"<!DOCTYPE html><style>body{margin:0} #s{height:100px;overflow:auto}"
                  u8" #s>div{height:50px} #s>#st{height:0}</style><div id=s><div id=st></div>"
                  u8"<div></div><div></div><div id=c2></div><div></div><div></div><div></div></div>")};
    auto &scroller = fixture.ById(u8"s");
    scroller.SetScrollTop(100);
    fixture.Frame();
    REQUIRE(scroller.ScrollTop() == 100);
    auto *area = Downcast<RenderLayerModelObject>(*scroller.Renderer()).Layer()->GetScrollableArea();
    REQUIRE(area);
    REQUIRE(area->GetScrollAnchoringController());
    REQUIRE(area->GetScrollAnchoringController()->AnchorElement() == &fixture.ById(u8"c2"));

    fixture.SetStyle(u8"st", u8"height:30px");
    fixture.Frame();
    CHECK(scroller.ScrollTop() == 130);
    CHECK(ViewY(fixture) == 0);

    SECTION("but not with overflow-anchor: none")
    {
      fixture.SetStyle(u8"s", u8"overflow-anchor:none");
      fixture.Frame();
      scroller.SetScrollTop(100);
      fixture.Frame();
      fixture.SetStyle(u8"st", u8"height:0");
      fixture.Frame();
      CHECK(scroller.ScrollTop() == 100);
    }
  }
}
