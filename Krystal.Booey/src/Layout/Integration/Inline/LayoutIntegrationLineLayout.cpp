#include "Krystal.Booey/Layout/Integration/Inline/LayoutIntegrationLineLayout.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Block/BlockFormattingState.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Block/BlockLayoutState.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineContentCache.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineFormattingUtils.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineItemsBuilder.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/LayoutIntegrationInlineContent.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/LayoutIntegrationInlineContentBuilder.hpp"
#include "Krystal.Booey/Layout/Integration/LayoutIntegrationBoxTreeUpdater.hpp"
#include "Krystal.Booey/Layout/Integration/LayoutIntegrationUtils.hpp"
#include "Krystal.Booey/Layout/LayoutState.hpp"
#include "Krystal.Booey/Layout/LayoutTree/FormattingContextBoxIterator.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutBoxGeometry.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutBoxInlines.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Page/LocalFrameViewLayoutContext.hpp"
#include "Krystal.Booey/Rendering/FloatingObjects.hpp"
#include "Krystal.Booey/Rendering/InlineWalker.hpp"
#include "Krystal.Booey/Rendering/RenderDescendantIterator.hpp"
#include "Krystal.Booey/Rendering/RenderInline.hpp"
#include "Krystal.Booey/Rendering/RenderLayer.hpp"
#include "Krystal.Booey/Rendering/RenderLayoutState.hpp"
#include "Krystal.Booey/Rendering/RenderLineBreak.hpp"
#include "Krystal.Booey/Rendering/RenderListMarker.hpp"
#include "Krystal.Booey/Rendering/RenderReplaced.hpp"
#include "Krystal.Booey/Rendering/RenderText.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/Rendering/Shapes/ShapeOutsideInfo.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"

#include <ranges>

namespace krys::boo::layout_integration
{
  namespace
  {
    KRYS_NODISCARD const inline_display::Line &
      LastLineWithInflowContent(const inline_display::Lines &lines) noexcept
    {
      // Out-of-flow/float content only don't produce lines with inline content. They should not be taken into
      // account when computing content box height/baselines.
      for (auto index = lines.size(); index--;)
      {
        auto &line = lines[index];
        krys_debug_assert(line.BoxCount());
        if (line.HasContentfulInFlowBox())
        {
          return line;
        }
        // FIXME: This should be merged with margin collapsing.
        if (line.HasInflowBox() && index
            && lines[index - 1].LineBoxLogicalRect().MaxY() > line.LineBoxLogicalRect().Y())
        {
          krys_debug_assert(lines[index - 1].HasBlockLevelBox());
          return line;
        }
      }
      return lines.front();
    }

    KRYS_NODISCARD inline Maybe<layout::BlockLayoutState::LineClamp>
      LineClamp(const RenderBlockFlow &rootRenderer) noexcept
    {
      // The legacy line clamp (-webkit-line-clamp) was dropped.
      auto *layoutState = rootRenderer.View().FrameView().LayoutContext().LayoutState();
      if (!layoutState)
      {
        return null;
      }
      if (auto lineClamp = layoutState->GetLineClamp())
      {
        return layout::BlockLayoutState::LineClamp {lineClamp->MaximumLines, lineClamp->ShouldDiscardOverflow,
                                                    false};
      }
      return null;
    }

    KRYS_NODISCARD float BaselineForEmptyContent(const RenderBlockFlow &rootRenderer) noexcept
    {
      auto *rootLayoutBox = rootRenderer.LayoutBox();
      if (!rootLayoutBox)
      {
        krys_debug_assert(false);
        return {};
      }

      auto &fontMetrics = rootRenderer.Style().MetricsOfPrimaryFont();
      auto ascent = layout::InlineFormattingUtils::SnapToInt(fontMetrics.Ascent(), *rootLayoutBox);
      auto descent = layout::InlineFormattingUtils::SnapToInt(fontMetrics.Descent(), *rootLayoutBox);
      auto baseline =
        ascent + (rootLayoutBox->FirstLineStyle().ComputedLineHeight() - (ascent + descent)) / 2;
      return layout::InlineFormattingUtils::SnapToInt(
        rootRenderer.BorderAndPaddingBefore().ToFloat() + baseline, *rootLayoutBox,
        layout::InlineFormattingUtils::SnapDirection::Floor);
    }

    // FIXME: SVG blocks other than foreign objects use legacy line layout unless the setting says otherwise,
    // once there's SVG.
    KRYS_NODISCARD bool CanUseForLineLayout(const RenderBlockFlow &) noexcept
    {
      return true;
    }

