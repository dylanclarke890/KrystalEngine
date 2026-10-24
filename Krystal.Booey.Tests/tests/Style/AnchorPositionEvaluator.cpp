#include "Krystal.Booey/Style/AnchorPositionEvaluator.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/PseudoElement.hpp"
#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Page/LocalFrameViewLayoutContext.hpp"
#include "Krystal.Booey/Rendering/RenderBox.hpp"
#include "Krystal.Booey/Rendering/RenderLayer.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/Style/Scope.hpp"
#include "Krystal.Booey/Style/Styleable.hpp"

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

  TEST_CASE_METHOD(css::tests::ValueFixture, "AnchorPositionEvaluator: position options",
                   "[Style][AnchorPositionEvaluator]")
  {
    ResolverFixture fixture {u8R"html(<!DOCTYPE html><style>
      body { margin: 0 }
      #anchor { position: absolute; left: 700px; top: 50px; width: 40px; height: 20px; anchor-name: --a }
      #target { position: absolute; position-anchor: --a; left: anchor(right); top: anchor(top);
                width: 200px; height: 10px }
      @position-try --left { left: auto; right: anchor(left) }
      @position-try --below { left: anchor(left); top: anchor(bottom) }
      #host.on::before { content: ''; position: absolute; position-anchor: --a; left: anchor(right); top: 0;
                         width: 200px; height: 5px; position-try-fallbacks: --left }
    </style><div id=anchor></div><div id=target></div><div id=host></div>)html"};
    auto &anchor = fixture.ById(u8"anchor");
    auto &target = fixture.ById(u8"target");
    auto usedIndex = [&]
    {
      return target.Renderer()->Style().UsedPositionOptionIndex();
    };

    SECTION("The first option that doesn't overflow is used")
    {
      SetStyle(target, u8"position-try-fallbacks: --below, --left");
      CHECK(FrameRectOf(target) == LayoutRect(500, 50, 200, 10));
      CHECK(usedIndex() == 1);

      auto *targetBox = DynamicDowncast<RenderBox>(target.Renderer());
      REQUIRE(targetBox);
      CHECK(fixture.Document->GetRenderView()->PositionTryBoxes().contains(targetBox));
    }

    SECTION("The original style is used while it fits, or when no option does")
    {
      SetStyle(target, u8"position-try-fallbacks: --left");
      SetStyle(anchor, u8"left: 100px");
      CHECK(FrameRectOf(target).X() == 140);
      CHECK_FALSE(usedIndex());

      SetStyle(target, u8"position-try-fallbacks: --left; width: 900px");
      CHECK(FrameRectOf(target).X() == 140);
      CHECK_FALSE(usedIndex());
    }

    SECTION("Try tactics flip the original style")
    {
      SetStyle(target, u8"position-try-fallbacks: flip-inline");
      CHECK(FrameRectOf(target).X() == 500);
      CHECK(usedIndex() == 0);
    }

    SECTION("position-try-order sorts the options by their inset-modified containing block")
    {
      SetStyle(anchor, u8"left: 300px");
      SetStyle(target, u8"left: 750px; width: 100px; position-try-fallbacks: --left, --below");
      CHECK(FrameRectOf(target).X() == 200);
      SetStyle(target, u8"left: 750px; width: 100px; position-try-fallbacks: --left, --below; "
                       u8"position-try-order: most-width");
      CHECK(FrameRectOf(target).Location() == LayoutPoint(300, 70));
    }

    SECTION("The last successful option is tried first, until the fallbacks change")
    {
      SetStyle(target, u8"position-try-fallbacks: --left");
      CHECK(FrameRectOf(target).X() == 500);
      fixture.Document->UpdateResizeObservations();
      CHECK(fixture.Document->StyleScope().LastSuccessfulPositionOptionIndexFor(Styleable {target, null})
            == 0);

      // Both styles fit now, but the last successful option stays.
      SetStyle(anchor, u8"left: 400px");
      CHECK(FrameRectOf(target).X() == 200);

      SetStyle(target, u8"position-try-fallbacks: --left, --below");
      CHECK(FrameRectOf(target).X() == 440);
      CHECK_FALSE(
        fixture.Document->StyleScope().LastSuccessfulPositionOptionIndexFor(Styleable {target, null}));
    }

    SECTION("A pseudo-element's position options")
    {
      auto &host = fixture.ById(u8"host");
      REQUIRE_FALSE(host.SetAttribute(u8"class", u8"on").HasException());
      (void)fixture.Document->UpdateLayout();
      auto *before = host.BeforePseudoElement();
      REQUIRE(before);
      auto *beforeBox = DynamicDowncast<RenderBox>(before->Renderer());
      REQUIRE(beforeBox);
      CHECK(beforeBox->Style().UsedPositionOptionIndex() == 0);
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "AnchorPositionEvaluator: position-visibility",
                   "[Style][AnchorPositionEvaluator]")
  {
    ResolverFixture fixture {u8R"html(<!DOCTYPE html><style>
      body { margin: 0 }
      #anchor { position: absolute; left: 100px; top: 50px; width: 40px; height: 20px; anchor-name: --a }
      #target { position: absolute; position-anchor: --a; left: anchor(right); top: anchor(top);
                width: 20px; height: 10px }
    </style><div id=anchor></div><div id=target></div>)html"};
    auto &anchor = fixture.ById(u8"anchor");
    auto &target = fixture.ById(u8"target");
    auto isHidden = [&]
    {
      (void)fixture.Document->UpdateLayout();
      REQUIRE(target.Renderer());
      return target.Renderer()->Style().IsForceHidden();
    };

    SECTION("anchors-visible, the initial value, hides the box with its anchor")
    {
      CHECK_FALSE(isHidden());
      SetStyle(anchor, u8"visibility: hidden");
      CHECK(isHidden());
      SetStyle(target, u8"position-visibility: always");
      CHECK_FALSE(isHidden());
    }

    SECTION("no-overflow hides an overflowing box")
    {
      SetStyle(target, u8"position-visibility: no-overflow; width: 900px");
      CHECK(isHidden());
      SetStyle(target, u8"position-visibility: no-overflow");
      CHECK_FALSE(isHidden());
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "AnchorPositionEvaluator: scroll adjustment",
                   "[Style][AnchorPositionEvaluator]")
  {
    ResolverFixture fixture {u8R"html(<!DOCTYPE html><style>
      body { margin: 0; height: 2000px }
      #scroller { position: absolute; left: 0; top: 0; width: 300px; height: 200px; overflow: scroll }
      #content { height: 1000px }
      #anchor { position: absolute; left: 50px; top: 100px; width: 40px; height: 20px; anchor-name: --a }
      #target { position: absolute; position-anchor: --a; left: anchor(right); top: anchor(bottom);
                width: 20px; height: 10px }
      @position-try --below { bottom: auto; top: anchor(bottom) }
    </style><div id=scroller><div id=content><div id=anchor></div></div></div><div id=target></div>)html"};
    auto &scroller = fixture.ById(u8"scroller");
    auto &target = fixture.ById(u8"target");
    auto adjustment = [&]
    {
      auto *box = DynamicDowncast<RenderBox>(target.Renderer());
      REQUIRE(box);
      REQUIRE(box->Layer());
      return box->Layer()->AnchorScrollAdjustment();
    };
    auto &layoutContext = fixture.Frame->View()->LayoutContext();

    SECTION("The box is shifted by its anchor's scrolling, without a layout")
    {
      CHECK(FrameRectOf(target) == LayoutRect(90, 120, 20, 10));
      REQUIRE(adjustment());
      CHECK(*adjustment() == LayoutSize());
      auto *targetBox = DynamicDowncast<RenderBox>(target.Renderer());
      CHECK(layoutContext.AnchorScrollAdjusterFor(*targetBox));
      CHECK(targetBox->Layer()->IsTransformed());

      scroller.SetScrollTop(30);
      CHECK(*adjustment() == LayoutSize(0, -30));
      CHECK(targetBox->FrameRect() == LayoutRect(90, 120, 20, 10));
    }

    SECTION("Only the axes with anchor() are compensated")
    {
      SetStyle(target, u8"left: 10px");
      CHECK(FrameRectOf(target).X() == 10);
      SetStyle(scroller, u8"overflow: scroll; width: 100px");
      SetStyle(fixture.ById(u8"content"), u8"width: 1000px");
      (void)fixture.Document->UpdateLayout();
      scroller.ScrollTo(20, 30);
      REQUIRE(adjustment());
      CHECK(*adjustment() == LayoutSize(0, -30));
    }

    SECTION("A box without anchor() in a scrolled axis isn't adjusted")
    {
      SetStyle(target, u8"left: 10px; top: 10px");
      (void)FrameRectOf(target);
      CHECK_FALSE(adjustment());
      auto *targetBox = DynamicDowncast<RenderBox>(target.Renderer());
      CHECK_FALSE(layoutContext.AnchorScrollAdjusterFor(*targetBox));
    }

    SECTION("Scrolling past the fallback limits chooses another option")
    {
      SetStyle(target, u8"top: auto; bottom: anchor(top); height: 90px; position-try-fallbacks: --below");
      CHECK(FrameRectOf(target).Y() == 10);
      CHECK_FALSE(target.Renderer()->Style().UsedPositionOptionIndex());

      scroller.SetScrollTop(30);
      CHECK(FrameRectOf(target).Y() == 90);
      CHECK(target.Renderer()->Style().UsedPositionOptionIndex() == 0);
      REQUIRE(adjustment());
      CHECK(*adjustment() == LayoutSize());
    }

    SECTION("The adjuster goes with the box")
    {
      (void)FrameRectOf(target);
      CHECK(layoutContext.AnchorScrollAdjusters().size() == 1);
      REQUIRE_FALSE(target.Remove().HasException());
      (void)fixture.Document->UpdateLayout();
      CHECK(layoutContext.AnchorScrollAdjusters().empty());
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "AnchorPositionEvaluator: scroll adjustment of fixed boxes",
                   "[Style][AnchorPositionEvaluator]")
  {
    ResolverFixture fixture {u8R"html(<!DOCTYPE html><style>
      body { margin: 0; height: 2000px }
      #anchor { position: absolute; left: 50px; top: 100px; width: 40px; height: 20px; anchor-name: --a }
      #target { position: fixed; position-anchor: --a; left: anchor(right); top: anchor(bottom);
                width: 20px; height: 10px }
    </style><div id=anchor></div><div id=target></div>)html"};
    auto &target = fixture.ById(u8"target");
    CHECK(FrameRectOf(target) == LayoutRect(90, 120, 20, 10));

    auto *targetBox = DynamicDowncast<RenderBox>(target.Renderer());
    REQUIRE(targetBox);
    auto *adjuster = fixture.Frame->View()->LayoutContext().AnchorScrollAdjusterFor(*targetBox);
    REQUIRE(adjuster);
    CHECK(adjuster->HasViewportSnapshot());

    fixture.Frame->View()->SetScrollPosition({0, 40});
    REQUIRE(targetBox->Layer()->AnchorScrollAdjustment());
    CHECK(*targetBox->Layer()->AnchorScrollAdjustment() == LayoutSize(0, -40));
  }
}
