#include "Krystal.Booey/Rendering/Layers/RenderLayerScrollableArea.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Rendering/PaintTestUtils.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/HTML/EventLoop/EventLoop.hpp"
#include "Krystal.Booey/Page/EventHandler.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Platform/Events/PlatformMouseEvent.hpp"
#include "Krystal.Booey/Platform/Scrolling/ScrollbarsController.hpp"
#include "Krystal.Booey/Platform/Scrolling/ScrollbarTheme.hpp"
#include "Krystal.Booey/Platform/Scrolling/ScrollbarThemeAdwaita.hpp"
#include "Krystal.Booey/Rendering/RenderBox.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Core/Utils/ScopeExit.hpp"
#include <chrono>

namespace krys::boo::tests
{
  using style::tests::ResolverFixture;

  namespace
  {
    KRYS_NODISCARD RenderBox &BoxOf(ResolverFixture &fixture, dom::DOMStringView id)
    {
      return Downcast<RenderBox>(*fixture.ById(id).Renderer());
    }

    KRYS_NODISCARD RenderLayerScrollableArea *ScrollableAreaOf(ResolverFixture &fixture,
                                                               dom::DOMStringView id)
    {
      auto *layer = BoxOf(fixture, id).Layer();
      return layer ? layer->GetScrollableArea() : nullptr;
    }
  }