    KRYS_NODISCARD bool CanUseForPreferredWidthComputationImpl(const RenderBlockFlow &blockContainer) noexcept
    {
      for (auto walker = InlineWalker(blockContainer); !walker.AtEnd(); walker.Advance())
      {
        auto &renderer = *walker.Current();
        if (!renderer.IsInFlow())
        {
          return false;
        }

        auto isFullySupportedInFlowRenderer = renderer.IsRenderText() || Is<RenderLineBreak>(renderer)
                                              || Is<RenderInline>(renderer) || Is<RenderListMarker>(renderer);
        if (isFullySupportedInFlowRenderer)
        {
          continue;
        }

        auto &unsupportedRenderElement = Downcast<RenderElement>(renderer);
        if (!unsupportedRenderElement.GetWritingMode().IsHorizontal()
            || !unsupportedRenderElement.Style().LogicalWidth().IsFixed())
        {
          return false;
        }

        auto isNonSupportedFixedWidthContent = [&]
        {
          // FIXME: Implement this image special in line builder.
          auto allowImagesToBreak =
            !blockContainer.GetDocument().InQuirksMode() || !blockContainer.IsRenderTableCell();
          if (!allowImagesToBreak)
          {
            return true;
          }
          // FIXME: See RenderReplaced::ComputePreferredLogicalWidths where the minimum preferred logical
          // width is set to 0.
          auto isReplacedWithSpecialIntrinsicWidth = [&]
          {
            if (auto *renderReplaced = DynamicDowncast<RenderReplaced>(unsupportedRenderElement))
            {
              return renderReplaced->Style().LogicalMaxWidth().IsPercentOrCalculated();
            }
            return false;
          };
          return isReplacedWithSpecialIntrinsicWidth();
        };
        if (isNonSupportedFixedWidthContent())
        {
          return false;
        }
      }
      return true;
    }
  }

  LineLayout::LineLayout(RenderBlockFlow &flow) noexcept
      : _rootLayoutBox(ShareCheckedPtr(BoxTreeUpdater {flow}.Build().get())),
        _document(ShareCheckedPtr(&flow.GetDocument())),
        _layoutState(WeakPtr<layout::LayoutState>::WithRef(flow.View().GetLayoutState())),
        _blockFormattingState(GetLayoutState().EnsureBlockFormattingState(RootLayoutBox())),
        _inlineContentCache(GetLayoutState().GetInlineContentCache(RootLayoutBox())),
        _boxGeometryUpdater(flow.View().GetLayoutState(), RootLayoutBox())
  {
  }

  LineLayout::~LineLayout() noexcept
  {
    auto &rootRenderer = Flow();
    auto shouldPopulateBreakingPositionCache = [&]
    {
      auto mayHaveInvalidContent = !_inlineContent;
      if (_document->RenderTreeBeingDestroyed() || mayHaveInvalidContent)
      {
        return false;
      }
      return !_inlineContentCache.GetInlineItems().IsPopulatedFromCache();
    };
    if (shouldPopulateBreakingPositionCache())
    {
      layout::InlineItemsBuilder::PopulateBreakingPositionCache(
        _inlineContentCache.GetInlineItems().Content());
    }
    ClearInlineContent();
    GetLayoutState().DestroyInlineContentCache(RootLayoutBox());
    GetLayoutState().DestroyBlockFormattingState(RootLayoutBox());
    _boxGeometryUpdater.Clear();
    _rootLayoutBox = nullptr;

    BoxTreeUpdater {rootRenderer, *_document}.TearDown();
  }

  RenderBlockFlow *LineLayout::BlockContainer(const RenderObject &renderer) noexcept
  {
    // FIXME: Renderer replicas and scrollbar parts have their parent set but are not actually in the tree,
    // once there are any.
    for (auto *parent = renderer.Parent(); parent; parent = parent->Parent())
    {
      if (!parent->ChildrenInline())
      {
        return nullptr;
      }
      if (auto *renderBlockFlow = DynamicDowncast<RenderBlockFlow>(*parent))
      {
        return renderBlockFlow;
      }
    }

    return nullptr;
  }

  bool LineLayout::Contains(const RenderElement &renderer) const noexcept
  {
    if (!renderer.LayoutBox())
    {
      return false;
    }
    if (!renderer.LayoutBox()->IsInFormattingContextEstablishedBy(RootLayoutBox()))
    {
      return false;
    }
    return GetLayoutState().HasBoxGeometry(*renderer.LayoutBox());
  }

  LineLayout *LineLayout::Containing(RenderObject &renderer) noexcept
  {
    if (!renderer.IsInline())
    {
      // IFC may contain block level boxes (floats and out-of-flow boxes).
      // FIXME: SVG blocks and frame sets, once there are any.
      auto adjustedContainingBlock = [&]() -> RenderBlockFlow *
      {
        if (renderer.IsOutOfFlowPositioned())
        {
          // Here we are looking for the containing block as if the out-of-flow box was inflow (for static
          // position purpose).
          auto *containingBlock = renderer.Parent();
          if (Is<RenderInline>(containingBlock))
          {
            return DynamicDowncast<RenderBlockFlow>(containingBlock->ContainingBlock());
          }
          return DynamicDowncast<RenderBlockFlow>(containingBlock);
        }
        if (renderer.IsFloating())
        {
          // Note that ContainingBlock() on boxes in top layer (i.e. dialog) may return incorrect result
          // during style change even with not-yet-updated style.
          return DynamicDowncast<RenderBlockFlow>(RenderObject::ContainingBlockForPositionType(
            Downcast<RenderBox>(renderer).Style().GetPosition(), renderer));
        }
        if (auto *parentInlineBox = DynamicDowncast<RenderInline>(renderer.Parent()))
        {
          return DynamicDowncast<RenderBlockFlow>(parentInlineBox->ContainingBlock());
        }
        return nullptr;
      };
      if (auto *blockContainer = adjustedContainingBlock())
      {
        return blockContainer->InlineLayout();
      }
      return nullptr;
    }

    if (auto *container = BlockContainer(renderer))
    {
      return container->InlineLayout();
    }

    return nullptr;
  }

