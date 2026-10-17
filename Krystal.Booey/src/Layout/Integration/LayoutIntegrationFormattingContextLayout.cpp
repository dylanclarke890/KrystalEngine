#include "Krystal.Booey/Layout/Integration/LayoutIntegrationFormattingContextLayout.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Block/BlockLayoutState.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineLayoutState.hpp"
#include "Krystal.Booey/Layout/Integration/LayoutIntegrationBoxGeometryUpdater.hpp"
#include "Krystal.Booey/Layout/Integration/LayoutIntegrationUtils.hpp"
#include "Krystal.Booey/Layout/LayoutState.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutBoxInlines.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutElementBox.hpp"
#include "Krystal.Booey/Rendering/FloatingObjects.hpp"
#include "Krystal.Booey/Rendering/RenderBlockFlow.hpp"
#include "Krystal.Booey/Rendering/Shapes/ShapeOutsideInfo.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/TextBoxTrimmer.hpp"

namespace krys::boo::layout_integration
{
  namespace
  {
    KRYS_NODISCARD inline const layout::ElementBox &RootLayoutBox(const layout::ElementBox &child) noexcept
    {
      auto *ancestor = &child.Parent();
      while (!ancestor->IsInitialContainingBlock())
      {
        if (ancestor->EstablishesFormattingContext())
        {
          break;
        }
        ancestor = &ancestor->Parent();
      }
      return *ancestor;
    }

    inline void PopulateRootRendererWithFloatsFromIFC(RenderBlockFlow &rootBlockContainer,
                                                      const layout::PlacedFloats &placedFloats) noexcept
    {
      auto blockFormattingContextRootWritingMode =
        placedFloats.BlockFormattingContextRoot().Style().GetWritingMode();
      for (auto &floatItem : placedFloats.GetList())
      {
        auto *layoutBox = floatItem.LayoutBox();
        if (!layoutBox)
        {
          // Floats inherited by IFC do not have associated layout boxes.
          continue;
        }
        auto &floatingObject =
          rootBlockContainer.InsertFloatingBox(Downcast<RenderBox>(*layoutBox->RendererForIntegration()));
        if (floatingObject.IsPlaced())
        {
          // We have already inserted this float when laying out a previous middle-block.
          continue;
        }

        auto [marginBoxVisualRect, borderBoxVisualRect] =
          layout::IntegrationUtils::ToMarginAndBorderBoxVisualRect(
            floatItem.GetBoxGeometry(), rootBlockContainer.Size(), blockFormattingContextRootWritingMode);
        floatingObject.SetFrameRect(marginBoxVisualRect);
        floatingObject.SetMarginOffset({borderBoxVisualRect.X() - marginBoxVisualRect.X(),
                                        borderBoxVisualRect.Y() - marginBoxVisualRect.Y()});
        floatingObject.SetIsPlaced(true);
      }
    }

    inline void PopulateIFCWithNewlyPlacedFloats(const RenderBox &blockRenderer,
                                                 layout::PlacedFloats &placedFloats,
                                                 LayoutPoint blockLogicalTopLeft) noexcept
    {
      auto *renderBlockFlow = DynamicDowncast<RenderBlockFlow>(blockRenderer);
      if (!renderBlockFlow)
      {
        return;
      }

      if (!renderBlockFlow->ContainsFloats() || renderBlockFlow->CreatesNewFormattingContext())
      {
        return;
      }

      for (auto &floatingObject : *renderBlockFlow->GetFloatingObjectSet())
      {
        if (!floatingObject->Renderer())
        {
          continue;
        }
        if (!floatingObject->IsDescendant())
        {
          continue;
        }

        auto floatRect = floatingObject->FrameRect();

        auto boxGeometry = layout::BoxGeometry {};
        boxGeometry.SetTopLeft(blockLogicalTopLeft + floatRect.Location());
        boxGeometry.SetContentBoxWidth(floatRect.Width());
        boxGeometry.SetContentBoxHeight(floatRect.Height());
        boxGeometry.SetBorder({});
        boxGeometry.SetPadding({});
        boxGeometry.SetHorizontalMargin({});
        boxGeometry.SetVerticalMargin({});

        auto *shapeOutsideInfo = floatingObject->Renderer()->GetShapeOutsideInfo();
        auto shape = shapeOutsideInfo ? ShareRefPtr(&shapeOutsideInfo->ComputedShape()) : nullptr;

        auto usedPosition = RenderStyle::GetUsedFloat(*floatingObject->Renderer()) == UsedFloat::Left
                              ? layout::PlacedFloats::Item::Position::Start
                              : layout::PlacedFloats::Item::Position::End;
        placedFloats.Add({usedPosition, boxGeometry, floatRect.Location(), krys::move(shape)});
      }
    }
  }

  void LayoutWithFormattingContextForBox(const layout::ElementBox &box, Maybe<LayoutUnit> widthConstraint,
                                         Maybe<LayoutUnit> heightConstraint,
                                         layout::LayoutState &layoutState) noexcept
  {
    auto &renderer = Downcast<RenderBox>(*box.RendererForIntegration());

    if (widthConstraint)
    {
      renderer.SetOverridingBorderBoxLogicalWidth(*widthConstraint);
      renderer.SetNeedsLayout(RenderObjectMarkingBehavior::MarkOnlyThis);
    }

    if (heightConstraint)
    {
      renderer.SetOverridingBorderBoxLogicalHeight(*heightConstraint);
      renderer.SetNeedsLayout(RenderObjectMarkingBehavior::MarkOnlyThis);
    }

    renderer.LayoutIfNeeded();

    if (widthConstraint)
    {
      renderer.ClearOverridingBorderBoxLogicalWidth();
    }

    auto updater = BoxGeometryUpdater {layoutState, RootLayoutBox(box)};
    updater.UpdateBoxGeometryAfterIntegrationLayout(
      box, widthConstraint.value_or(renderer.ContainingBlock()->ContentBoxLogicalWidth()));
  }

