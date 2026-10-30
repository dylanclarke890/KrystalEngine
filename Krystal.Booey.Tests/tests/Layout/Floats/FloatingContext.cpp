#include "Krystal.Booey/Layout/Floats/FloatingContext.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Layout/LayoutTestUtils.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/Layout/Floats/PlacedFloats.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Block/BlockFormattingState.hpp"
#include "Krystal.Booey/Layout/Integration/LayoutIntegrationBoxTreeUpdater.hpp"
#include "Krystal.Booey/Layout/LayoutState.hpp"
#include "Krystal.Booey/Rendering/RenderBlockFlow.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"

namespace krys::boo::tests
{
  using style::tests::ResolverFixture;

  namespace
  {
    KRYS_NODISCARD const layout::Box &BoxOf(ResolverFixture &fixture, dom::DOMStringView id) noexcept
    {
      auto box = fixture.ById(id).Renderer()->LayoutBox();
      REQUIRE(box);
      return *box;
    }

    layout::BoxGeometry &SetGeometry(layout::LayoutState &layoutState, const layout::Box &box,
                                     LayoutSize contentBoxSize,
                                     layout::BoxGeometry::HorizontalEdges margin = {},
                                     LayoutPoint topLeft = {}) noexcept
    {
      auto &geometry = layoutState.EnsureGeometryForBox(box);
      geometry.SetTopLeft(topLeft);
      geometry.SetContentBoxSize(contentBoxSize);
      geometry.SetHorizontalMargin(margin);
      geometry.SetVerticalMargin({});
      geometry.SetBorder({});
      geometry.SetPadding({});
      return geometry;
    }

    // Places a float the way the inline layout does: at its static position, then among the floats.
    LayoutPoint PlaceFloat(const layout::FloatingContext &floatingContext, layout::PlacedFloats &placedFloats,
                           layout::BoxGeometry &geometry, const layout::Box &box) noexcept
    {
      auto position = floatingContext.PositionForFloat(box, geometry, {0_lu, 300_lu});
      geometry.SetTopLeft(position);
      placedFloats.Add(floatingContext.MakeFloatItem(box, geometry));
      return position;
    }