  const LineLayout *LineLayout::Containing(const RenderObject &renderer) noexcept
  {
    return Containing(const_cast<RenderObject &>(renderer));
  }

  bool LineLayout::CanUseFor(const RenderBlockFlow &flow) noexcept
  {
    return CanUseForLineLayout(flow);
  }

  bool LineLayout::CanUseForPreferredWidthComputation(const RenderBlockFlow &flow) noexcept
  {
    return CanUseForPreferredWidthComputationImpl(flow);
  }

  bool LineLayout::ShouldInvalidateLineLayoutAfterContentChange(const RenderBlockFlow &, const RenderObject &,
                                                                const LineLayout &) noexcept
  {
    // FIXME: Partial invalidation (shouldInvalidateLineLayoutAfterChangeFor), once there's partial layout.
    return true;
  }

  bool LineLayout::ShouldInvalidateLineLayoutAfterTreeMutation(const RenderBlockFlow &, const RenderObject &,
                                                               const LineLayout &, bool) noexcept
  {
    // FIXME: Partial invalidation (shouldInvalidateLineLayoutAfterChangeFor), once there's partial layout.
    return true;
  }

  void LineLayout::UpdateFormattingContexGeometries(LayoutUnit availableLogicalWidth) noexcept
  {
    _boxGeometryUpdater.SetFormattingContextRootGeometry(availableLogicalWidth);
    _inlineContentConstraints = _boxGeometryUpdater.FormattingContextConstraints(availableLogicalWidth);
    _boxGeometryUpdater.SetFormattingContextContentGeometry(
      _inlineContentConstraints->Horizontal().LogicalWidth, null);
  }

  void LineLayout::UpdateStyle(const RenderObject &renderer) noexcept
  {
    BoxTreeUpdater::UpdateStyle(renderer);
  }

  void LineLayout::UpdateOverflow() noexcept
  {
    if (!_inlineContent)
    {
      return;
    }
    InlineContentBuilder {Flow()}.UpdateLineOverflow(*_inlineContent);
  }

  std::pair<LayoutUnit, LayoutUnit> LineLayout::ComputeIntrinsicWidthConstraints() noexcept
  {
    auto parentBlockLayoutState = layout::BlockLayoutState {_blockFormattingState.GetPlacedFloats(), {}};
    auto inlineFormattingContext =
      layout::InlineFormattingContext {RootLayoutBox(), GetLayoutState(), parentBlockLayoutState};
    // FIXME: This is where we need to switch between minimum and maximum box geometries.
    // Currently we only support content where min == max.
    _boxGeometryUpdater.SetFormattingContextContentGeometry(null, layout::IntrinsicWidthMode::Minimum);
    return inlineFormattingContext.MinimumMaximumContentSize();
  }

  Maybe<LayoutRect> LineLayout::Layout(RenderBlockFlow::MarginInfo &marginInfo, ForceFullLayout) noexcept
  {
    PreparePlacedFloats();

    // FIXME: Partial layout should not rely on previous inline display content.
    ClearInlineContent();

    krys_debug_assert(_inlineContentConstraints.has_value());
    auto intrusiveInitialLetterBottom = [&]() -> Maybe<LayoutUnit>
    {
      if (auto lowestInitialLetterLogicalBottom = Flow().LowestInitialLetterLogicalBottom())
      {
        return *lowestInitialLetterLogicalBottom - _inlineContentConstraints->LogicalTop();
      }
      return null;
    };

    // FIXME: The trimmed sides of text-box-trim (TextBoxTrimmer), once there's text-box-trim.
    auto parentBlockLayoutState =
      layout::BlockLayoutState {_blockFormattingState.GetPlacedFloats(),
                                layout::IntegrationUtils::ToMarginState(marginInfo, {}),
                                LineClamp(Flow()),
                                {},
                                Flow().Style().GetTextBoxEdge(),
                                intrusiveInitialLetterBottom()};
    auto inlineFormattingContext =
      layout::InlineFormattingContext {RootLayoutBox(), GetLayoutState(), parentBlockLayoutState};
    // Temporary, integration only.
    inlineFormattingContext.GetLayoutState().SetNestedListMarkerOffsets(
      _boxGeometryUpdater.TakeNestedListMarkerOffsets());

    auto layoutResult = inlineFormattingContext.Layout(*_inlineContentConstraints);
    auto didDiscardContent = layoutResult.DidDiscardContent;
    (void)ConstructContent(inlineFormattingContext.GetLayoutState(), krys::move(layoutResult));

    // FIXME: Adjust the lines for pagination (adjustContentForPagination), once there's pagination.

    UpdateRenderTreePositions(inlineFormattingContext.GetLayoutState(), didDiscardContent);

    marginInfo = layout::IntegrationUtils::ToMarginInfo(parentBlockLayoutState.GetMarginState());

    if (!_inlineContent->HasPaintedInlineLevelBoxes())
    {
      // Nothing to repaint. If layout affects painted block level content those issue their own repaints.
      return LayoutRect {};
    }

    return null;
  }