  void LayoutWithFormattingContextForBlockInInline(const layout::ElementBox &block,
                                                   LayoutPoint blockLineLogicalTopLeft,
                                                   layout::InlineLayoutState &inlineLayoutState,
                                                   layout::LayoutState &layoutState) noexcept
  {
    auto &parentBlockLayoutState = inlineLayoutState.ParentBlockLayoutState();
    auto &placedFloats = parentBlockLayoutState.GetPlacedFloats();
    auto &blockRenderer = Downcast<RenderBox>(*block.RendererForIntegration());
    auto &rootBlockContainer = Downcast<RenderBlockFlow>(*RootLayoutBox(block).RendererForIntegration());

    // The legacy line clamp (-webkit-line-clamp) was dropped, so only the floats need updating.
    PopulateRootRendererWithFloatsFromIFC(rootBlockContainer, placedFloats);

    auto positionAndMargin = RenderBlockFlow::BlockPositionAndMargin {};
    auto layoutBlockRenderer = [&]
    {
      if (inlineLayoutState.LineCount())
      {
        auto textBoxTrimStartDisabler = TextBoxTrimStartDisabler {blockRenderer};
        positionAndMargin = rootBlockContainer.LayoutBlockChildFromInlineLayout(
          blockRenderer, blockLineLogicalTopLeft.Y(),
          layout::IntegrationUtils::ToMarginInfo(parentBlockLayoutState.GetMarginState()));
        return;
      }
      positionAndMargin = rootBlockContainer.LayoutBlockChildFromInlineLayout(
        blockRenderer, blockLineLogicalTopLeft.Y(),
        layout::IntegrationUtils::ToMarginInfo(parentBlockLayoutState.GetMarginState()));
    };
    layoutBlockRenderer();
    krys_debug_assert(!blockRenderer.NeedsLayout());

    auto updateIFCAfterLayout = [&]
    {
      auto updater = BoxGeometryUpdater {layoutState, RootLayoutBox(block)};
      updater.UpdateBoxGeometryAfterIntegrationLayout(block, rootBlockContainer.ContentBoxLogicalWidth());

      auto &blockGeometry = layoutState.EnsureGeometryForBox(block);
      auto resolvedMarginBefore = positionAndMargin.ChildLogicalTop - blockLineLogicalTopLeft.Y();
      blockGeometry.SetTopLeft(LayoutPoint {blockGeometry.MarginStart(), resolvedMarginBefore});
      // We don't know what the after margin here is (or if there's any at all) before processing the content
      // after.
      // FIXME: Check if blockGeometry needs the adjusted margin after value at all.
      blockGeometry.SetVerticalMargin({resolvedMarginBefore, {}});

      PopulateIFCWithNewlyPlacedFloats(blockRenderer, placedFloats, blockLineLogicalTopLeft);
      auto contentOffsetAfterSelfCollapsingBlock =
        blockRenderer.IsSelfCollapsingBlock()
          ? positionAndMargin.ContainerLogicalBottom - positionAndMargin.ChildLogicalTop
          : 0_lu;
      parentBlockLayoutState.GetMarginState() = layout::IntegrationUtils::ToMarginState(
        positionAndMargin.ChildMarginInfo, contentOffsetAfterSelfCollapsingBlock);
    };
    updateIFCAfterLayout();
  }

  LayoutUnit FormattingContextRootLogicalWidthForType(const layout::ElementBox &box,
                                                      LogicalWidthType logicalWidthType) noexcept
  {
    krys_debug_assert(box.EstablishesFormattingContext());

    auto &renderer = Downcast<RenderBox>(*box.RendererForIntegration());
    switch (logicalWidthType)
    {
      case LogicalWidthType::PreferredMaximum: return renderer.MaxPreferredLogicalWidth();
      case LogicalWidthType::PreferredMinimum: return renderer.MinPreferredLogicalWidth();
      case LogicalWidthType::MaxContent:
      case LogicalWidthType::MinContent:
      {
        auto minimunLogicalWidth = LayoutUnit {};
        auto maximumLogicalWidth = LayoutUnit {};
        renderer.ComputeIntrinsicLogicalWidths(minimunLogicalWidth, maximumLogicalWidth);
        return logicalWidthType == LogicalWidthType::MaxContent ? maximumLogicalWidth : minimunLogicalWidth;
      }
    }
    krys_debug_assert(false);
    return {};
  }

  LayoutUnit FormattingContextRootLogicalHeightForType(const layout::ElementBox &box,
                                                       LogicalHeightType logicalHeightType) noexcept
  {
    krys_debug_assert(box.EstablishesFormattingContext());

    switch (logicalHeightType)
    {
      case LogicalHeightType::MinContent:
        // FIXME: The flex item's cached intrinsic content height
        // (cachedFlexItemIntrinsicContentLogicalHeight), once there's flex layout.
        krys_debug_assert(false);
        return {};
    }
    krys_debug_assert(false);
    return {};
  }
}
