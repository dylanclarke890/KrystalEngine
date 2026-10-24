#include "Krystal.Booey/Style/AnchorPositionEvaluator.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/PseudoElement.hpp"
#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "Krystal.Booey/Rendering/RenderBox.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/Style/Scope.hpp"

namespace krys::boo::style::tests
{
  namespace
  {
    KRYS_NODISCARD LayoutRect FrameRectOf(dom::Element &element)
    {
      (void)element.NodeDocument().UpdateLayout();
      auto *box = DynamicDowncast<RenderBox>(element.Renderer());
      REQUIRE(box);
      return box->FrameRect();
    }

    void SetStyle(dom::Element &element, dom::DOMStringView style)
    {
      REQUIRE_FALSE(element.SetAttribute(u8"style", dom::DOMString(style)).HasException());
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "AnchorPositionEvaluator: anchor() and anchor-size()",
                   "[Style][AnchorPositionEvaluator]")
  {
    ResolverFixture fixture {u8R"html(<!DOCTYPE html><style>
      body { margin: 0 }
      #anchor { position: absolute; left: 100px; top: 50px; width: 40px; height: 20px; anchor-name: --a }
      #target { position: absolute; position-anchor: --a; height: 10px }
    </style><div id=anchor></div><div id=target></div>)html"};
    auto &anchor = fixture.ById(u8"anchor");
    auto &target = fixture.ById(u8"target");

    SECTION("Sides of the default anchor, and its size")
    {
      SetStyle(target, u8"left: anchor(right); top: anchor(bottom); width: anchor-size(width)");
      CHECK(FrameRectOf(target) == LayoutRect(140, 70, 40, 10));

      auto *anchorBox = DynamicDowncast<RenderBox>(anchor.Renderer());
      auto *targetBox = DynamicDowncast<RenderBox>(target.Renderer());
      REQUIRE(anchorBox);
      REQUIRE(targetBox);
      CHECK(AnchorPositionEvaluator::DefaultAnchorForBox(*targetBox) == anchorBox);
      CHECK(targetBox->Style().UsesAnchorFunctions());
      CHECK(AnchorPositionEvaluator::IsAnchor(anchorBox->Style()));
      CHECK(fixture.Document->GetRenderView()->Anchors().contains(anchorBox));
    }

    SECTION("A named anchor, the opposite insets, logical sides, centers and percentages")
    {
      SetStyle(target, u8"right: anchor(--a left); bottom: anchor(--a top); width: anchor-size(--a height)");
      CHECK(FrameRectOf(target) == LayoutRect(80, 40, 20, 10));

      SetStyle(target, u8"inset-inline-start: anchor(end); inset-block-start: anchor(center); width: 5px");
      CHECK(FrameRectOf(target) == LayoutRect(140, 60, 5, 10));

      SetStyle(target, u8"left: anchor(25%); top: anchor(start); width: anchor-size(self-inline)");
      CHECK(FrameRectOf(target) == LayoutRect(110, 50, 40, 10));
    }

    SECTION("The target follows its anchor")
    {
      SetStyle(target, u8"left: anchor(right); top: anchor(bottom); width: 5px");
      CHECK(FrameRectOf(target).Location() == LayoutPoint(140, 70));
      SetStyle(anchor, u8"left: 300px; width: 10px");
      CHECK(FrameRectOf(target).Location() == LayoutPoint(310, 70));
    }

    SECTION("Without an anchor, the fallback is used, or the property is invalid")
    {
      SetStyle(target, u8"left: anchor(--missing right, 7px); top: anchor(--missing top); "
                       u8"width: anchor-size(--missing width, 3px)");
      CHECK(FrameRectOf(target) == LayoutRect(7, 0, 3, 10));
    }

    SECTION("A physical side in the other axis is invalid")
    {
      SetStyle(target, u8"left: anchor(top, 9px); top: anchor(left, 4px); width: 5px");
      CHECK(FrameRectOf(target) == LayoutRect(9, 4, 5, 10));
    }

    SECTION("anchor() only works on absolutely positioned boxes")
    {
      SetStyle(target, u8"position: relative; left: anchor(right, 6px); width: 5px");
      (void)FrameRectOf(target);
      CHECK(Downcast<RenderBox>(*target.Renderer()).OffsetLeft() == 6);
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "AnchorPositionEvaluator: acceptable anchors and anchor-scope",
                   "[Style][AnchorPositionEvaluator]")
  {
    SECTION("The last acceptable anchor with the name wins; one laid out after the target isn't acceptable")
    {
      ResolverFixture fixture {u8R"html(<!DOCTYPE html><style>
        body { margin: 0 }
        .anchor { position: absolute; width: 10px; height: 10px; anchor-name: --a }
        #target { position: absolute; left: anchor(--a left, 1px); top: 0; width: 5px; height: 5px }
      </style><div class=anchor style="left: 20px"></div><div class=anchor style="left: 40px"></div>
      <div id=target></div><div class=anchor style="left: 60px"></div>)html"};
      CHECK(FrameRectOf(fixture.ById(u8"target")).X() == 40);
    }

    SECTION("An in-flow anchor after the target is acceptable")
    {
      ResolverFixture fixture {u8R"html(<!DOCTYPE html><style>
        body { margin: 0 }
        #target { position: absolute; left: anchor(--a right); top: 0; width: 5px; height: 5px }
        #anchor { margin-left: 30px; width: 10px; height: 10px; anchor-name: --a }
      </style><div id=target></div><div id=anchor></div>)html"};
      CHECK(FrameRectOf(fixture.ById(u8"target")).X() == 40);
    }

    SECTION("An anchor-positioned box can be an anchor itself")
    {
      ResolverFixture fixture {u8R"html(<!DOCTYPE html><style>
        body { margin: 0 }
        div { position: absolute; top: 0; width: 10px; height: 10px }
        #a { left: 10px; anchor-name: --a }
        #b { position-anchor: --a; left: anchor(right); anchor-name: --b }
        #c { left: anchor(--b right) }
      </style><div id=a></div><div id=b></div><div id=c></div>)html"};
      CHECK(FrameRectOf(fixture.ById(u8"b")).X() == 20);
      CHECK(FrameRectOf(fixture.ById(u8"c")).X() == 30);
    }

    SECTION("A pseudo-element's implicit anchor is its originating element")
    {
      ResolverFixture fixture {u8R"html(<!DOCTYPE html><style>
        body { margin: 0 }
        #host { width: 50px; height: 20px }
        #host::before { content: ''; position: absolute; left: anchor(right); top: anchor(bottom); width: 5px;
                        height: 5px }
      </style><div id=host></div>)html"};
      auto &host = fixture.ById(u8"host");
      (void)FrameRectOf(host);
      auto *before = host.BeforePseudoElement();
      REQUIRE(before);
      CHECK(FrameRectOf(*before).Location() == LayoutPoint(50, 20));
    }

    SECTION("anchor-scope limits a name to a subtree")
    {
      ResolverFixture fixture {u8R"html(<!DOCTYPE html><style>
        body { margin: 0 }
        .scope { anchor-scope: --a }
        .anchor { position: absolute; top: 0; width: 10px; height: 10px; anchor-name: --a }
        .target { position: absolute; left: anchor(--a left, 1px); top: 20px; width: 5px; height: 5px }
      </style><div class=scope><div class=anchor style="left: 20px"></div><div id=first class=target></div></div>
      <div class=scope><div class=anchor style="left: 50px"></div><div id=second class=target></div></div>
      <div id=outside class=target></div>)html"};
      CHECK(FrameRectOf(fixture.ById(u8"first")).X() == 20);
      CHECK(FrameRectOf(fixture.ById(u8"second")).X() == 50);
      CHECK(FrameRectOf(fixture.ById(u8"outside")).X() == 1);
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture,
                   "AnchorPositionEvaluator: position-area, anchor-center and implicit anchors",
                   "[Style][AnchorPositionEvaluator]")
  {
    ResolverFixture fixture {u8R"html(<!DOCTYPE html><style>
      body { margin: 0 }
      #anchor { position: absolute; left: 100px; top: 50px; width: 40px; height: 20px; anchor-name: --a }
      #target { position: absolute; position-anchor: --a; width: 10px; height: 10px }
      #invoker { margin-left: 200px; width: 60px; height: 30px }
      [popover] { margin: 0; padding: 0; border: 0; inset: auto; width: 10px; height: 10px }
    </style><div id=anchor></div><div id=target></div><div id=invoker></div><div id=menu popover></div>)html"};
    auto &target = fixture.ById(u8"target");

    SECTION("position-area places the box in a region of the anchor's grid")
    {
      SetStyle(target, u8"position-area: bottom right");
      CHECK(FrameRectOf(target).Location() == LayoutPoint(140, 70));
      SetStyle(target, u8"position-area: top left");
      CHECK(FrameRectOf(target).Location() == LayoutPoint(90, 40));
      // Spanning the anchor's column centres the box over it.
      SetStyle(target, u8"position-area: bottom");
      CHECK(FrameRectOf(target).Location() == LayoutPoint(115, 70));
    }

    SECTION("anchor-center centres the box on its anchor")
    {
      SetStyle(target, u8"justify-self: anchor-center; top: 0");
      CHECK(FrameRectOf(target).X() == 115);
    }

    SECTION("A popover's invoker is its implicit anchor")
    {
      auto &menu = Downcast<html::HTMLElement>(fixture.ById(u8"menu"));
      auto &invoker = Downcast<html::HTMLElement>(fixture.ById(u8"invoker"));
      SetStyle(menu, u8"left: anchor(left); top: anchor(bottom)");
      REQUIRE_FALSE(menu.ShowPopover({.Source = ShareRefPtr(&invoker)}).HasException());
      CHECK(FrameRectOf(menu).Location() == LayoutPoint(200, 30));

      auto *invokerBox = DynamicDowncast<RenderBox>(invoker.Renderer());
      REQUIRE(invokerBox);
      CHECK(AnchorPositionEvaluator::IsImplicitAnchor(invokerBox->Style()));

      // Without an invoker, there's no implicit anchor, so anchor() is invalid.
      REQUIRE_FALSE(menu.HidePopover().HasException());
      SetStyle(menu, u8"left: anchor(left, 3px); top: anchor(bottom, 4px)");
      REQUIRE_FALSE(menu.ShowPopover().HasException());
      CHECK(FrameRectOf(menu).Location() == LayoutPoint(3, 4));
    }
  }
}