  FloatRect LineLayout::ConstructContent(const layout::InlineLayoutState &inlineLayoutState,
                                         layout::InlineLayoutResult &&layoutResult) noexcept
  {
    auto damagedRect = InlineContentBuilder {Flow()}.Build(krys::move(layoutResult), EnsureInlineContent());

    _inlineContent->SetClearGapBeforeFirstLine(inlineLayoutState.ClearGapBeforeFirstLine());
    _inlineContent->SetClearGapAfterLastLine(inlineLayoutState.ClearGapAfterLastLine());
    _inlineContent->ShrinkToFit();

    _inlineContentCache.GetInlineItems().ShrinkToFit();
    _blockFormattingState.ShrinkToFit();

    // FIXME: These needs to be incorporated into the partial damage.
    auto offsetAndGaps = _inlineContent->FirstLinePaginationOffset() + _inlineContent->ClearBeforeAfterGaps();
    damagedRect.Expand(0.f, offsetAndGaps);
    return damagedRect;
  }

  void LineLayout::UpdateRenderTreePositions(const layout::InlineLayoutState &inlineLayoutState,
                                             bool didDiscardContent) noexcept
  {
    if (!_inlineContent && !didDiscardContent)
    {
      return;
    }

    auto &blockFlow = Flow();
    auto placedFloatsWritingMode =
      _blockFormattingState.GetPlacedFloats().BlockFormattingContextRoot().Style().GetWritingMode();

    if (_inlineContent)
    {
      for (auto &box : _inlineContent->DisplayContent().BoxList)
      {
        if (box.IsInlineBox() || box.IsText())
        {
          continue;
        }

        auto &layoutBox = box.LayoutBox();
        if (!layoutBox.IsAtomicInlineBox())
        {
          continue;
        }

        auto &renderer = Downcast<RenderBox>(*box.LayoutBox().RendererForIntegration());

        if (auto *layer = renderer.Layer())
        {
          layer->SetIsHiddenByOverflowTruncation(box.IsFullyTruncated());
        }

        renderer.SetLocation(LayoutPoint {box.VisualRectIgnoringBlockDirection().Location()});
      }
    }

    // FIXME: The floats' pagination offsets, once there's pagination.

    for (auto &layoutBox : layout::FormattingContextBoxes(RootLayoutBox()))
    {
      if (didDiscardContent)
      {
        layoutBox.RendererForIntegration()->ClearNeedsLayout();
      }

      if (!layoutBox.IsFloatingPositioned() && !layoutBox.IsOutOfFlowPositioned())
      {
        continue;
      }
      if (layoutBox.IsLineBreakBox())
      {
        continue;
      }
      auto &renderer = Downcast<RenderBox>(*layoutBox.RendererForIntegration());
      auto &logicalGeometry = GetLayoutState().GeometryForBox(layoutBox);

      if (layoutBox.IsFloatingPositioned())
      {
        // FIXME: Find out what to do with discarded (see line-clamp) floats in render tree.
        auto isInitialLetter = layoutBox.Style().GetPseudoElementType() == PseudoElementType::FirstLetter;
        auto &floatingObject = Flow().InsertFloatingBox(renderer);
        auto [marginBoxVisualRect, borderBoxVisualRect] =
          layout::IntegrationUtils::ToMarginAndBorderBoxVisualRect(
            logicalGeometry, _inlineContentConstraints->FormattingRootBorderBoxSize(),
            placedFloatsWritingMode);

        if (isInitialLetter)
        {
          auto firstLineTrim = LayoutUnit {inlineLayoutState.FirstLineStartTrimForInitialLetter()};
          marginBoxVisualRect.Move(0_lu, -firstLineTrim);
          borderBoxVisualRect.Move(0_lu, -firstLineTrim);
        }

        floatingObject.SetFrameRect(marginBoxVisualRect);
        floatingObject.SetMarginOffset({borderBoxVisualRect.X() - marginBoxVisualRect.X(),
                                        borderBoxVisualRect.Y() - marginBoxVisualRect.Y()});
        floatingObject.SetIsPlaced(true);

        // FIXME: Repaint the float if it moved, once there's painting.
        renderer.SetLocation(borderBoxVisualRect.Location());
        continue;
      }

      if (layoutBox.IsOutOfFlowPositioned())
      {
        krys_debug_assert(renderer.Layer());
        auto &layer = *renderer.Layer();
        auto borderBoxLogicalTopLeft = layout::BoxGeometry::BorderBoxTopLeft(logicalGeometry);
        auto previousStaticPosition = LayoutPoint {layer.StaticInlinePosition(), layer.StaticBlockPosition()};
        auto delta = borderBoxLogicalTopLeft - previousStaticPosition;
        auto hasStaticInlinePositioning =
          layoutBox.Style().HasStaticInlinePosition(renderer.IsHorizontalWritingMode());

        if (layoutBox.Style().IsOriginalDisplayInlineType())
        {
          blockFlow.SetStaticInlinePositionForChild(renderer, borderBoxLogicalTopLeft.X());
          if (hasStaticInlinePositioning)
          {
            renderer.Move(delta.Width(), delta.Height());
          }
        }

        layer.SetStaticBlockPosition(borderBoxLogicalTopLeft.Y());
        layer.SetStaticInlinePosition(borderBoxLogicalTopLeft.X());
        continue;
      }
    }
  }

