#include "Krystal.Booey/Layout/Integration/LayoutIntegrationUtils.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Layout/LayoutTestUtils.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/Layout/Integration/LayoutIntegrationBoxTreeUpdater.hpp"
#include "Krystal.Booey/Layout/Integration/LayoutIntegrationFormattingContextLayout.hpp"
#include "Krystal.Booey/Layout/LayoutState.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutInitialContainingBlock.hpp"
#include "Krystal.Booey/Rendering/RenderBlock.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"

namespace krys::boo::tests
{
  using style::tests::ResolverFixture;

  TEST_CASE("IntegrationUtils: margin state", "[Layout][Integration][IntegrationUtils]")
  {
    auto marginInfo =
      RenderBlockFlow::MarginInfo {true, false, true, false, true, false, true, false, true, 5_lu, 3_lu};
    auto marginState = layout::IntegrationUtils::ToMarginState(marginInfo, 7_lu);
    CHECK(marginState.CanCollapseWithChildren);
    CHECK_FALSE(marginState.CanCollapseMarginBeforeWithChildren);
    CHECK(marginState.CanCollapseMarginAfterWithChildren);
    CHECK_FALSE(marginState.QuirkContainer);
    CHECK(marginState.AtBeforeSideOfBlock);
    CHECK_FALSE(marginState.AtAfterSideOfBlock);
    CHECK(marginState.HasMarginBeforeQuirk);
    CHECK_FALSE(marginState.HasMarginAfterQuirk);
    CHECK(marginState.DeterminedMarginBeforeQuirk);
    CHECK(marginState.PositiveMargin == 5_lu);
    CHECK(marginState.NegativeMargin == 3_lu);
    CHECK(marginState.Margin() == 2_lu);
    CHECK(marginState.ContentOffsetAfterSelfCollapsingBlock == 7_lu);

    auto roundTrip = layout::IntegrationUtils::ToMarginInfo(marginState);
    CHECK(roundTrip.CanCollapseWithChildren());
    CHECK_FALSE(roundTrip.CanCollapseMarginBeforeWithChildren());
    CHECK(roundTrip.CanCollapseMarginAfterWithChildren());
    CHECK_FALSE(roundTrip.QuirkContainer());
    CHECK(roundTrip.AtBeforeSideOfBlock());
    CHECK_FALSE(roundTrip.AtAfterSideOfBlock());
    CHECK(roundTrip.HasMarginBeforeQuirk());
    CHECK_FALSE(roundTrip.HasMarginAfterQuirk());
    CHECK(roundTrip.DeterminedMarginBeforeQuirk());
    CHECK(roundTrip.PositiveMargin() == 5_lu);
    CHECK(roundTrip.NegativeMargin() == 3_lu);

    marginState.ResetMarginValues();
    CHECK(marginState.Margin() == 0_lu);
    CHECK(marginState.ContentOffsetAfterSelfCollapsingBlock == 0_lu);
    marginState.ResetBeforeSideOfBlock();
    CHECK_FALSE(marginState.AtBeforeSideOfBlock);
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "IntegrationUtils: visual rects",
                   "[Layout][Integration][IntegrationUtils]")
  {
    ResolverFixture fixture {u8R"html(<!DOCTYPE html><body><div id=ltr></div><div id=rtl dir=rtl></div>
    <div id=vrl style="writing-mode: vertical-rl"></div><div id=vlr style="writing-mode: vertical-lr"></div></body>)html"};

    auto writingModeOf = [&](dom::DOMStringView id)
    {
      return fixture.ById(id).Renderer()->Style().GetWritingMode();
    };

    // A 10x20 logical content box at (5,6), with margins 1/2/3/4 (start, end, before, after): its margin box
    // is 13x27 at (4,3).
    auto geometry = layout::BoxGeometry {};
    geometry.SetTopLeft({5_lu, 6_lu});
    geometry.SetContentBoxSize({10_lu, 20_lu});
    geometry.SetHorizontalMargin({1_lu, 2_lu});
    geometry.SetVerticalMargin({3_lu, 4_lu});
    geometry.SetBorder({});
    geometry.SetPadding({});
    auto containerSize = LayoutSize {100_lu, 200_lu};

    auto [ltrMarginBox, ltrBorderBox] = layout::IntegrationUtils::ToMarginAndBorderBoxVisualRect(
      geometry, containerSize, writingModeOf(u8"ltr"));
    CHECK(ltrMarginBox == LayoutRect {4_lu, 3_lu, 13_lu, 27_lu});
    CHECK(ltrBorderBox == LayoutRect {5_lu, 6_lu, 10_lu, 20_lu});

    // Right-to-left flips the inline axis within the container, and swaps the start and end margins.
    auto [rtlMarginBox, rtlBorderBox] = layout::IntegrationUtils::ToMarginAndBorderBoxVisualRect(
      geometry, containerSize, writingModeOf(u8"rtl"));
    CHECK(rtlMarginBox == LayoutRect {83_lu, 3_lu, 13_lu, 27_lu});
    CHECK(rtlBorderBox == LayoutRect {85_lu, 6_lu, 10_lu, 20_lu});

    // Vertical writing modes swap the axes; vertical-lr's line is inverted, so its after margin is on the
    // left.
    auto [vrlMarginBox, vrlBorderBox] = layout::IntegrationUtils::ToMarginAndBorderBoxVisualRect(
      geometry, containerSize, writingModeOf(u8"vrl"));
    CHECK(vrlMarginBox == LayoutRect {3_lu, 4_lu, 27_lu, 13_lu});
    CHECK(vrlBorderBox == LayoutRect {6_lu, 5_lu, 20_lu, 10_lu});

    auto [vlrMarginBox, vlrBorderBox] = layout::IntegrationUtils::ToMarginAndBorderBoxVisualRect(
      geometry, containerSize, writingModeOf(u8"vlr"));
    CHECK(vlrMarginBox == LayoutRect {3_lu, 4_lu, 27_lu, 13_lu});
    CHECK(vlrBorderBox == LayoutRect {7_lu, 5_lu, 20_lu, 10_lu});
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "IntegrationUtils: formatting context callbacks",
                   "[Layout][Integration][IntegrationUtils]")
  {
    ResolverFixture fixture {
      u8R"html(<!DOCTYPE html><body><div id=flex style="display: flex"><div id=flexItem></div></div>
    <div id=grid style="display: grid"><div id=gridItem></div></div></body>)html"};
    (void)fixture.Document->UpdateLayout();

    auto &flexRenderer = Downcast<RenderBlock>(*fixture.ById(u8"flex").Renderer());
    auto &flexBox = layout::tests::BuildBoxTree(flexRenderer);
    auto &flexItem = Downcast<layout::ElementBox>(*flexBox.FirstChild());
    CHECK(flexBox.EstablishesFlexFormattingContext());
    CHECK(flexItem.IsFlexItem());
    CHECK(flexItem.EstablishesIndependentFormattingContext());

    auto &gridRenderer = Downcast<RenderBlock>(*fixture.ById(u8"grid").Renderer());
    auto &gridBox = layout::tests::BuildBoxTree(gridRenderer);
    auto &gridItem = Downcast<layout::ElementBox>(*gridBox.FirstChild());
    CHECK(gridBox.EstablishesGridFormattingContext());
    CHECK(gridItem.IsGridItem());

    // The layout state hands the formatting context roots to the render tree's layout.
    const layout::ElementBox *laidOut = nullptr;
    Maybe<LayoutUnit> widthConstraint;
    auto layoutState = CreateUnique<layout::LayoutState>(
      *fixture.Document, fixture.Document->GetRenderView()->GetInitialContainingBlock(),
      layout::LayoutState::Type::Secondary,
      [&](const layout::ElementBox &box, Maybe<LayoutUnit> width, Maybe<LayoutUnit>, layout::LayoutState &)
      {
        laidOut = &box;
        widthConstraint = width;
      },
      [](const layout::ElementBox &, layout_integration::LogicalWidthType type)
      {
        return type == layout_integration::LogicalWidthType::MaxContent
                 ? 100_lu
                 : LayoutUnit(static_cast<int32>(type));
      },
      [](const layout::ElementBox &, layout_integration::LogicalHeightType) { return 7_lu; }, nullptr);
    auto utils = layout::IntegrationUtils {*layoutState};

    utils.LayoutWithFormattingContextForBox(flexItem, 50_lu);
    CHECK(laidOut == &flexItem);
    CHECK(widthConstraint == 50_lu);
    CHECK(utils.MaxContentWidth(flexItem) == 100_lu);
    CHECK(utils.MinContentWidth(flexItem) == 3_lu);
    CHECK(utils.MinContentHeight(flexItem) == 7_lu);
    CHECK(utils.PreferredMaxWidth(gridItem) == 0_lu);
    CHECK(utils.PreferredMinWidth(gridItem) == 1_lu);
  }
}