    KRYS_NODISCARD layout::PlacedFloats::Item MakeItem(layout::PlacedFloats::Item::Position position,
                                                       LayoutUnit left, LayoutUnit width,
                                                       LayoutUnit marginStart) noexcept
    {
      auto geometry = layout::BoxGeometry {};
      geometry.SetTopLeft({left, 0_lu});
      geometry.SetContentBoxSize({width, 10_lu});
      geometry.SetHorizontalMargin({marginStart, 0_lu});
      geometry.SetVerticalMargin({});
      geometry.SetBorder({});
      geometry.SetPadding({});
      return {position, geometry, {left, 0_lu}, nullptr};
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "FloatingContext: placing floats and float avoiders",
                   "[Layout][Floats][FloatingContext]")
  {
    ResolverFixture fixture {u8R"html(<!DOCTYPE html><style>
      .l { float: left }
      .r { float: right }
      #ib { display: inline-block }
      #brl { clear: left }
      #brr { clear: right }
    </style><body><div id=root><div class=l id=f1></div><div class=l id=f2></div><div class=r id=r1></div><div class=l id=f3></div><span id=ib></span><br id=brl><br id=brr></div>
    <div id=rtl dir=rtl><div class=r id=rtlFloat></div></div></body>)html"};
    (void)fixture.Document->UpdateLayout();

    auto &rootRenderer = Downcast<RenderBlockFlow>(*fixture.ById(u8"root").Renderer());
    auto &rootBox = layout::tests::BuildBoxTree(rootRenderer);
    auto &layoutState = fixture.Document->GetRenderView()->GetLayoutState();

    // The root's content box is 300px wide, with no borders or padding.
    (void)SetGeometry(layoutState, rootBox, {300_lu, 0_lu});

    auto &placedFloats = layoutState.EnsureBlockFormattingState(rootBox).GetPlacedFloats();
    CHECK(&placedFloats.BlockFormattingContextRoot() == &rootBox);
    CHECK(placedFloats.IsEmpty());
    auto floatingContext = layout::FloatingContext {rootBox, layoutState, placedFloats};
    CHECK(floatingContext.IsEmpty());

    auto &f1 = BoxOf(fixture, u8"f1");
    auto &f2 = BoxOf(fixture, u8"f2");
    auto &r1 = BoxOf(fixture, u8"r1");
    auto &f3 = BoxOf(fixture, u8"f3");
    CHECK(floatingContext.IsStartPositioned(f1));
    CHECK_FALSE(floatingContext.IsStartPositioned(r1));

    // f1 (50x30) goes to the start of the line; f2 (40x20) after it; r1 (60x20) to the end.
    CHECK(PlaceFloat(floatingContext, placedFloats, SetGeometry(layoutState, f1, {50_lu, 30_lu}), f1)
          == LayoutPoint {0_lu, 0_lu});
    CHECK(PlaceFloat(floatingContext, placedFloats, SetGeometry(layoutState, f2, {40_lu, 20_lu}), f2)
          == LayoutPoint {50_lu, 0_lu});
    CHECK(PlaceFloat(floatingContext, placedFloats, SetGeometry(layoutState, r1, {60_lu, 20_lu}), r1)
          == LayoutPoint {240_lu, 0_lu});
    CHECK(placedFloats.HasStartPositioned());
    CHECK(placedFloats.HasEndPositioned());

    SECTION("The line constraints next to the floats")
    {
      auto constraints =
        floatingContext.GetConstraints(0_lu, 10_lu, layout::FloatingContext::MayBeAboveLastFloat::No);
      REQUIRE(constraints.Start);
      REQUIRE(constraints.End);
      CHECK(LayoutPoint(*constraints.Start) == LayoutPoint {90_lu, 20_lu});
      CHECK(LayoutPoint(*constraints.End) == LayoutPoint {240_lu, 20_lu});

      // Below f2 and r1, only f1 is left.
      constraints =
        floatingContext.GetConstraints(20_lu, 30_lu, layout::FloatingContext::MayBeAboveLastFloat::Yes);
      REQUIRE(constraints.Start);
      CHECK_FALSE(constraints.End);
      CHECK(LayoutPoint(*constraints.Start) == LayoutPoint {50_lu, 30_lu});

      constraints =
        floatingContext.GetConstraints(30_lu, 40_lu, layout::FloatingContext::MayBeAboveLastFloat::No);
      CHECK_FALSE(constraints.Start);
      CHECK_FALSE(constraints.End);
    }

    SECTION("A float that doesn't fit moves down to the next float pair")
    {
      // f3 (200x10) doesn't fit between f2 and r1 (150px); below them, it fits next to f1.
      CHECK(PlaceFloat(floatingContext, placedFloats, SetGeometry(layoutState, f3, {200_lu, 10_lu}), f3)
            == LayoutPoint {50_lu, 20_lu});
      CHECK(placedFloats.GetList().size() == 4);
      CHECK(placedFloats.HighestPositionOnBlockAxis() == 0_lu);
      CHECK(placedFloats.LowestPositionOnBlockAxis() == 30_lu);
      CHECK(placedFloats.LowestPositionOnBlockAxis(Clear::InlineStart) == 30_lu);
      CHECK(placedFloats.LowestPositionOnBlockAxis(Clear::InlineEnd) == 20_lu);

      // Clearance takes a line break below the floats it clears.
      auto &clearLeft = BoxOf(fixture, u8"brl");
      auto clearance =
        floatingContext.GetBlockAxisPositionWithClearance(clearLeft, SetGeometry(layoutState, clearLeft, {}));
      REQUIRE(clearance);
      CHECK(clearance->Position == 30_lu);
      CHECK(clearance->Clearance == 30_lu);

      auto &clearRight = BoxOf(fixture, u8"brr");
      clearance = floatingContext.GetBlockAxisPositionWithClearance(clearRight,
                                                                    SetGeometry(layoutState, clearRight, {}));
      REQUIRE(clearance);
      CHECK(clearance->Position == 20_lu);

      // A clear position already below the floats needs no clearance.
      clearance = floatingContext.GetBlockAxisPositionWithClearance(
        clearRight, SetGeometry(layoutState, clearRight, {}, {}, {0_lu, 25_lu}));
      CHECK_FALSE(clearance);

      // A formatting context root is placed beside the floats where it fits, without overlapping them.
      auto &inlineBlock = BoxOf(fixture, u8"ib");
      CHECK(floatingContext.PositionForNonFloatingFloatAvoider(
              inlineBlock, SetGeometry(layoutState, inlineBlock, {100_lu, 10_lu}))
            == LayoutPoint {90_lu, 0_lu});
      CHECK(floatingContext.PositionForNonFloatingFloatAvoider(
              inlineBlock, SetGeometry(layoutState, inlineBlock, {200_lu, 10_lu}))
            == LayoutPoint {0_lu, 30_lu});

      // Removing a float.
      CHECK(placedFloats.Remove(f3));
      CHECK_FALSE(placedFloats.Remove(f3));
      CHECK(placedFloats.GetList().size() == 3);
      CHECK(placedFloats.Last()->LayoutBox() == &r1);
    }

    SECTION("Float items")
    {
      auto &item = placedFloats.GetList()[1];
      CHECK(item.LayoutBox() == &f2);
      CHECK(item.IsStartPositioned());
      CHECK(item.IsInFormattingContextOf(rootBox));
      CHECK(LayoutRect(item.AbsoluteRectWithMargin()) == LayoutRect {50_lu, 0_lu, 40_lu, 20_lu});
      CHECK(item.AbsoluteBottom() == 20_lu);
      CHECK_FALSE(item.Shape());
      CHECK_FALSE(item.PlacedByLine());
      CHECK(layout::BoxGeometry::BorderBoxTopLeft(item.GetBoxGeometry()) == LayoutPoint {50_lu, 0_lu});
      CHECK(placedFloats.GetList()[2].IsEndPositioned());

      placedFloats.Clear();
      CHECK(placedFloats.IsEmpty());
      CHECK_FALSE(placedFloats.HasStartPositioned());
      CHECK_FALSE(placedFloats.Last());
    }

    SECTION("A negative inline margin keeps the floats in inline order")
    {
      using Position = layout::PlacedFloats::Item::Position;
      auto items = layout::PlacedFloats {rootBox};
      items.Add(MakeItem(Position::Start, 0_lu, 50_lu, 0_lu));
      // Its margin box ends at 30px, before the first float's end: it goes first.
      items.Add(MakeItem(Position::Start, 0_lu, 30_lu, -10_lu));
      REQUIRE(items.GetList().size() == 2);
      CHECK(items.GetList()[0].AbsoluteRectWithMargin().Right() == 30_lu);
      CHECK(items.GetList()[1].AbsoluteRectWithMargin().Right() == 50_lu);
      CHECK_FALSE(items.GetList()[0].LayoutBox());
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "FloatingContext: right-to-left",
                   "[Layout][Floats][FloatingContext]")
  {
    ResolverFixture fixture {
      u8R"html(<!DOCTYPE html><body><div id=root dir=rtl><div id=r style="float: right"></div><div id=s style="float: inline-start"></div><div id=l style="float: left"></div></div></body>)html"};
    (void)fixture.Document->UpdateLayout();

    auto &rootRenderer = Downcast<RenderBlockFlow>(*fixture.ById(u8"root").Renderer());
    auto &rootBox = layout::tests::BuildBoxTree(rootRenderer);
    auto &layoutState = fixture.Document->GetRenderView()->GetLayoutState();
    (void)SetGeometry(layoutState, rootBox, {300_lu, 0_lu});
    auto placedFloats = layout::PlacedFloats {rootBox};
    auto floatingContext = layout::FloatingContext {rootBox, layoutState, placedFloats};

    // In a right-to-left context, the start is on the right.
    CHECK(floatingContext.IsStartPositioned(BoxOf(fixture, u8"r")));
    CHECK(floatingContext.IsStartPositioned(BoxOf(fixture, u8"s")));
    CHECK_FALSE(floatingContext.IsStartPositioned(BoxOf(fixture, u8"l")));

    // The floats are placed in logical coordinates: the start float at the logical start.
    auto &start = BoxOf(fixture, u8"r");
    CHECK(PlaceFloat(floatingContext, placedFloats, SetGeometry(layoutState, start, {50_lu, 10_lu}), start)
          == LayoutPoint {0_lu, 0_lu});
    auto &end = BoxOf(fixture, u8"l");
    CHECK(PlaceFloat(floatingContext, placedFloats, SetGeometry(layoutState, end, {50_lu, 10_lu}), end)
          == LayoutPoint {250_lu, 0_lu});
  }
}