  void LineLayout::PreparePlacedFloats() noexcept
  {
    auto &placedFloats = _blockFormattingState.GetPlacedFloats();
    placedFloats.Clear();

    if (!Flow().ContainsFloats())
    {
      return;
    }

    auto placedFloatsWritingMode = placedFloats.BlockFormattingContextRoot().Style().GetWritingMode();
    auto placedFloatsIsLeftToRight = placedFloatsWritingMode.IsLogicalLeftInlineStart();
    auto isHorizontalWritingMode = placedFloatsWritingMode.IsHorizontal();
    for (auto &floatingObject : *Flow().GetFloatingObjectSet())
    {
      if (!floatingObject->Renderer())
      {
        continue;
      }

      auto &visualRect = floatingObject->FrameRect();

      auto usedPosition = RenderStyle::GetUsedFloat(*floatingObject->Renderer());
      auto logicalPosition = (usedPosition == UsedFloat::Left) == placedFloatsIsLeftToRight
                               ? layout::PlacedFloats::Item::Position::Start
                               : layout::PlacedFloats::Item::Position::End;

      auto boxGeometry = layout::BoxGeometry {};
      auto logicalRect = [&]
      {
        // FIXME: We are flooring here for legacy compatibility. See
        // FloatingObjects::IntervalForFloatingObject and RenderBlockFlow::ClearFloats.
        auto logicalTop = isHorizontalWritingMode ? LayoutUnit(visualRect.Y().Floor()) : visualRect.X();
        auto logicalLeft = isHorizontalWritingMode ? visualRect.X() : LayoutUnit(visualRect.Y().Floor());
        auto logicalHeight =
          (isHorizontalWritingMode ? LayoutUnit(visualRect.MaxY().Floor()) : visualRect.MaxX()) - logicalTop;
        auto logicalWidth =
          (isHorizontalWritingMode ? visualRect.MaxX() : LayoutUnit(visualRect.MaxY().Floor())) - logicalLeft;
        if (!placedFloatsIsLeftToRight)
        {
          auto rootBorderBoxWidth = _inlineContentConstraints->VisualLeft()
                                    + _inlineContentConstraints->Horizontal().LogicalWidth
                                    + _inlineContentConstraints->Horizontal().LogicalLeft;
          logicalLeft = rootBorderBoxWidth - (logicalLeft + logicalWidth);
        }
        return LayoutRect {logicalLeft, logicalTop, logicalWidth, logicalHeight};
      }();

      boxGeometry.SetTopLeft(logicalRect.Location());
      boxGeometry.SetContentBoxWidth(logicalRect.Width());
      boxGeometry.SetContentBoxHeight(logicalRect.Height());
      boxGeometry.SetBorder({});
      boxGeometry.SetPadding({});
      boxGeometry.SetHorizontalMargin({});
      boxGeometry.SetVerticalMargin({});

      auto *shapeOutsideInfo = floatingObject->Renderer()->GetShapeOutsideInfo();
      auto shape = shapeOutsideInfo ? ShareRefPtr(&shapeOutsideInfo->ComputedShape()) : nullptr;

      placedFloats.Add({logicalPosition, boxGeometry, logicalRect.Location(), krys::move(shape)});
    }
  }

  bool LineLayout::IsPaginated() const noexcept
  {
    return _inlineContent && _inlineContent->IsPaginated();
  }

  bool LineLayout::HasEllipsisInBlockDirectionOnLastFormattedLine() const noexcept
  {
    if (!_inlineContent)
    {
      return false;
    }

    for (auto &line : _inlineContent->DisplayContent().LineList | std::views::reverse)
    {
      if (!line.HasContentfulInFlowBox())
      {
        // Out-of-flow content could initiate a line with no inline content.
        continue;
      }
      auto lastFormattedLineEllipsis = line.GetEllipsis();
      return lastFormattedLineEllipsis
             && lastFormattedLineEllipsis->EllipsisType == inline_display::Line::Ellipsis::Type::Block;
    }
    return false;
  }

  Maybe<LayoutUnit> LineLayout::ClampedContentLogicalHeight() const noexcept
  {
    if (!_inlineContent)
    {
      return null;
    }

    auto &lines = _inlineContent->DisplayContent().LineList;
    if (lines.empty())
    {
      // Out-of-flow only content (and/or with floats) may produce blank inline content.
      return null;
    }

    auto firstTruncatedLineIndex = [&]() -> Maybe<size_t>
    {
      for (size_t lineIndex = 0; lineIndex < lines.size(); ++lineIndex)
      {
        if (lines[lineIndex].IsFullyTruncatedInBlockDirection())
        {
          return lineIndex;
        }
      }
      return null;
    }();
    if (!firstTruncatedLineIndex)
    {
      return null;
    }
    if (!*firstTruncatedLineIndex)
    {
      // This content is fully truncated in the block direction.
      return LayoutUnit {};
    }

    auto contentHeight = lines[*firstTruncatedLineIndex - 1].LineBoxLogicalRect().MaxY()
                         - lines.front().LineBoxLogicalRect().Y();
    auto offsetAndGaps = _inlineContent->FirstLinePaginationOffset() + _inlineContent->ClearBeforeAfterGaps();
    return LayoutUnit {contentHeight + offsetAndGaps};
  }

