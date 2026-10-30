#include "Krystal.Booey/Layout/LayoutState.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Layout/LayoutTestUtils.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Block/BlockFormattingState.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineContentCache.hpp"
#include "Krystal.Booey/Layout/Integration/LayoutIntegrationBoxTreeUpdater.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutInitialContainingBlock.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Rendering/RenderBlockFlow.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"

namespace krys::boo::tests
{
  using style::tests::ResolverFixture;

  TEST_CASE_METHOD(css::tests::ValueFixture, "LayoutState: the render view's layout state",
                   "[Layout][LayoutState]")
  {
    ResolverFixture fixture {u8R"html(<!DOCTYPE html><body><div id=root>text</div></body>)html"};
    (void)fixture.Document->UpdateLayout();

    auto &view = *fixture.Document->GetRenderView();
    auto &layoutState = view.GetLayoutState();
    auto &initialContainingBlock = view.GetInitialContainingBlock();
    CHECK(layoutState.GetType() == layout::LayoutState::Type::Primary);
    CHECK(&layoutState.Root() == &initialContainingBlock);
    CHECK(layoutState.InStandardsMode());
    CHECK(layoutState.IsTextShapingAcrossInlineBoxesEnabled());

    // Layout sizes the initial containing block to the view.
    REQUIRE(layoutState.HasBoxGeometry(initialContainingBlock));
    auto &geometry = layoutState.GeometryForBox(initialContainingBlock);
    CHECK(geometry.ContentBoxWidth() == LayoutUnit(view.FrameView().Size().Width()));
    CHECK(geometry.ContentBoxHeight() == LayoutUnit(view.FrameView().Size().Height()));
    CHECK(&layoutState.GeometryForRootBox() == &geometry);

    // The layout state follows the document's mode.
    // FIXME: Changing the mode of a document that has a render view, once something can (document.open()).
    ResolverFixture quirks {u8R"html(<body>text</body>)html"};
    (void)quirks.Document->UpdateLayout();
    CHECK(quirks.Document->GetRenderView()->GetLayoutState().InQuirksMode());
    ResolverFixture limitedQuirks {
      u8R"html(<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN"><body>text</body>)html"};
    (void)limitedQuirks.Document->UpdateLayout();
    CHECK(limitedQuirks.Document->GetRenderView()->GetLayoutState().InLimitedQuirksMode());

    SECTION("Block formatting states")
    {
      auto &rootRenderer = Downcast<RenderBlockFlow>(*fixture.ById(u8"root").Renderer());
      auto &rootBox = layout::tests::BuildBoxTree(rootRenderer);
      CHECK_FALSE(layoutState.HasFormattingState(rootBox));

      auto &formattingState = layoutState.EnsureBlockFormattingState(rootBox);
      CHECK(&layoutState.EnsureBlockFormattingState(rootBox) == &formattingState);
      CHECK(layoutState.HasFormattingState(rootBox));
      CHECK(&layoutState.FormattingStateForBlockFormattingContext(rootBox) == &formattingState);
      CHECK(&layoutState.FormattingStateForFormattingContext(rootBox) == &formattingState);
      CHECK(formattingState.IsBlockFormattingState());
      CHECK(&formattingState.GetLayoutState() == &layoutState);
      CHECK(&formattingState.GetPlacedFloats().BlockFormattingContextRoot() == &rootBox);

      auto &text = *rootBox.FirstChild();
      CHECK_FALSE(formattingState.HasUsedVerticalMargin(text));
      formattingState.SetUsedVerticalMargin(text, {{1_lu, 2_lu}, {}, {}});
      CHECK(formattingState.HasUsedVerticalMargin(text));
      CHECK(layout::MarginBefore(formattingState.GetUsedVerticalMargin(text)) == 1_lu);
      CHECK(layout::MarginAfter(formattingState.GetUsedVerticalMargin(text)) == 2_lu);

      formattingState.SetHasClearance(text);
      CHECK(formattingState.HasClearance(text));
      formattingState.ClearHasClearance(text);
      CHECK_FALSE(formattingState.HasClearance(text));

      formattingState.AddOutOfFlowBox(text);
      CHECK(formattingState.OutOfFlowBoxes().size() == 1);
      formattingState.SetOutOfFlowBoxes({});
      CHECK(formattingState.OutOfFlowBoxes().empty());

      CHECK_FALSE(formattingState.IntrinsicWidthConstraintsForBox(text));
      formattingState.SetIntrinsicWidthConstraintsForBox(text, {10_lu, 20_lu});
      REQUIRE(formattingState.IntrinsicWidthConstraintsForBox(text));
      CHECK(formattingState.IntrinsicWidthConstraintsForBox(text)->Maximum == 20_lu);
      formattingState.SetIntrinsicWidthConstraints({1_lu, 2_lu});
      formattingState.ClearIntrinsicWidthConstraints(text);
      CHECK_FALSE(formattingState.IntrinsicWidthConstraintsForBox(text));
      CHECK_FALSE(formattingState.GetIntrinsicWidthConstraints());

      CHECK(&formattingState.GetBoxGeometry(text) == &layoutState.EnsureGeometryForBox(text));

      layoutState.DestroyBlockFormattingState(rootBox);
      CHECK_FALSE(layoutState.HasFormattingState(rootBox));
    }

    SECTION("Inline content caches")
    {
      auto &rootRenderer = Downcast<RenderBlockFlow>(*fixture.ById(u8"root").Renderer());
      auto &rootBox = layout::tests::BuildBoxTree(rootRenderer);

      auto &inlineContentCache = layoutState.GetInlineContentCache(rootBox);
      CHECK(&layoutState.GetInlineContentCache(rootBox) == &inlineContentCache);
      CHECK(inlineContentCache.GetInlineItems().IsEmpty());
      inlineContentCache.SetMinimumContentSize(10);
      CHECK(layoutState.GetInlineContentCache(rootBox).MinimumContentSize() == 10.f);

      // A destroyed cache starts again empty.
      layoutState.DestroyInlineContentCache(rootBox);
      CHECK_FALSE(layoutState.GetInlineContentCache(rootBox).MinimumContentSize());
    }

    SECTION("A secondary layout state keeps its own geometries")
    {
      auto secondary = CreateUnique<layout::LayoutState>(*fixture.Document, initialContainingBlock,
                                                         layout::LayoutState::Type::Secondary, nullptr,
                                                         nullptr, nullptr, nullptr);
      CHECK(secondary->GetType() == layout::LayoutState::Type::Secondary);
      CHECK_FALSE(secondary->HasBoxGeometry(initialContainingBlock));

      auto &secondaryGeometry = secondary->EnsureGeometryForBox(initialContainingBlock);
      CHECK(&secondary->EnsureGeometryForBox(initialContainingBlock) == &secondaryGeometry);
      CHECK(&secondary->GeometryForBox(initialContainingBlock) == &secondaryGeometry);
      CHECK(&secondaryGeometry != &geometry);
    }
  }
}
