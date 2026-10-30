#include "Krystal.Booey/Rendering/RenderLayerScrollableArea.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/Platform/ScrollbarTheme.hpp"
#include "Krystal.Booey/Rendering/RenderBox.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"

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
    // Classic Adwaita scrollbars are 21 pixels thick, thin or not.
    CHECK(theme.ScrollbarThickness() == 21);
    CHECK(theme.ScrollbarThickness(ScrollbarWidth::Thin) == 21);
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
      CHECK(BoxOf(fixture, u8"thin").VerticalScrollbarWidth() == 21);
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
}