  LayoutUnit LineLayout::ContentLogicalHeight() const noexcept
  {
    if (!_inlineContent)
    {
      return {};
    }

    auto &lines = _inlineContent->DisplayContent().LineList;
    if (lines.empty())
    {
      // Out-of-flow only content (and/or with floats) may produce blank inline content.
      return {};
    }

    auto contentHeight =
      LastLineWithInflowContent(lines).LineBoxLogicalRect().MaxY() - lines.front().LineBoxLogicalRect().Y();
    auto offsetAndGaps = _inlineContent->FirstLinePaginationOffset() + _inlineContent->ClearBeforeAfterGaps();
    return LayoutUnit {contentHeight + offsetAndGaps};
  }

  bool LineLayout::IsSelfCollapsingContent() const noexcept
  {
    if (!_inlineContent || !_inlineContent->HasContentfulInFlowBox())
    {
      return true;
    }

    auto &displayContent = _inlineContent->DisplayContent();
    for (auto &line : displayContent.LineList)
    {
      if (line.HasContentfulInlineLevelBox())
      {
        return false;
      }

      if (line.HasBlockLevelBox())
      {
        auto blockLevelBox = [&]() -> RenderBox *
        {
          for (auto index = line.FirstBoxIndex(); index < line.LastBoxIndex(); ++index)
          {
            if (displayContent.BoxList[index].IsBlockLevelBox())
            {
              return DynamicDowncast<RenderBox>(
                displayContent.BoxList[index].LayoutBox().RendererForIntegration());
            }
            krys_debug_assert(displayContent.BoxList[index].IsInlineBox());
          }
          return nullptr;
        };
        if (auto *renderBox = blockLevelBox(); renderBox && !renderBox->IsSelfCollapsingBlock())
        {
          return false;
        }
      }
    }
    return true;
  }

  bool LineLayout::HasContentfulInlineOrBlockLine() const noexcept
  {
    return _inlineContent && _inlineContent->HasContentfulInFlowBox();
  }

  bool LineLayout::HasContentfulInlineLine() const noexcept
  {
    return _inlineContent && _inlineContent->HasContentfulInlineLevelBox();
  }

  size_t LineLayout::LineCount() const noexcept
  {
    if (!_inlineContent)
    {
      return 0;
    }
    if (!_inlineContent->HasContentfulInFlowBox())
    {
      return 0;
    }

    auto &lines = _inlineContent->DisplayContent().LineList;
    if (lines.empty())
    {
      return 0;
    }
    // In some cases (trailing out-of-flow, non-contentful content after <br>) we produce last line with no
    // content but root inline box only.
    return lines.back().HasContentfulInFlowBox() ? lines.size() : lines.size() - 1;
  }

  bool LineLayout::HasInkOverflow() const noexcept
  {
    return _inlineContent && _inlineContent->HasInkOverflow();
  }

  Maybe<LayoutUnit> LineLayout::FirstLineBaseline() const noexcept
  {
    if (!_inlineContent)
    {
      // FIXME: We should really assert here but table code asks for baseline even before running layout.
      // and we get here by having a LineLayout object initiated by preferred with computation.
      return null;
    }

    auto baselineForLineOrBlock = [&](const inline_display::Line &line) -> Maybe<LayoutUnit>
    {
      if (!line.HasContentfulInFlowBox())
      {
        return null;
      }

      if (auto *blockLevelBox = _inlineContent->BlockLevelBoxForLine(line))
      {
        // For block-in-inline look for the baseline of the child box.
        auto &blockRenderer = Downcast<RenderBox>(*blockLevelBox->LayoutBox().RendererForIntegration());
        return blockRenderer.FirstLineBaseline();
      }
      return LayoutUnit {
        layout::InlineFormattingUtils::SnapToInt(BaselineForLine(line).ToFloat(), RootLayoutBox(),
                                                 layout::InlineFormattingUtils::SnapDirection::Floor)};
    };

    for (auto &line : _inlineContent->DisplayContent().LineList)
    {
      if (auto baseline = baselineForLineOrBlock(line))
      {
        return baseline;
      }
    }

    return Flow().HasLineIfEmpty() ? Maybe<LayoutUnit> {LayoutUnit {BaselineForEmptyContent(Flow())}} : null;
  }

  Maybe<LayoutUnit> LineLayout::LastLineBaseline() const noexcept
  {
    if (!_inlineContent)
    {
      // FIXME: We should really assert here but table code asks for baseline even before running layout.
      // and we get here by having a LineLayout object initiated by preferred with computation.
      return null;
    }

    auto baselineForLineOrBlock = [&](const inline_display::Line &line) -> Maybe<LayoutUnit>
    {
      if (!line.HasContentfulInFlowBox())
      {
        return null;
      }

      if (auto *blockLevelBox = _inlineContent->BlockLevelBoxForLine(line))
      {
        // For block-in-inline look for the baseline of the child box.
        auto &blockRenderer = Downcast<RenderBox>(*blockLevelBox->LayoutBox().RendererForIntegration());
        return blockRenderer.LastLineBaseline();
      }
      return LayoutUnit {
        layout::InlineFormattingUtils::SnapToInt(BaselineForLine(line).ToFloat(), RootLayoutBox(),
                                                 layout::InlineFormattingUtils::SnapDirection::Floor)};
    };

    for (auto &line : _inlineContent->DisplayContent().LineList | std::views::reverse)
    {
      if (auto baseline = baselineForLineOrBlock(line))
      {
        return baseline;
      }
    }

    return Flow().HasLineIfEmpty() ? Maybe<LayoutUnit> {LayoutUnit {BaselineForEmptyContent(Flow())}} : null;
  }