  TEST_CASE("RenderLayerScrollableArea: the Adwaita scrollbar theme",
            "[Rendering][RenderLayerScrollableArea]")
  {
    auto &theme = ScrollbarTheme::Theme();
    CHECK_FALSE(theme.UsesOverlayScrollbars());
    // Classic Adwaita scrollbars are 21 pixels thick, or 13 thin.
    CHECK(theme.ScrollbarThickness() == 21);
    CHECK(theme.ScrollbarThickness(ScrollbarWidth::Thin) == 13);
    CHECK(theme.ScrollbarThickness(ScrollbarWidth::None) == 0);
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "RenderLayerScrollableArea: scrollbars",
                   "[Rendering][RenderLayerScrollableArea]")
  {
    ResolverFixture fixture {u8R"html(<!DOCTYPE html><style>
      #scroll { overflow: scroll }
      #scrollx { overflow-x: scroll; overflow-y: hidden }
      #auto { overflow: auto }
      #hidden { overflow: hidden }
      #gutter { overflow: hidden; scrollbar-gutter: stable }
      #thin { overflow: scroll; scrollbar-width: thin }
      #none { overflow: scroll; scrollbar-width: none }
      #rel { position: relative }
    </style><body><div id=scroll><div id=child style="position: relative"></div></div><div id=scrollx></div>
    <div id=auto></div><div id=hidden></div><div id=gutter></div><div id=thin></div><div id=none></div>
    <div id=rel></div></body>)html"};
    auto &document = *fixture.Document;
    document.UpdateStyleIfNeeded();
    (void)document.UpdateLayout();

    SECTION("Which layers have a scrollable area")
    {
      CHECK(document.GetRenderView()->Layer()->GetScrollableArea());
      CHECK(Downcast<RenderLayerModelObject>(*document.DocumentElement()->Renderer())
              .Layer()
              ->GetScrollableArea());
      for (auto id : {u8"scroll", u8"scrollx", u8"auto", u8"hidden", u8"gutter", u8"thin", u8"none"})
      {
        INFO(css::tests::ToNarrow(id));
        CHECK(ScrollableAreaOf(fixture, id));
      }
      REQUIRE(BoxOf(fixture, u8"rel").Layer());
      CHECK_FALSE(ScrollableAreaOf(fixture, u8"rel"));
    }

    SECTION("overflow: scroll has both scrollbars, disabled without overflow")
    {
      auto &scrollableArea = *ScrollableAreaOf(fixture, u8"scroll");
      REQUIRE(scrollableArea.HorizontalScrollbar());
      REQUIRE(scrollableArea.VerticalScrollbar());
      CHECK_FALSE(scrollableArea.HorizontalScrollbar()->Enabled());
      CHECK_FALSE(scrollableArea.VerticalScrollbar()->Enabled());
      CHECK(scrollableArea.VerticalScrollbar()->Orientation() == ScrollbarOrientation::Vertical);
      CHECK_FALSE(scrollableArea.HasOverlayScrollbars());
      CHECK(scrollableArea.HasOverflowControls());

      auto &box = BoxOf(fixture, u8"scroll");
      CHECK(box.VerticalScrollbarWidth() == 21);
      CHECK(box.HorizontalScrollbarHeight() == 21);
      CHECK(box.HasAlwaysPresentScrollbar(ScrollbarOrientation::Vertical));
      CHECK_FALSE(box.HasAutoScrollbar(ScrollbarOrientation::Vertical));
      // The corner between the two scrollbars.
      CHECK(scrollableArea.IsScrollCornerVisible());
    }

    SECTION("Each axis has its own scrollbar")
    {
      auto &scrollableArea = *ScrollableAreaOf(fixture, u8"scrollx");
      CHECK(scrollableArea.HorizontalScrollbar());
      CHECK_FALSE(scrollableArea.VerticalScrollbar());
      CHECK(BoxOf(fixture, u8"scrollx").VerticalScrollbarWidth() == 0);
      CHECK(BoxOf(fixture, u8"scrollx").HorizontalScrollbarHeight() == 21);
      CHECK_FALSE(scrollableArea.IsScrollCornerVisible());
    }

    SECTION("overflow: auto and hidden have no scrollbars without overflow")
    {
      CHECK_FALSE(ScrollableAreaOf(fixture, u8"auto")->HasScrollbars());
      CHECK(BoxOf(fixture, u8"auto").HasAutoScrollbar(ScrollbarOrientation::Horizontal));
      CHECK_FALSE(ScrollableAreaOf(fixture, u8"hidden")->HasScrollbars());
      CHECK(BoxOf(fixture, u8"hidden").VerticalScrollbarWidth() == 0);
    }

    SECTION("scrollbar-gutter: stable reserves the vertical scrollbar's space")
    {
      CHECK_FALSE(ScrollableAreaOf(fixture, u8"gutter")->HasScrollbars());
      CHECK(BoxOf(fixture, u8"gutter").VerticalScrollbarWidth() == 21);
    }

    SECTION("scrollbar-width")
    {
      CHECK(BoxOf(fixture, u8"thin").VerticalScrollbarWidth() == 13);
      CHECK(BoxOf(fixture, u8"thin").HorizontalScrollbarHeight() == 13);
      CHECK(BoxOf(fixture, u8"none").VerticalScrollbarWidth() == 0);
    }

    SECTION("Scroll positions")
    {
      auto &scrollableArea = *ScrollableAreaOf(fixture, u8"scroll");
      // There's no overflow, so programmatic scrolls clamp to the origin.
      CHECK(scrollableArea.ScrollToOffset({10, 20}) == ScrollOffset());
      CHECK(scrollableArea.GetScrollPosition() == ScrollPosition());

      // Unclamped scrolls move the child layers and are kept on the element.
      (void)scrollableArea.ScrollToOffset({10, 20},
                                          ScrollPositionChangeOptions::CreateProgrammaticUnclamped());
      CHECK(scrollableArea.GetScrollPosition() == ScrollPosition(10, 20));
      CHECK(scrollableArea.VerticalScrollbar()->CurrentPos() == 20);
      CHECK(fixture.ById(u8"scroll").SavedLayerScrollPosition() == IntPoint(10, 20));
      CHECK(BoxOf(fixture, u8"child").Layer()->Location() == LayoutPoint(-10, -20));
      CHECK(BoxOf(fixture, u8"scroll").ScrollLeft() == 10);
      CHECK(BoxOf(fixture, u8"scroll").ScrollTop() == 20);
    }

    SECTION("Losing the scrollable area")
    {
      REQUIRE_FALSE(fixture.ById(u8"hidden").SetAttribute(u8"style", u8"overflow: visible").HasException());
      document.UpdateStyleIfNeeded();
      CHECK_FALSE(BoxOf(fixture, u8"hidden").Layer());

      REQUIRE_FALSE(fixture.ById(u8"rel").SetAttribute(u8"style", u8"overflow: scroll").HasException());
      document.UpdateStyleIfNeeded();
      REQUIRE(ScrollableAreaOf(fixture, u8"rel"));
      CHECK(ScrollableAreaOf(fixture, u8"rel")->HasScrollbars());
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture,
                   "RenderLayerScrollableArea: changing scrollbar-width resizes the scrollbars",
                   "[Rendering][RenderLayerScrollableArea][css-scrollbars:scrollbar-width]")
  {
    ResolverFixture fixture {
      u8R"html(<!DOCTYPE html><div id=box style="overflow: scroll; width: 100px; height: 100px"></div>)html"};
    auto &document = *fixture.Document;
    auto relayout = [&]
    {
      document.UpdateStyleIfNeeded();
      (void)document.UpdateLayout();
    };
    relayout();
    auto &box = fixture.ById(u8"box");
    REQUIRE(ScrollableAreaOf(fixture, u8"box")->VerticalScrollbar());
    CHECK(ScrollableAreaOf(fixture, u8"box")->VerticalScrollbar()->Width() == 21);

    REQUIRE_FALSE(
      box.SetAttribute(u8"style", u8"overflow: scroll; width: 100px; height: 100px; scrollbar-width: thin")
        .HasException());
    relayout();
    CHECK(ScrollableAreaOf(fixture, u8"box")->VerticalScrollbar()->Width() == 13);
    CHECK(ScrollableAreaOf(fixture, u8"box")->HorizontalScrollbar()->Height() == 13);
    CHECK(BoxOf(fixture, u8"box").VerticalScrollbarWidth() == 13);
    // The content box gives up only the thin scrollbar's width.
    CHECK(BoxOf(fixture, u8"box").ContentBoxWidth() == 87);

    REQUIRE_FALSE(
      box.SetAttribute(u8"style", u8"overflow: scroll; width: 100px; height: 100px").HasException());
    relayout();
    CHECK(ScrollableAreaOf(fixture, u8"box")->VerticalScrollbar()->Width() == 21);
  }

  TEST_CASE_METHOD(css::tests::ValueFixture,
                   "RenderLayerScrollableArea: scrollbar-color colours the thumb and track",
                   "[Rendering][RenderLayerScrollableArea][css-scrollbars:scrollbar-color]")
  {
    constexpr utf8_stringview Thumb = u8"(color #FF0000)";
    constexpr utf8_stringview Track = u8"(color #0000FF)";
    auto scroller = [](utf8_stringview colour)
    {
      return u8"<!DOCTYPE html><style>body { margin: 0 }</style><div id=box style='overflow: scroll; width: "
             u8"100px; height: 80px; scrollbar-color: "
             + utf8_string(colour) + u8"'><div style='width: 300px; height: 300px'></div></div>";
    };

    SECTION("auto leaves the theme's colours, and no track")
    {
      PaintFixture fixture(scroller(u8"auto"));
      auto painted = fixture.Painted();
      CHECK(painted.find(Thumb) == utf8_string::npos);
      CHECK(painted.find(Track) == utf8_string::npos);
      CHECK_FALSE(ScrollableAreaOf(fixture, u8"box")->ScrollbarColorStyle());
    }

    SECTION("two colours: the thumb's and the track's, which fills the tracks and the corner")
    {
      PaintFixture fixture(scroller(u8"red blue"));
      auto painted = fixture.Painted();
      auto colours = ScrollableAreaOf(fixture, u8"box")->ScrollbarColorStyle();
      REQUIRE(colours);
      CHECK(colours->ThumbColor == Color(SRGBA<uint8> {255, 0, 0}));
      CHECK(colours->TrackColor == Color(SRGBA<uint8> {0, 0, 255}));

      // The vertical track, the horizontal track and the corner between them.
      CHECK(painted.find(u8"(rect at (79,0) size 21x59)") != utf8_string::npos);
      CHECK(painted.find(u8"(rect at (0,59) size 79x21)") != utf8_string::npos);
      CHECK(painted.find(u8"(rect at (79,59) size 21x21)") != utf8_string::npos);
      size_t tracks = 0;
      for (auto at = painted.find(Track); at != utf8_string::npos; at = painted.find(Track, at + 1))
      {
        ++tracks;
      }
      CHECK(tracks == 3u);
      CHECK(painted.find(Thumb) != utf8_string::npos);
    }

    SECTION("the root's colours are the viewport's")
    {
      PaintFixture fixture(u8"<!DOCTYPE html><style>html { scrollbar-color: red blue }</style>"
                           u8"<div style='height: 300px'></div>");
      (void)fixture.Painted();
      auto colours = fixture.Document->View()->ScrollbarColorStyle();
      REQUIRE(colours);
      CHECK(colours->TrackColor == Color(SRGBA<uint8> {0, 0, 255}));
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture,
                   "RenderLayerScrollableArea: scrollbars follow color-scheme's dark appearance",
                   "[Rendering][RenderLayerScrollableArea][css-color-adjust:color-scheme-prop]")
  {
    SECTION("a scroller's own color-scheme")
    {
      ResolverFixture fixture {u8R"html(<!DOCTYPE html><style>div { overflow: scroll }</style>
        <div id=dark style="color-scheme: dark"></div><div id=light></div><div id=either style="color-scheme: light dark"></div>)html"};
      fixture.Document->UpdateStyleIfNeeded();
      (void)fixture.Document->UpdateLayout();
      CHECK(ScrollableAreaOf(fixture, u8"dark")->UseDarkAppearanceForScrollbars());
      CHECK_FALSE(ScrollableAreaOf(fixture, u8"light")->UseDarkAppearanceForScrollbars());
      // Light or dark is light unless the page is dark.
      CHECK_FALSE(ScrollableAreaOf(fixture, u8"either")->UseDarkAppearanceForScrollbars());
      CHECK_FALSE(fixture.Document->View()->UseDarkAppearanceForScrollbars());
    }

    SECTION("the root's color-scheme is the viewport's")
    {
      ResolverFixture fixture {u8"<!DOCTYPE html><style>html { color-scheme: dark }</style>"};
      fixture.Document->UpdateStyleIfNeeded();
      (void)fixture.Document->UpdateLayout();
      CHECK(fixture.Document->View()->UseDarkAppearanceForScrollbars());
    }

    SECTION("light dark follows the page's appearance")
    {
      PaintFixture fixture(
        u8"<!DOCTYPE html><div id=box style='overflow: scroll; color-scheme: light dark'></div>", {200, 100},
        true);
      (void)fixture.Painted();
      CHECK_FALSE(ScrollableAreaOf(fixture, u8"box")->UseDarkAppearanceForScrollbars());

      auto environment = fixture.TestPage->MainFrame().GetMediaEnvironment();
      environment.UseDarkAppearance = true;
      fixture.TestPage->MainFrame().SetMediaEnvironment(environment);
      (void)fixture.Painted();
      CHECK(ScrollableAreaOf(fixture, u8"box")->UseDarkAppearanceForScrollbars());
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture,
                   "RenderLayerScrollableArea: overlay scrollbars take no space and fade",
                   "[Rendering][RenderLayerScrollableArea][ScrollbarsController]")
  {
    using namespace std::chrono_literals;
    ScrollbarThemeAdwaita::SetUsesOverlayScrollbars(true);
    auto classic = MakeScopeExit([] { ScrollbarThemeAdwaita::SetUsesOverlayScrollbars(false); });

    ResolverFixture fixture {
      u8R"html(<!DOCTYPE html><div id=box style="overflow: scroll; width: 100px; height: 100px">
      <div style="width: 300px; height: 300px"></div></div>)html"};
    fixture.Document->UpdateStyleIfNeeded();
    (void)fixture.Document->UpdateLayout();
    auto &loop = html::EventLoop::Current();
    auto &area = *ScrollableAreaOf(fixture, u8"box");
    REQUIRE(area.VerticalScrollbar());
    auto &scrollbar = *area.VerticalScrollbar();
    CHECK(scrollbar.IsOverlayScrollbar());
    CHECK(BoxOf(fixture, u8"box").VerticalScrollbarWidth() == 0);
    CHECK(BoxOf(fixture, u8"box").ContentBoxWidth() == 100);

    // A new scrollbar shows, then fades out after two seconds, over a second.
    CHECK(scrollbar.Opacity() == 1);
    loop.Run(loop.Now() + 2'500ms);
    CHECK(scrollbar.Opacity() > 0);
    CHECK(scrollbar.Opacity() < 1);
    loop.Run(loop.Now() + 1s);
    CHECK(scrollbar.Opacity() == 0);

    SECTION("the mouse moving over the area shows it")
    {
      area.MouseMovedInContentArea();
      loop.Run(loop.Now() + 1s);
      CHECK(scrollbar.Opacity() == 1);
      loop.Run(loop.Now() + 3'100ms);
      CHECK(scrollbar.Opacity() == 0);
    }

    SECTION("scrolling shows it")
    {
      (void)area.ScrollToOffset({0, 50});
      loop.Run(loop.Now() + 1s);
      CHECK(scrollbar.Opacity() == 1);
    }

    SECTION("hiding the page hides it at once")
    {
      area.MouseMovedInContentArea();
      loop.Run(loop.Now() + 1s);
      area.ContentAreaDidHide();
      CHECK(scrollbar.Opacity() == 0);
    }
  }

  TEST_CASE_METHOD(
    css::tests::ValueFixture,
    "RenderLayerScrollableArea: overlay scrollbars paint over the content, and show for the mouse",
    "[Rendering][RenderLayerScrollableArea][ScrollbarsController]")
  {
    using namespace std::chrono_literals;
    ScrollbarThemeAdwaita::SetUsesOverlayScrollbars(true);
    auto classic = MakeScopeExit([] { ScrollbarThemeAdwaita::SetUsesOverlayScrollbars(false); });

    // The child has a layer of its own, which covers where the scrollbar is.
    PaintFixture fixture(
      u8"<!DOCTYPE html><style>body { margin: 0 }</style>"
      u8"<div id=box style='overflow: scroll; width: 100px; height: 100px; scrollbar-color: red "
      u8"blue'><div style='position: relative; width: 300px; height: 300px; background: lime'>"
      u8"</div></div>",
      {200, 100}, true);
    constexpr utf8_stringview Thumb = u8"(color #FF0000)";
    constexpr utf8_stringview Child = u8"(color #00FF00)";
    auto &loop = html::EventLoop::Current();

    auto painted = fixture.Painted();
    REQUIRE(painted.find(Child) != utf8_string::npos);
    REQUIRE(painted.find(Thumb) != utf8_string::npos);
    CHECK(painted.find(Thumb) > painted.find(Child));

    // Faded out, it isn't drawn.
    loop.Run(loop.Now() + 3'100ms);
    CHECK(fixture.Painted().find(Thumb) == utf8_string::npos);

    // The mouse coming over the scroller shows it.
    auto move = PlatformMouseEvent(DoublePoint(50, 50), DoublePoint(50, 50), MouseButton::None,
                                   PlatformEvent::Type::MouseMoved, 0, {}, MonotonicTime::Now());
    (void)fixture.Frame->GetEventHandler().MouseMoved(move);
    loop.Run(loop.Now() + 1s);
    CHECK(ScrollableAreaOf(fixture, u8"box")->VerticalScrollbar()->Opacity() == 1);
    CHECK(fixture.Painted().find(Thumb) != utf8_string::npos);
  }

  TEST_CASE_METHOD(
    css::tests::ValueFixture,
    "RenderLayerScrollableArea: the mouse in a nested scroller is in the scrollers around it too",
    "[Rendering][RenderLayerScrollableArea][ScrollbarsController]")
  {
    using namespace std::chrono_literals;
    ScrollbarThemeAdwaita::SetUsesOverlayScrollbars(true);
    auto classic = MakeScopeExit([] { ScrollbarThemeAdwaita::SetUsesOverlayScrollbars(false); });

    PaintFixture fixture(
      u8"<!DOCTYPE html><style>body { margin: 0 } div { overflow: scroll }</style>"
      u8"<div id=outer style='width: 150px; height: 90px'>"
      u8"<div id=inner style='width: 60px; height: 60px'><p style='width: 200px; height: 200px'></div>"
      u8"<p style='width: 300px; height: 300px'></div>",
      {200, 100}, true);
    (void)fixture.Painted();
    auto &loop = html::EventLoop::Current();
    auto &outer = *ScrollableAreaOf(fixture, u8"outer")->VerticalScrollbar();
    auto &inner = *ScrollableAreaOf(fixture, u8"inner")->VerticalScrollbar();
    auto move = [&](double x, double y)
    {
      (void)fixture.Frame->GetEventHandler().MouseMoved(
        PlatformMouseEvent(DoublePoint(x, y), DoublePoint(x, y), MouseButton::None,
                           PlatformEvent::Type::MouseMoved, 0, {}, MonotonicTime::Now()));
    };

    // Both fade out once they're made, and the mouse starts outside them.
    move(180, 95);
    loop.Run(loop.Now() + 3'100ms);
    REQUIRE(outer.Opacity() == 0);
    REQUIRE(inner.Opacity() == 0);

    // Straight into the inner scroller: it's in the outer one too.
    move(30, 30);
    loop.Run(loop.Now() + 1s);
    CHECK(inner.Opacity() == 1);
    CHECK(outer.Opacity() == 1);

    // Into the outer one alone: the inner one fades while the outer one, moved over, stays.
    move(120, 70);
    loop.Run(loop.Now() + 1'500ms);
    move(121, 70);
    loop.Run(loop.Now() + 1'600ms);
    CHECK(inner.Opacity() == 0);
    CHECK(outer.Opacity() == 1);
  }
}
