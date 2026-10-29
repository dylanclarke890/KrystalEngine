#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Layout/LayoutTestUtils.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/DOMRect.hpp"
#include "Krystal.Booey/DOM/DOMRectList.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/Range.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/HTML/HTMLElement.hpp"

namespace krys::boo::dom::tests
{
  using style::tests::ResolverFixture;

  namespace
  {
    // A wrapped inline, a box with a border and padding, and a scroller (without scrollbars) whose content is
    // larger than it is.
    constexpr auto Style = u8R"(
      #w { width: 30px }
      #box { margin: 10px 0 0 20px; width: 50px; height: 30px; border: 1px solid; padding: 4px }
      #s { width: 100px; height: 50px; overflow: hidden; border: 5px solid; padding: 2px }
      #content { display: flow-root; width: 300px; height: 200px }
      #target { margin: 150px 0 0 120px; width: 20px; height: 10px }
    )";

    constexpr auto Body = u8R"(<div id=w><span id=inl>aa bb</span></div><div id=box></div>)"
                          u8R"(<div id=s><div id=content><div id=target tabindex=0></div></div></div>)";

    KRYS_NODISCARD bool RectIs(const DOMRectReadOnly &rect, double x, double y, double width, double height)
    {
      return rect.X() == x && rect.Y() == y && rect.Width() == width && rect.Height() == height;
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "Element: CSSOM View geometry", "[DOM][Element][CSSOMView]")
  {
    ResolverFixture fixture {layout::tests::AhemDocument(Style, Body)};
    auto &box = fixture.ById(u8"box");

    SECTION("Client rects")
    {
      // The box's border box. The layout is brought up to date first.
      CHECK(RectIs(*box.GetBoundingClientRect(), 20, 30, 60, 40));
      auto boxRects = box.GetClientRects();
      REQUIRE(boxRects->Length() == 1u);
      CHECK(RectIs(*boxRects->Item(0), 20, 30, 60, 40));
      CHECK(boxRects->Item(1) == nullptr);

      // One rect per line of an inline; the bounding rect is their union.
      auto &inl = fixture.ById(u8"inl");
      auto inlineRects = inl.GetClientRects();
      REQUIRE(inlineRects->Length() == 2u);
      CHECK(RectIs(*inlineRects->Item(0), 0, 0, 20, 10));
      CHECK(RectIs(*inlineRects->Item(1), 0, 10, 20, 10));
      CHECK(RectIs(*inl.GetBoundingClientRect(), 0, 0, 20, 20));

      // A scrolled box's descendants move up and left by the scroll offset.
      auto &target = fixture.ById(u8"target");
      CHECK(RectIs(*target.GetBoundingClientRect(), 127, 227, 20, 10));
      fixture.ById(u8"s").ScrollTo(10, 20);
      CHECK(RectIs(*target.GetBoundingClientRect(), 117, 207, 20, 10));
    }

    SECTION("A range's client rects")
    {
      auto &document = box.NodeDocument();
      auto &text = *fixture.ById(u8"inl").FirstChild();

      // The text a range covers, a rect per box.
      auto range = document.CreateRange();
      REQUIRE_FALSE(range->SetStart(text, 0).HasException());
      REQUIRE_FALSE(range->SetEnd(text, 2).HasException());
      auto rects = range->GetClientRects();
      REQUIRE(rects->Length() == 1u);
      CHECK(RectIs(*rects->Item(0), 0, 0, 20, 10));
      CHECK(RectIs(*range->GetBoundingClientRect(), 0, 0, 20, 10));

      // An element it selects wholly gives its border box, and its contents nothing more.
      REQUIRE_FALSE(range->SelectNode(box).HasException());
      rects = range->GetClientRects();
      REQUIRE(rects->Length() == 1u);
      CHECK(RectIs(*rects->Item(0), 20, 30, 60, 40));

      // Text across two lines: a rect on each, and the bounding rect around both.
      REQUIRE_FALSE(range->SetStart(text, 0).HasException());
      REQUIRE_FALSE(range->SetEnd(text, 5).HasException());
      rects = range->GetClientRects();
      REQUIRE(rects->Length() == 2u);
      CHECK(rects->Item(1)->Y() == 10);
      CHECK(RectIs(*rects->Item(1), 0, 10, 20, 10));
      CHECK(range->GetBoundingClientRect()->Height() == 20);
    }

    SECTION("Elements without boxes")
    {
      REQUIRE_FALSE(box.SetAttribute(u8"style", u8"display: none").HasException());
      CHECK(box.GetClientRects()->Length() == 0u);
      CHECK(RectIs(*box.GetBoundingClientRect(), 0, 0, 0, 0));
      CHECK(box.OffsetParent() == nullptr);
      CHECK(box.OffsetWidth() == 0);
      CHECK(box.ClientWidth() == 0);
      CHECK(box.ScrollWidth() == 0);
    }

    SECTION("Offset, client and scroll sizes")
    {
      // The offset parent is the body: the offsets are from the initial containing block.
      CHECK(box.OffsetParent() == fixture.Document->Body().get());
      CHECK(box.OffsetLeft() == 20);
      CHECK(box.OffsetTop() == 30);
      CHECK(box.OffsetWidth() == 60);
      CHECK(box.OffsetHeight() == 40);

      // The padding box, inside the border.
      CHECK(box.ClientLeft() == 1);
      CHECK(box.ClientTop() == 1);
      CHECK(box.ClientWidth() == 58);
      CHECK(box.ClientHeight() == 38);
      CHECK(box.ScrollWidth() == 58);
      CHECK(box.ScrollHeight() == 38);

      // The scroller's scrollable overflow includes its padding at the block end, but not at the inline end.
      auto &s = fixture.ById(u8"s");
      CHECK(s.ClientWidth() == 104);
      CHECK(s.ClientHeight() == 54);
      CHECK(s.ScrollWidth() == 302);
      CHECK(s.ScrollHeight() == 204);

      // Offsets are relative to a positioned offset parent's padding edge.
      REQUIRE_FALSE(s.SetAttribute(u8"style", u8"position: relative").HasException());
      auto &target = fixture.ById(u8"target");
      CHECK(target.OffsetParent() == &s);
      CHECK(target.OffsetLeft() == 122);
      CHECK(target.OffsetTop() == 152);

      // The root element's client size is the viewport's.
      auto &root = *fixture.Document->DocumentElement();
      CHECK(root.ClientWidth() == 800);
      CHECK(root.ClientHeight() == 600);
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "Element: CSSOM View scrolling", "[DOM][Element][CSSOMView]")
  {
    ResolverFixture fixture {layout::tests::AhemDocument(Style, Body)};
    auto &s = fixture.ById(u8"s");
    auto &target = fixture.ById(u8"target");

    SECTION("Scroll positions")
    {
      CHECK(s.ScrollTop() == 0);
      CHECK(s.ScrollLeft() == 0);

      s.SetScrollTop(30);
      s.SetScrollLeft(40);
      CHECK(s.ScrollTop() == 30);
      CHECK(s.ScrollLeft() == 40);

      // Scroll positions are clamped to the scrollable overflow.
      s.SetScrollTop(1'000);
      s.SetScrollLeft(-5);
      CHECK(s.ScrollTop() == 150);
      CHECK(s.ScrollLeft() == 0);

      s.ScrollTo(10, 20);
      CHECK(s.ScrollLeft() == 10);
      CHECK(s.ScrollTop() == 20);

      s.ScrollBy(5, 5);
      CHECK(s.ScrollLeft() == 15);
      CHECK(s.ScrollTop() == 25);

      // A missing coordinate keeps the current position, and a non-finite one is 0.
      ScrollToOptions options;
      options.Top = 7;
      s.Scroll(options);
      CHECK(s.ScrollLeft() == 15);
      CHECK(s.ScrollTop() == 7);
      options.Left = std::numeric_limits<double>::infinity();
      s.ScrollTo(options);
      CHECK(s.ScrollLeft() == 0);

      // Boxes without overflow don't scroll.
      auto &box = fixture.ById(u8"box");
      box.SetScrollTop(10);
      box.ScrollTo(10, 10);
      CHECK(box.ScrollTop() == 0);
      CHECK(box.ScrollLeft() == 0);

      // Neither does the root element, while the viewport can't scroll.
      auto &root = *fixture.Document->DocumentElement();
      root.SetScrollTop(10);
      CHECK(root.ScrollTop() == 0);
    }

    SECTION("Scrolling into view")
    {
      // The start of the block axis, and the nearest edge of the inline axis.
      target.ScrollIntoView();
      CHECK(s.ScrollTop() == 150);
      CHECK(s.ScrollLeft() == 38);

      target.ScrollIntoView(false);
      CHECK(s.ScrollTop() == 108);

      ScrollIntoViewOptions options;
      options.BlockPosition = ScrollLogicalPosition::Center;
      options.InlinePosition = ScrollLogicalPosition::Start;
      target.ScrollIntoView(options);
      CHECK(s.ScrollTop() == 130);
      CHECK(s.ScrollLeft() == 122);

      // Nearest doesn't scroll an axis that's already showing the element.
      options.BlockPosition = ScrollLogicalPosition::Nearest;
      options.InlinePosition = ScrollLogicalPosition::Nearest;
      target.ScrollIntoView(options);
      CHECK(s.ScrollTop() == 130);
      CHECK(s.ScrollLeft() == 122);

      // The scroll margin is kept visible.
      REQUIRE_FALSE(target.SetAttribute(u8"style", u8"scroll-margin-top: 5px").HasException());
      target.ScrollIntoView();
      CHECK(s.ScrollTop() == 147);
    }

    SECTION("Focusing")
    {
      auto &html = Downcast<html::HTMLElement>(target);
      FocusOptions options;
      options.PreventScroll = true;
      html.Focus(options);
      CHECK(fixture.Document->FocusedElement() == &target);
      html::EventLoop::Current().RunUntilIdle();
      CHECK(s.ScrollTop() == 0);
      CHECK(s.ScrollLeft() == 0);

      // Focusing centres a hidden element, on a zero-delay timer.
      html.Blur();
      html.Focus();
      CHECK(s.ScrollTop() == 0);
      html::EventLoop::Current().RunUntilIdle();
      CHECK(s.ScrollTop() == 130);
      CHECK(s.ScrollLeft() == 80);
    }
  }
  TEST_CASE_METHOD(css::tests::ValueFixture, "Element: CSSOM View scrolling right to left",
                   "[DOM][Element][CSSOMView]")
  {
    // The values are Chrome's for the same page.
    ResolverFixture fixture {
      u8"<!DOCTYPE html><style>body { margin: 0 }"
      u8"#s { direction: rtl; width: 100px; height: 40px; overflow: auto; scrollbar-width: none;"
      u8" white-space: nowrap }"
      u8"#s span { display: inline-block; width: 60px; height: 20px }"
      u8"#v { direction: rtl; width: 100px; height: 40px; overflow: scroll }"
      u8"#v div { width: 300px; height: 100px }</style>"
      u8"<div id=s><span id=a></span><span id=b></span><span id=c></span><span id=d></span>"
      u8"<span id=e></span></div><div id=v><div id=vc></div></div>"};
    auto &s = fixture.ById(u8"s");
    auto x = [&](dom::DOMStringView id)
    {
      return fixture.ById(id).GetBoundingClientRect()->X();
    };

    // A right-to-left scroller starts scrolled to its right edge, which is 0; scrolling left is negative.
    CHECK(s.ScrollLeft() == 0);
    CHECK(s.ScrollWidth() == 300);
    CHECK(s.ClientWidth() == 100);
    CHECK(x(u8"a") == 40);
    CHECK(x(u8"e") == -200);

    s.SetScrollLeft(-100);
    CHECK(s.ScrollLeft() == -100);
    CHECK(x(u8"a") == 140);
    s.SetScrollLeft(50);
    CHECK(s.ScrollLeft() == 0);
    s.SetScrollLeft(-1'000);
    CHECK(s.ScrollLeft() == -200);
    CHECK(x(u8"e") == 0);

    // The inline start of a right-to-left box is its right.
    s.ScrollTo(0, 0);
    ScrollIntoViewOptions options;
    options.InlinePosition = ScrollLogicalPosition::Start;
    fixture.ById(u8"c").ScrollIntoView(options);
    CHECK(s.ScrollLeft() == -120);
    CHECK(x(u8"c") == 40);
    options.InlinePosition = ScrollLogicalPosition::End;
    fixture.ById(u8"c").ScrollIntoView(options);
    CHECK(s.ScrollLeft() == -80);
    CHECK(x(u8"c") == 0);

    // Its vertical scrollbar is on the left, and its content starts on the right.
    auto &v = fixture.ById(u8"v");
    CHECK(v.ClientLeft() > 0);
    CHECK(v.ClientLeft() + v.ClientWidth() == 100);
    CHECK(v.ScrollLeft() == 0);
    auto content = fixture.ById(u8"vc").GetBoundingClientRect();
    CHECK(content->X() + content->Width() == 100);
  }
}