  LayoutUnit LineLayout::BaselineForLine(const inline_display::Line &line) const noexcept
  {
    auto rootWritingMode = RootLayoutBox().GetWritingMode();
    auto baseline = line.Baseline();
    if (rootWritingMode.IsLineInverted())
    {
      baseline = line.LineBoxLogicalRect().Height() - baseline;
    }

    switch (rootWritingMode.BlockDirection())
    {
      case FlowDirection::TopToBottom:
      case FlowDirection::BottomToTop: return LayoutUnit {line.LineBoxTop() + baseline};
      case FlowDirection::LeftToRight:
      case FlowDirection::RightToLeft: return LayoutUnit {line.LineBoxLeft() + baseline};
    }
    krys_debug_assert(false);
    return {};
  }

  void LineLayout::CollectOverflow() noexcept
  {
    if (!_inlineContent)
    {
      return;
    }

    Flow().AddLayoutOverflow(LayoutRect {_inlineContent->ScrollableOverflow()});
    if (!Flow().HasNonVisibleOverflow())
    {
      Flow().AddVisualOverflow(LayoutRect {_inlineContent->InkOverflow()});
    }
  }

  InlineContent &LineLayout::EnsureInlineContent() noexcept
  {
    if (!_inlineContent)
    {
      _inlineContent = CreateUnique<InlineContent>(Flow());
    }
    return *_inlineContent;
  }

  inline_iterator::TextBoxIterator LineLayout::TextBoxesFor(const RenderText &renderText) const noexcept
  {
    if (!_inlineContent)
    {
      return {};
    }

    auto &layoutBox = *renderText.LayoutBox();
    auto firstIndex = _inlineContent->FirstBoxIndexForLayoutBox(layoutBox);
    if (!firstIndex)
    {
      return {};
    }

    return inline_iterator::TextBoxFor(*_inlineContent, *firstIndex);
  }

  inline_iterator::LeafBoxIterator LineLayout::BoxFor(const RenderElement &renderElement) const noexcept
  {
    if (!_inlineContent)
    {
      return {};
    }

    auto &layoutBox = *renderElement.LayoutBox();
    auto firstIndex = _inlineContent->FirstBoxIndexForLayoutBox(layoutBox);
    if (!firstIndex)
    {
      return {};
    }

    return inline_iterator::BoxFor(*_inlineContent, *firstIndex);
  }

  inline_iterator::InlineBoxIterator
    LineLayout::FirstInlineBoxFor(const RenderInline &renderInline) const noexcept
  {
    if (!_inlineContent)
    {
      return {};
    }

    auto &layoutBox = *renderInline.LayoutBox();
    auto *box = _inlineContent->FirstBoxForLayoutBox(layoutBox);
    if (!box)
    {
      return {};
    }

    return inline_iterator::InlineBoxFor(*_inlineContent, *box);
  }

  inline_iterator::InlineBoxIterator LineLayout::FirstRootInlineBox() const noexcept
  {
    if (!_inlineContent || !_inlineContent->HasContentfulInFlowBox())
    {
      return {};
    }

    return inline_iterator::InlineBoxFor(*_inlineContent, _inlineContent->DisplayContent().BoxList[0]);
  }

  inline_iterator::LineBoxIterator LineLayout::FirstLineBox() const noexcept
  {
    if (!_inlineContent || !_inlineContent->HasContentfulInFlowBox())
    {
      return {};
    }

    return inline_iterator::LineBoxIterator {inline_iterator::LineBoxIteratorModernPath {*_inlineContent, 0}};
  }

  inline_iterator::LineBoxIterator LineLayout::LastLineBox() const noexcept
  {
    if (!_inlineContent || !_inlineContent->HasContentfulInFlowBox())
    {
      return {};
    }

    auto &lines = _inlineContent->DisplayContent().LineList;
    return inline_iterator::LineBoxIterator {
      inline_iterator::LineBoxIteratorModernPath {*_inlineContent, lines.empty() ? 0 : lines.size() - 1}};
  }

  LayoutRect LineLayout::FirstInlineBoxRect(const RenderInline &renderInline) const noexcept
  {
    if (!_inlineContent)
    {
      return {};
    }

    auto &layoutBox = *renderInline.LayoutBox();
    auto *firstBox = _inlineContent->FirstBoxForLayoutBox(layoutBox);
    if (!firstBox)
    {
      return {};
    }

    // FIXME: We should be able to flip the display boxes soon after the root block
    // is finished sizing in one go.
    auto firstBoxRect = LayoutRect {firstBox->VisualRectIgnoringBlockDirection()};
    switch (RootLayoutBox().GetWritingMode().BlockDirection())
    {
      case FlowDirection::TopToBottom:
      case FlowDirection::BottomToTop:
      case FlowDirection::LeftToRight: return firstBoxRect;
      case FlowDirection::RightToLeft:
        firstBoxRect.SetX(Flow().Width() - firstBoxRect.MaxX());
        return firstBoxRect;
    }
    krys_debug_assert(false);
    return firstBoxRect;
  }

  LayoutRect LineLayout::EnclosingBorderBoxRectFor(const RenderInline &renderInline) const noexcept
  {
    if (!_inlineContent)
    {
      return {};
    }

    // FIXME: This preserves existing output.
    if (!_inlineContent->HasContentfulInFlowBox())
    {
      return {};
    }

    auto borderBoxLogicalRect = LayoutRect {
      layout::BoxGeometry::BorderBoxRect(GetLayoutState().GeometryForBox(*renderInline.LayoutBox()))};
    return Flow().GetWritingMode().IsHorizontal() ? borderBoxLogicalRect
                                                  : borderBoxLogicalRect.TransposedRect();
  }

  LayoutRect LineLayout::InkOverflowBoundingBoxRectFor(const RenderInline &renderInline) const noexcept
  {
    if (!_inlineContent)
    {
      return {};
    }

    auto &layoutBox = *renderInline.LayoutBox();

    LayoutRect result;
    _inlineContent->TraverseNonRootInlineBoxes(layoutBox, [&](const inline_display::Box &inlineBox)
                                               { result.Unite(LayoutRect {inlineBox.InkOverflow()}); });

    return result;
  }

  List<FloatRect> LineLayout::CollectInlineBoxRects(const RenderInline &renderInline) const noexcept
  {
    if (!_inlineContent)
    {
      return {};
    }

    auto &layoutBox = *renderInline.LayoutBox();

    List<FloatRect> result;
    _inlineContent->TraverseNonRootInlineBoxes(
      layoutBox, [&](const inline_display::Box &inlineBox)
      { result.push_back(inlineBox.VisualRectIgnoringBlockDirection()); });

    return result;
  }

  void LineLayout::ShiftLinesBy(LayoutUnit blockShift) noexcept
  {
    if (!_inlineContent)
    {
      return;
    }
    bool isHorizontalWritingMode = Flow().GetWritingMode().IsHorizontal();

    for (auto &line : _inlineContent->DisplayContent().LineList)
    {
      line.MoveInBlockDirection(blockShift.ToFloat());
    }

    auto deltaX = isHorizontalWritingMode ? 0_lu : blockShift;
    auto deltaY = isHorizontalWritingMode ? blockShift : 0_lu;
    for (auto &box : _inlineContent->DisplayContent().BoxList)
    {
      if (isHorizontalWritingMode)
      {
        box.MoveVertically(blockShift.ToFloat());
      }
      else
      {
        box.MoveHorizontally(blockShift.ToFloat());
      }

      if (box.IsAtomicInlineBox())
      {
        auto &renderer = Downcast<RenderBox>(*box.LayoutBox().RendererForIntegration());
        renderer.Move(deltaX, deltaY);
      }
    }

    for (auto &layoutBox : layout::FormattingContextBoxes(RootLayoutBox()))
    {
      if (layoutBox.IsOutOfFlowPositioned()
          && layoutBox.Style().HasStaticBlockPosition(isHorizontalWritingMode))
      {
        if (auto *layerRenderer = DynamicDowncast<RenderLayerModelObject>(layoutBox.RendererForIntegration()))
        {
          if (auto *layer = layerRenderer->Layer())
          {
            layer->SetStaticBlockPosition(layer->StaticBlockPosition() + blockShift);
          }
        }
        if (auto *renderBox = DynamicDowncast<RenderBox>(layoutBox.RendererForIntegration()))
        {
          renderBox->Move(deltaX, deltaY);
        }
      }
    }
  }

  const RenderBlockFlow &LineLayout::Flow() const noexcept
  {
    return Downcast<RenderBlockFlow>(*_rootLayoutBox->RendererForIntegration());
  }

  RenderBlockFlow &LineLayout::Flow() noexcept
  {
    return Downcast<RenderBlockFlow>(*_rootLayoutBox->RendererForIntegration());
  }

  void LineLayout::ReleaseCaches(RenderView &view) noexcept
  {
    for (auto &renderer : DescendantsOfType<RenderBlockFlow>(view))
    {
      if (auto *lineLayout = renderer.InlineLayout())
      {
        lineLayout->ReleaseCachesAndResetDamage();
      }
    }
  }

  void LineLayout::ReleaseCachesAndResetDamage() noexcept
  {
    _inlineContentCache.GetInlineItems().Content().clear();
    if (_inlineContent)
    {
      _inlineContent->ReleaseCaches();
    }
  }

  void LineLayout::ClearInlineContent() noexcept
  {
    _inlineContent = nullptr;
  }

  bool LineLayout::ContentNeedsVisualReordering() const noexcept
  {
    return _inlineContentCache.GetInlineItems().RequiresVisualReordering();
  }

  bool LineLayout::HasBlocks() const noexcept
  {
    return _inlineContent && _inlineContent->HasBlockLevelBoxes();
  }
}
