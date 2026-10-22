#include "Krystal.Booey/Rendering/RenderBlockFlow.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Position.hpp"
#include "Krystal.Booey/Editing/Editing.hpp"
#include "Krystal.Booey/Editing/EditingBehavior.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/Text/TextUtil.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/InlineIteratorLogicalOrderTraversal.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/LayoutIntegrationLineLayout.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/LineSelection.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Page/LocalFrameViewLayoutContext.hpp"
#include "Krystal.Booey/Page/Settings.hpp"
#include "Krystal.Booey/Rendering/BlockStepSizing.hpp"
#include "Krystal.Booey/Rendering/HitTestResult.hpp"
#include "Krystal.Booey/Rendering/HitTestSource.hpp"
#include "Krystal.Booey/Rendering/InlineWalker.hpp"
#include "Krystal.Booey/Rendering/LayoutRepainter.hpp"
#include "Krystal.Booey/Rendering/LineClampUpdater.hpp"
#include "Krystal.Booey/Rendering/PaintInfo.hpp"
#include "Krystal.Booey/Rendering/RenderAncestorIterator.hpp"
#include "Krystal.Booey/Rendering/RenderChildIterator.hpp"
#include "Krystal.Booey/Rendering/RenderDescendantIterator.hpp"
#include "Krystal.Booey/Rendering/RenderInline.hpp"
#include "Krystal.Booey/Rendering/RenderIterator.hpp"
#include "Krystal.Booey/Rendering/RenderLayer.hpp"
#include "Krystal.Booey/Rendering/RenderLayoutState.hpp"
#include "Krystal.Booey/Rendering/RenderLineBreak.hpp"
#include "Krystal.Booey/Rendering/RenderTableCell.hpp"
#include "Krystal.Booey/Rendering/RenderText.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/Rendering/Shapes/ShapeOutsideInfo.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+ContainmentInlines.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/TextBoxTrimmer.hpp"
#include "Krystal.Core/Text/Unicode.hpp"

namespace krys::boo
{
  bool RenderBlockFlow::s_canPropagateFloatIntoSibling = false;

  namespace
  {

    KRYS_NODISCARD bool IsSkippedContentRootOrSkippedContent(const RenderBlockFlow &blockFlow) noexcept
    {
      auto *element = blockFlow.GetElement();
      return (element && IsSkippedContentRoot(blockFlow.Style(), *element)) || blockFlow.IsSkippedContent();
    }
  }

  RenderBlockFlowRareData::RenderBlockFlowRareData(const RenderBlockFlow &block) noexcept
      : Margins(PositiveMarginBeforeDefault(block), NegativeMarginBeforeDefault(block),
                PositiveMarginAfterDefault(block), NegativeMarginAfterDefault(block))
  {
  }

  // Our MarginInfo state used when laying out block children.
  RenderBlockFlow::MarginInfo::MarginInfo(
    const RenderBlockFlow &block, IgnoreScrollbarForAfterMargin ignoreScrollbarForAfterMargin) noexcept
  {
    const auto &blockStyle = block.Style();
    krys_debug_assert(block.IsRenderView() || block.Parent());

    _canCollapseWithChildren = !block.CreatesNewFormattingContext() && !block.IsRenderView();

    _canCollapseMarginBeforeWithChildren = _canCollapseWithChildren && !block.BorderAndPaddingBefore();

    // If any height other than auto is specified in CSS, then we don't collapse our bottom
    // margins with our children's margins. To do otherwise would be to risk odd visual
    // effects when the children overflow out of the parent block and yet still collapse
    // with it. We also don't collapse if we have any bottom border/padding.
    auto canCollapseMarginAfterWithChildren = [&]() -> bool
    {
      if (!_canCollapseWithChildren)
      {
        return false;
      }
      if (!blockStyle.LogicalHeight().IsAuto())
      {
        return false;
      }
      if (block.BorderAndPaddingAfter())
      {
        return false;
      }
      // FIXME: Check if all callsites are supposed to take scrollbar into account here.
      return ignoreScrollbarForAfterMargin == IgnoreScrollbarForAfterMargin::Yes
               ? true
               : !block.ScrollbarLogicalHeight();
    };
    _canCollapseMarginAfterWithChildren = canCollapseMarginAfterWithChildren();

    _quirkContainer = block.IsRenderTableCell() || block.IsBody();

    _positiveMargin = _canCollapseMarginBeforeWithChildren ? block.MaxPositiveMarginBefore() : 0_lu;
    _negativeMargin = _canCollapseMarginBeforeWithChildren ? block.MaxNegativeMarginBefore() : 0_lu;
  }

  RenderBlockFlow::MarginInfo::MarginInfo(bool canCollapseWithChildren,
                                          bool canCollapseMarginBeforeWithChildren,
                                          bool canCollapseMarginAfterWithChildren, bool quirkContainer,
                                          bool atBeforeSideOfBlock, bool atAfterSideOfBlock,
                                          bool hasMarginBeforeQuirk, bool hasMarginAfterQuirk,
                                          bool determinedMarginBeforeQuirk, LayoutUnit positiveMargin,
                                          LayoutUnit negativeMargin) noexcept
      : _canCollapseWithChildren(canCollapseWithChildren),
        _canCollapseMarginBeforeWithChildren(canCollapseMarginBeforeWithChildren),
        _canCollapseMarginAfterWithChildren(canCollapseMarginAfterWithChildren),
        _quirkContainer(quirkContainer), _atBeforeSideOfBlock(atBeforeSideOfBlock),
        _atAfterSideOfBlock(atAfterSideOfBlock), _hasMarginBeforeQuirk(hasMarginBeforeQuirk),
        _hasMarginAfterQuirk(hasMarginAfterQuirk), _determinedMarginBeforeQuirk(determinedMarginBeforeQuirk),
        _positiveMargin(positiveMargin), _negativeMargin(negativeMargin)
  {
  }

  RenderBlockFlow::RenderBlockFlow(Type type, dom::Element &element, RenderStyle &&style,
                                   EnumSet<BlockFlowFlag> flags) noexcept
      : RenderBlock(type, element, krys::move(style), {}, flags)
  {
    krys_debug_assert(IsRenderBlockFlow());
    SetChildrenInline(true);
  }

  RenderBlockFlow::RenderBlockFlow(Type type, dom::Document &document, RenderStyle &&style,
                                   EnumSet<BlockFlowFlag> flags) noexcept
      : RenderBlock(type, document, krys::move(style), {}, flags)
  {
    krys_debug_assert(IsRenderBlockFlow());
    SetChildrenInline(true);
  }

  // Do not add any code in below destructor. Add it to willBeDestroyed() instead.
  RenderBlockFlow::~RenderBlockFlow() noexcept = default;

  RenderBlockFlow *RenderBlockFlow::PreviousSiblingWithOverhangingFloats(bool &parentHasFloats) const noexcept
  {
    // Attempt to locate a previous sibling with overhanging floats. We skip any elements that are
    // out of flow (like floating/positioned elements), and we also skip over any objects that may have
    // shifted to avoid floats.
    parentHasFloats = false;
    for (RenderObject *sibling = PreviousSibling(); sibling; sibling = sibling->PreviousSibling())
    {
      if (auto *siblingBlock = DynamicDowncast<RenderBlockFlow>(*sibling))
      {
        if (!siblingBlock->AvoidsFloats())
        {
          return siblingBlock;
        }
      }
      if (sibling->IsFloating())
      {
        parentHasFloats = true;
      }
    }
    return nullptr;
  }

  void RenderBlockFlow::RebuildFloatingObjectSetFromIntrudingFloats() noexcept
  {
    // FIXME: Content skipped by content-visibility (isSkippedContentRootForLayout, the floating objects that
    // may be stale), once layout skips content.

    HashSet<RenderBox *> oldIntrudingFloatSet;

    if (_floatingObjects)
    {
      _floatingObjects->SetHorizontalWritingMode(IsHorizontalWritingMode());
      if (!ChildrenInline())
      {
        for (const auto &floatingObject : _floatingObjects->Set())
        {
          if (!floatingObject->Renderer())
          {
            continue;
          }
          if (!floatingObject->IsDescendant())
          {
            oldIntrudingFloatSet.insert(floatingObject->Renderer());
          }
        }
      }
      _floatingObjects->Clear();
    }

    // Inline blocks are covered by the isBlockLevelReplacedOrAtomicInline() check in the avoidFloats method.
    if (AvoidsFloats() || IsDocumentElementRenderer() || IsRenderView() || IsFloatingOrOutOfFlowPositioned()
        || IsRenderTableCell())
    {
      if (!oldIntrudingFloatSet.empty())
      {
        MarkAllDescendantsWithFloatsForLayout();
      }
      return;
    }

    auto parentBlock = [&]() -> RenderBlockFlow *
    {
      if (auto *blockFlowParent = DynamicDowncast<RenderBlockFlow>(Parent()))
      {
        return blockFlowParent;
      }
      // FIXME: The containing block of an inline box parent, when blocks are in inline layout
      // (blocksInInlineLayoutEnabled), once there's inline layout.
      // We should not process floats if the parent node is not a RenderBlock. Otherwise, we will add
      // floats in an invalid context. This will cause a crash arising from a bad cast on the parent.
      // See <rdar://problem/8049753>, where float property is applied on a text node in a SVG.
      return nullptr;
    }();
    if (!parentBlock)
    {
      return;
    }

    // First add in floats from the parent. Self-collapsing blocks let their parent track any floats that
    // intrude into them (as opposed to floats they contain themselves) so check for those here too. If margin
    // collapsing has moved us up past the top a previous sibling then we need to check for floats from the
    // parent too.
    bool parentHasFloats = false;
    RenderBlockFlow *previousBlock = PreviousSiblingWithOverhangingFloats(parentHasFloats);
    LayoutUnit logicalTopOffset = LogicalTop();
    bool parentHasIntrudingFloats = !parentHasFloats
                                    && (!previousBlock || previousBlock->IsSelfCollapsingBlock()
                                        || previousBlock->LogicalTop() > logicalTopOffset)
                                    && parentBlock->LowestFloatLogicalBottom() > logicalTopOffset;
    if (parentHasFloats || parentHasIntrudingFloats)
    {
      AddIntrudingFloats(parentBlock, parentBlock, parentBlock->LogicalLeftOffsetForContent(),
                         logicalTopOffset);
    }

    // Add overhanging floats from the previous RenderBlock, but only if it has a float that intrudes into our
    // space.
    if (previousBlock)
    {
      logicalTopOffset -= previousBlock->LogicalTop();
      if (previousBlock->LowestFloatLogicalBottom() > logicalTopOffset)
      {
        AddIntrudingFloats(previousBlock, parentBlock, 0_lu, logicalTopOffset);
      }
    }

    if (!ChildrenInline() && !oldIntrudingFloatSet.empty())
    {
      // If there are previously intruding floats that no longer intrude, then children with floats
      // should also get layout because they might need their floating object lists cleared.
      if (_floatingObjects->Set().size() < oldIntrudingFloatSet.size())
      {
        MarkAllDescendantsWithFloatsForLayout();
      }
      else
      {
        for (const auto &floatingObject : _floatingObjects->Set())
        {
          if (!floatingObject->Renderer())
          {
            continue;
          }
          oldIntrudingFloatSet.erase(floatingObject->Renderer());
          if (oldIntrudingFloatSet.empty())
          {
            break;
          }
        }
        if (!oldIntrudingFloatSet.empty())
        {
          MarkAllDescendantsWithFloatsForLayout();
        }
      }
    }
  }

  void RenderBlockFlow::ComputeIntrinsicLogicalWidths(LayoutUnit &minLogicalWidth,
                                                      LayoutUnit &maxLogicalWidth) const noexcept
  {
    if (ShouldApplySizeOrInlineSizeContainment())
    {
      if (auto width = ExplicitIntrinsicInnerLogicalWidth())
      {
        minLogicalWidth = *width;
        maxLogicalWidth = *width;
      }
    }
    else if (ChildrenInline())
    {
      ComputeInlinePreferredLogicalWidths(minLogicalWidth, maxLogicalWidth);
    }
    else
    {
      ComputeBlockPreferredLogicalWidths(minLogicalWidth, maxLogicalWidth);
    }

    maxLogicalWidth = std::max(minLogicalWidth, maxLogicalWidth);

    // FIXME: Multi-column widths (adjustIntrinsicLogicalWidthsForColumns) and horizontal marquees, once there
    // are columns and marquees.

    if (auto *cell = DynamicDowncast<RenderTableCell>(*this))
    {
      auto [tableCellWidth, usedZoom] = cell->StyleOrColLogicalWidth();
      if (auto fixedTableCellWidth = tableCellWidth.TryFixed();
          fixedTableCellWidth && fixedTableCellWidth->IsPositive())
      {
        maxLogicalWidth =
          std::max(minLogicalWidth, AdjustContentBoxLogicalWidthForBoxSizing(*fixedTableCellWidth));
      }
    }

    auto scrollbarWidth = LayoutUnit(IntrinsicScrollbarLogicalWidthIncludingGutter());
    maxLogicalWidth += scrollbarWidth;
    minLogicalWidth += scrollbarWidth;
  }

  bool RenderBlockFlow::RecomputeLogicalWidthAndColumnWidth() noexcept
  {
    // FIXME: The column count and width (computeColumnCountAndWidth), once there's multi-column layout.
    return RecomputeLogicalWidth();
  }

  void RenderBlockFlow::LayoutBlockWithNoChildren() noexcept
  {
    krys_debug_assert(!FirstChild());

    // Empty block containers produce empty formatting lines which may affect trim-start/end.
    auto textBoxTrimmer = TextBoxTrimmer {*this};
    auto repainter = LayoutRepainter {*this};

    // FIXME: Instead of taking floats from previous sibling and forwarding them to next unconditionally, we
    // should completely skip these empty block containers.
    RebuildFloatingObjectSetFromIntrudingFloats();

    auto computeInlineAxisSize = [&]
    {
      UpdateLogicalWidth();
    };
    computeInlineAxisSize();

    auto computeBlockAxisSize = [&]
    {
      const auto &style = this->Style();

      if (!IsRenderTableCell())
      {
        InitMaxMarginValues();
        SetHasMarginBeforeQuirk(style.MarginBefore().HasQuirk());
        SetHasMarginAfterQuirk(style.MarginAfter().HasQuirk());
      }
      SetLogicalHeight(BorderAndPaddingLogicalHeight() + LayoutUnit(ScrollbarLogicalHeight())
                       + (HasLineIfEmpty() ? LineHeight() : 0_lu));
      UpdateLogicalHeight();
    };
    computeBlockAxisSize();

    auto computeOverflow = [&]
    {
      ClearOverflow();
      AddVisualEffectOverflow();
      AddVisualOverflowFromTheme();
    };
    computeOverflow();

    auto updateLayerProperties = [&]
    {
      UpdateLayerTransform();
    };
    if (HasLayer())
    {
      updateLayerProperties();
    }

    repainter.RepaintAfterLayout();
  }

  void RenderBlockFlow::LayoutBlock(RelayoutChildren relayoutChildren, LayoutUnit pageLogicalHeight) noexcept
  {
    krys_debug_assert(NeedsLayout());

    if (relayoutChildren == RelayoutChildren::No && SimplifiedLayout())
    {
      return;
    }

    auto isPaginated = [&]
    {
      // FIXME: Grid calls into layout outside of regular layout phase (during preferred width computation).
      if (auto *layoutState = View().FrameView().LayoutContext().LayoutState())
      {
        return layoutState->IsPaginated();
      }
      return false;
    }();

    // FIXME: Multi-column sets always lay out their children, once there are columns.
    if (!FirstChild() && !isPaginated && (Parent() && Parent()->IsBlockContainer()))
    {
      return LayoutBlockWithNoChildren();
    }

    LayoutRepainter repainter(*this);

    if (RecomputeLogicalWidthAndColumnWidth())
    {
      relayoutChildren = RelayoutChildren::Yes;
    }

    RebuildFloatingObjectSetFromIntrudingFloats();

    LayoutUnit previousHeight = LogicalHeight();
    // FIXME: should this start out as borderAndPaddingLogicalHeight() + scrollbarLogicalHeight(),
    // for consistency with other render classes?
    ResetLogicalHeightBeforeLayoutIfNeeded();

    // FIXME: The column height of a multi-column flow (checkForPaginationLogicalHeightChange), once there are
    // columns.
    bool pageLogicalHeightChanged = false;

    LayoutUnit repaintLogicalTop;
    LayoutUnit repaintLogicalBottom;
    LayoutUnit maxFloatLogicalBottom;
    const RenderStyle &styleToUse = Style();
    {
      LayoutStateMaintainer statePusher(*this, LocationOffset(),
                                        IsTransformed() || styleToUse.GetWritingMode().IsBlockFlipped(),
                                        pageLogicalHeight, pageLogicalHeightChanged);

      // FIXME: The fragments' widths (preparePaginationBeforeBlockLayout) and the page's remaining height,
      // once there's pagination.

      // We use four values, maxTopPos, maxTopNeg, maxBottomPos, and maxBottomNeg, to track
      // our current maximal positive and negative margins. These values are used when we
      // are collapsed with adjacent blocks, so for example, if you have block A and B
      // collapsing together, then you'd take the maximal positive margin from both A and B
      // and subtract it from the maximal negative margin from both A and B to get the
      // true collapsed margin. This algorithm is recursive, so when we finish layout()
      // our block knows its current maximal positive/negative values.
      //
      // Start out by setting our margin values to our current margins. Table cells have
      // no margins, so we don't fill in the values for table cells.
      bool isCell = IsRenderTableCell();
      if (!isCell)
      {
        InitMaxMarginValues();

        SetHasMarginBeforeQuirk(styleToUse.MarginBefore().HasQuirk());
        SetHasMarginAfterQuirk(styleToUse.MarginAfter().HasQuirk());
        // FIXME: Clear the pagination strut, once there's pagination.
      }
      if (!FirstChild() && !IsAnonymousBlock())
      {
        SetChildrenInline(true);
      }
      DirtyForLayoutFromPercentageHeightDescendants();
      LayoutInFlowChildren(relayoutChildren, previousHeight, repaintLogicalTop, repaintLogicalBottom,
                           maxFloatLogicalBottom);
      // Expand our intrinsic height to encompass floats.
      LayoutUnit toAdd = BorderAndPaddingAfter() + LayoutUnit(ScrollbarLogicalHeight());
      if (LowestFloatLogicalBottom() > (LogicalHeight() - toAdd) && CreatesNewFormattingContext())
      {
        SetLogicalHeight(LowestFloatLogicalBottom() + toAdd);
      }
      // FIXME: Lay out again to break at a line that avoids a widow (shouldBreakAtLineToAvoidWidow), and
      // relayout for pagination, once there's pagination.
    }

    // Calculate our new height.
    LayoutUnit oldHeight = LogicalHeight();
    LayoutUnit oldClientAfterEdge = ClientLogicalBottom();

    // FIXME: Apply a forced break after the content of a fragmented flow, once there are fragmented flows.

    UpdateLogicalHeight();
    LayoutUnit newHeight = LogicalHeight();

    LayoutUnit alignContentShift;
    auto shouldApplyAlignContent = [&]
    {
      // Alignment isn't supported when fragmenting.
      // FIXME: Only when the content doesn't fit in the page's remaining height, once there's pagination.
      if (isPaginated)
      {
        return false;
      }
      // Table cell alignment is handled in RenderTableCell::computeIntrinsicPadding.
      if (IsRenderTableCell())
      {
        return false;
      }
      // FIXME: Not for <input>, once there are form controls.
      return true;
    };
    if (shouldApplyAlignContent())
    {
      alignContentShift = ShiftForAlignContent(oldHeight, repaintLogicalTop, repaintLogicalBottom);
      oldClientAfterEdge += alignContentShift;
      if (alignContentShift < 0)
      {
        EnsureRareBlockFlowData().AlignContentShift = alignContentShift;
      }
    }
    else if (HasRareBlockFlowData())
    {
      RareBlockFlowData()->AlignContentShift = 0_lu;
    }

    {
      // FIXME: This could be removed once relayoutForPagination() either stop recursing or we manage to
      // re-order them.
      LayoutStateMaintainer statePusher(*this, LocationOffset(),
                                        IsTransformed() || styleToUse.GetWritingMode().IsBlockFlipped(),
                                        pageLogicalHeight, pageLogicalHeightChanged);

      if (oldHeight != newHeight)
      {
        if (oldHeight > newHeight && maxFloatLogicalBottom > newHeight && !ChildrenInline())
        {
          // One of our children's floats may have become an overhanging float for us. We need to look for it.
          for (auto &blockFlow : ChildrenOfType<RenderBlockFlow>(*this))
          {
            if (blockFlow.IsFloatingOrOutOfFlowPositioned())
            {
              continue;
            }
            if (blockFlow.LowestFloatLogicalBottom() + blockFlow.LogicalTop() > newHeight)
            {
              (void)AddOverhangingFloats(blockFlow, false);
            }
          }
        }
      }

      bool heightChanged = (previousHeight != newHeight);
      if (heightChanged || alignContentShift)
      {
        relayoutChildren = RelayoutChildren::Yes;
      }
      if (IsDocumentElementRenderer())
      {
        LayoutOutOfFlowBoxes(RelayoutChildren::Yes);
      }
      else
      {
        LayoutOutOfFlowBoxes(relayoutChildren);
      }
    }

    UpdateDescendantTransformsAfterLayout();

    // Add overflow from children (unless we're multi-column, since in that case all our child overflow is
    // clipped anyway).
    ComputeOverflow(oldClientAfterEdge);

    // FIXME: The page logical offset, once there's pagination.

    UpdateLayerTransform();

    // FIXME: This repaint logic should be moved into a separate helper function!
    // Repaint with our new bounds if they are different from our old bounds.
    bool didFullRepaint = repainter.RepaintAfterLayout();
    if (!didFullRepaint && repaintLogicalTop != repaintLogicalBottom
        && (styleToUse.UsedVisibility() == Visibility::Visible || EnclosingLayer()->HasVisibleContent()))
    {
      // FIXME: We could tighten up the left and right invalidation points if we let layoutInlineChildren fill
      // them in based off the particular lines it had to lay out. We wouldn't need the
      // hasNonVisibleOverflow() hack in that case either.
      LayoutUnit repaintLogicalLeft = LogicalLeftVisualOverflow();
      LayoutUnit repaintLogicalRight = LogicalRightVisualOverflow();
      if (HasNonVisibleOverflow())
      {
        // If we have clipped overflow, we should use layout overflow as well, since visual overflow from
        // lines didn't propagate to our block's overflow. Note the old code did this as well but even for
        // overflow:visible. The addition of hasNonVisibleOverflow() at least tightens up the hack a bit.
        // layoutInlineChildren should be patched to compute the entire repaint rect.
        repaintLogicalLeft = std::min(repaintLogicalLeft, LogicalLeftLayoutOverflow());
        repaintLogicalRight = std::max(repaintLogicalRight, LogicalRightLayoutOverflow());
      }

      LayoutRect repaintRect;
      if (IsHorizontalWritingMode())
      {
        repaintRect =
          LayoutRect(repaintLogicalLeft, repaintLogicalTop, repaintLogicalRight - repaintLogicalLeft,
                     repaintLogicalBottom - repaintLogicalTop);
      }
      else
      {
        repaintRect =
          LayoutRect(repaintLogicalTop, repaintLogicalLeft, repaintLogicalBottom - repaintLogicalTop,
                     repaintLogicalRight - repaintLogicalLeft);
      }

      if (HasNonVisibleOverflow())
      {
        // Adjust repaint rect for scroll offset
        repaintRect.MoveBy(-LayoutPoint(GetScrollPosition()));

        // Don't allow this rect to spill out of our overflow box.
        repaintRect.Intersect(LayoutRect(LayoutPoint(), Size()));
      }

      // Make sure the rect is still non-empty after intersecting for overflow above
      if (!repaintRect.IsEmpty())
      {
        RepaintRectangle(repaintRect); // We need to do a partial repaint of our content.
        // FIXME: Reflections (`-webkit-box-reflect` was dropped).
      }
    }
  }

  void RenderBlockFlow::DirtyForLayoutFromPercentageHeightDescendants() noexcept
  {
    auto *descendants = PercentHeightDescendants();
    if (!descendants)
    {
      return;
    }

    for (auto *descendant : *descendants)
    {
      // Let's not dirty the height perecentage descendant when it has an absolutely positioned containing
      // block ancestor. We should be able to dirty such boxes through the regular invalidation logic.
      bool descendantNeedsLayout = true;
      for (auto *ancestor = descendant->ContainingBlock(); ancestor && ancestor != this;
           ancestor = ancestor->ContainingBlock())
      {
        if (ancestor->IsOutOfFlowPositioned())
        {
          descendantNeedsLayout = false;
          break;
        }
      }
      if (!descendantNeedsLayout)
      {
        continue;
      }

      for (RenderElement *renderer = descendant;
           renderer && renderer != this && !renderer->NormalChildNeedsLayout();
           renderer = renderer->Container())
      {
        renderer->SetChildNeedsLayout(MarkingBehavior::MarkOnlyThis);
        if (auto *renderBox = DynamicDowncast<RenderBox>(*renderer))
        {
          // If the width of an image is affected by the height of a child (e.g., an image with an aspect
          // ratio), then we have to dirty preferred widths, since even enclosing blocks can become dirty as a
          // result. (A horizontal flexbox that contains an inline image wrapped in an anonymous block for
          // example.)
          if (renderBox->HasIntrinsicAspectRatio() || renderBox->Style().HasAspectRatio())
          {
            renderBox->SetNeedsPreferredWidthsUpdate();
          }
        }
      }
    }
  }

  LayoutUnit RenderBlockFlow::ShiftForAlignContent(LayoutUnit intrinsicLogicalHeight,
                                                   LayoutUnit &repaintLogicalTop,
                                                   LayoutUnit &repaintLogicalBottom) noexcept
  {
    auto alignment = Style().GetAlignContent().Resolve();

    // Exit if no alignment necessary.
    if (alignment.IsNormal() || alignment.IsStartward())
    {
      return 0_lu;
    }

    // Calculate alignment shift.
    LayoutUnit computedLogicalHeight = LogicalHeight();
    LayoutUnit space = computedLogicalHeight - intrinsicLogicalHeight;
    if (space <= 0)
    {
      bool overflowIsSafe = (alignment.Overflow() == OverflowAlignment::Default && !IsScrollContainerY())
                            || alignment.Overflow() == OverflowAlignment::Safe
                            || alignment.Position() == ContentPosition::Normal;
      if (overflowIsSafe)
      {
        return 0_lu; // Floored at zero; we're done
      }
    }
    if (alignment.IsCentered())
    {
      space = space / 2;
    }

    // Now shift all our content.
    if (auto *inlineLayout = InlineLayout())
    {
      inlineLayout->ShiftLinesBy(space);
    }
    else
    {
      for (auto *child = FirstChildBox(); child; child = child->NextSiblingBox())
      {
        SetLogicalTopForChild(*child, LogicalTopForChild(*child) + space);
        if (child->IsOutOfFlowPositioned()
            && child->Style().HasStaticBlockPosition(IsHorizontalWritingMode()))
        {
          krys_debug_assert(child->Layer());
          child->Layer()->SetStaticBlockPosition(child->Layer()->StaticBlockPosition() + space);
          child->SetChildNeedsLayout(MarkingBehavior::MarkOnlyThis);
        }
      }
    }
    if (_floatingObjects)
    {
      _floatingObjects->ShiftFloatsBy(space);
    }

    // Update repaint region.
    if (space < 0_lu)
    {
      repaintLogicalTop += space;
    }
    else
    {
      repaintLogicalBottom += space;
    }

    return space;
  }

  void RenderBlockFlow::LayoutInFlowChildren(RelayoutChildren relayoutChildren, LayoutUnit previousHeight,
                                             LayoutUnit &repaintLogicalTop, LayoutUnit &repaintLogicalBottom,
                                             LayoutUnit &maxFloatLogicalBottom) noexcept
  {
    if (!FirstChild())
    {
      // Empty block containers produce empty formatting lines which may affect trim-start/end.
      auto textBoxTrimmer = TextBoxTrimmer {*this};

      auto logicalHeight = BorderAndPaddingLogicalHeight() + LayoutUnit(ScrollbarLogicalHeight());
      if (HasLineIfEmpty())
      {
        logicalHeight += LineHeight();
      }
      SetLogicalHeight(logicalHeight);

      repaintLogicalTop = 0_lu;
      repaintLogicalBottom = 0_lu;
      maxFloatLogicalBottom = 0_lu;
      return;
    }

    // FIXME: Skip the content that content-visibility skips (isSkippedContentRootForLayout), once layout
    // skips content.

    {
      auto textBoxTrimmer = TextBoxTrimmer {*this};
      auto lineClampUpdater = LineClampUpdater {*this};
      ChildrenInline()
        ? LayoutInlineChildren(relayoutChildren, previousHeight, repaintLogicalTop, repaintLogicalBottom)
        : LayoutBlockChildren(relayoutChildren, maxFloatLogicalBottom);
    }
    {
      auto applyTextBoxTrimEndIfNeeded = [&]
      {
        // With block children and blocks-inside-inline, there's no way to tell what the last formatted line
        // is until after we finished laying out the subtree. Dirty the last formatted line (in the last IFC)
        // and issue relayout with forcing trimming the last line if applicable.
        if (auto *rootForLastFormattedLine = TextBoxTrimmer::LastInlineFormattingContextRootForTrimEnd(*this))
        {
          krys_debug_assert(rootForLastFormattedLine != this);
          // FIXME: We should be able to damage the last line only.
          for (RenderBlock *ancestor = rootForLastFormattedLine; ancestor && ancestor != this;
               ancestor = ancestor->ContainingBlock())
          {
            ancestor->SetNeedsLayout(RenderObjectMarkingBehavior::MarkOnlyThis);
          }

          auto textBoxTrimmer = TextBoxTrimmer {*this, *rootForLastFormattedLine};
          ChildrenInline() ? LayoutInlineChildren(RelayoutChildren::No, previousHeight, repaintLogicalTop,
                                                  repaintLogicalBottom)
                           : LayoutBlockChildren(RelayoutChildren::No, maxFloatLogicalBottom);
        }
      };
      applyTextBoxTrimEndIfNeeded();
    }
  }

  void RenderBlockFlow::LayoutBlockChildren(RelayoutChildren relayoutChildren,
                                            LayoutUnit &maxFloatLogicalBottom) noexcept
  {
    krys_debug_assert(FirstChild());

    SetLogicalHeight(BorderAndPaddingBefore());
    auto *layoutState = View().FrameView().LayoutContext().LayoutState();

    // The margin struct caches all our current margin collapsing state.
    auto marginInfo = MarginInfo {*this, MarginInfo::IgnoreScrollbarForAfterMargin::No};

    bool marginTrimBlockStartFromContainingBlock = layoutState->MarginTrimBlockStart();
    bool newMarginTrimBlockStartForSubtree = [&]
    {
      if (Style().GetMarginTrim().Contains(style::MarginTrimSide::BlockStart))
      {
        return true;
      }
      if (!marginInfo.CanCollapseMarginBeforeWithChildren() && marginTrimBlockStartFromContainingBlock)
      {
        return false;
      }
      return marginTrimBlockStartFromContainingBlock;
    }();

    layoutState->SetMarginTrimBlockStart(newMarginTrimBlockStartForSubtree);

    // Fieldsets need to find their legend and position it inside the border of the object.
    // The legend then gets skipped during normal layout. The same is true for ruby text.
    // It doesn't get included in the normal layout process but is instead skipped.
    LayoutExcludedChildren(relayoutChildren);

    LayoutUnit previousFloatLogicalBottom;
    maxFloatLogicalBottom = 0_lu;

    RenderBox *next = FirstChildBox();

    while (next)
    {
      RenderBox &child = *next;
      next = child.NextSiblingBox();

      if (child.IsExcludedFromNormalLayout())
      {
        continue; // Skip this child, since it will be positioned by the specialized subclass (fieldsets and
                  // ruby runs).
      }

      // FIXME: Skip the column spanners that content-visibility skips (isSkippedContentForLayout), once
      // layout skips content.

      UpdateBlockChildDirtyBitsBeforeLayout(relayoutChildren, child);

      if (child.IsOutOfFlowPositioned())
      {
        child.ContainingBlock()->AddOutOfFlowBox(child);
        AdjustOutOfFlowBlock(child, marginInfo);
        continue;
      }
      if (child.IsFloating())
      {
        auto markSiblingsIfIntrudingForLayout = [&]
        {
          // Let's find out if this float box is (was) intruding to sibling boxes and mark them for layout
          // accordingly.
          if (!child.SelfNeedsLayout() || !child.EverHadLayout())
          {
            // At this point floatingObjectSet() is purged, we can't check whether
            // this is a new or an existing float in this block container.
            return;
          }
          for (auto *nextSibling = child.NextSibling(); nextSibling; nextSibling = nextSibling->NextSibling())
          {
            auto *block = DynamicDowncast<RenderBlockFlow>(*nextSibling);
            if (!block)
            {
              continue;
            }
            if (block->AvoidsFloats() && !block->ShrinkToAvoidFloats())
            {
              continue;
            }
            if (IsSkippedContentRootOrSkippedContent(*block) || block->ContainsFloat(child))
            {
              block->MarkAllDescendantsWithFloatsForLayout();
            }
          }
        };
        markSiblingsIfIntrudingForLayout();
        InsertFloatingBoxAndMarkForLayout(child);
        AdjustFloatingBlock(marginInfo);
        continue;
      }

      // Lay out the child.
      LayoutBlockChild(child, marginInfo, previousFloatLogicalBottom, maxFloatLogicalBottom);
    }

    if (Style().GetMarginTrim().Contains(style::MarginTrimSide::BlockEnd))
    {
      TrimBlockEndChildrenMargins();
    }
    // Now do the handling of the bottom of the block, adding in our bottom border/padding and
    // determining the correct collapsed bottom margin information.
    auto borderBoxLogicalHeight =
      HandleAfterSideOfBlock(marginInfo, LogicalHeight() - BorderAndPaddingBefore());
    SetLogicalHeight(borderBoxLogicalHeight);

    // Reset the block-start margin trimming for the containing block.
    layoutState->SetMarginTrimBlockStart(marginTrimBlockStartFromContainingBlock);
  }

  void RenderBlockFlow::TrimBlockEndChildrenMargins() noexcept
  {
    auto trimSelfCollapsingChildDescendantsMargins = [&](RenderBox &child)
    {
      krys_debug_assert(child.IsSelfCollapsingBlock());
      for (auto *descendant = child.FirstChildBox(); descendant;
           descendant = DynamicDowncast<RenderBox>(descendant->NextInPreOrder(&child)))
      {
        SetTrimmedMarginForChild(*descendant, style::MarginTrimSide::BlockStart);
        SetTrimmedMarginForChild(*descendant, style::MarginTrimSide::BlockEnd);
      }
    };

    krys_debug_assert(Style().GetMarginTrim().Contains(style::MarginTrimSide::BlockEnd));
    // If we are trimming the block end margin, we need to make sure we trim the margin of the children
    // at the end of the block by walking back up the container. Any self collapsing children will also need
    // to have their position adjusted to below the last non self-collapsing child in its containing block
    auto *child = LastChildBox();
    while (child)
    {
      if (child->IsExcludedFromNormalLayout() || !child->IsInFlow())
      {
        child = child->PreviousSiblingBox();
        continue;
      }

      auto *childContainingBlock = child->ContainingBlock();
      SetTrimmedMarginForChild(*child, style::MarginTrimSide::BlockEnd);
      if (child->IsSelfCollapsingBlock())
      {
        SetTrimmedMarginForChild(*child, style::MarginTrimSide::BlockStart);
        childContainingBlock->SetLogicalTopForChild(*child, childContainingBlock->LogicalHeight());

        // If this self-collapsing child has any other children, which must also be
        // self-collapsing, we should trim the margins of all its descendants
        if (child->FirstChildBox() && !child->ChildrenInline())
        {
          trimSelfCollapsingChildDescendantsMargins(*child);
        }

        child = child->PreviousSiblingBox();
      }
      else if (auto *nestedBlock = DynamicDowncast<RenderBlockFlow>(child);
               nestedBlock && nestedBlock->IsBlockContainer() && !nestedBlock->ChildrenInline()
               && !nestedBlock->Style().GetMarginTrim().Contains(style::MarginTrimSide::BlockEnd))
      {
        // The margins *inside* this nested block are protected so we should not introspect and try to trim
        // any of them.
        if (!MarginInfo {*nestedBlock}.CanCollapseMarginAfterWithChildren())
        {
          break;
        }

        child = child->LastChildBox();
      }
      else
      {
        // We hit another type of block child that doesn't apply to our search. We can just
        // end the search since nothing before this block can affect the bottom margin of the outer one we are
        // trimming for.
        break;
      }
    }
  }

  void RenderBlockFlow::SimplifiedNormalFlowLayout() noexcept
  {
    if (!ChildrenInline())
    {
      RenderBlock::SimplifiedNormalFlowLayout();
      return;
    }

    bool shouldUpdateOverflow = false;
    for (auto walker = InlineWalker(*this); !walker.AtEnd(); walker.Advance())
    {
      auto &renderer = *walker.Current();
      if (auto *box = DynamicDowncast<RenderBox>(renderer))
      {
        if (!box->IsOutOfFlowPositioned() && box->NeedsLayout())
        {
          box->Layout();
          shouldUpdateOverflow = true;
        }
        continue;
      }
      if (Is<RenderText>(renderer) || Is<RenderInline>(renderer))
      {
        renderer.ClearNeedsLayout();
      }
    }

    if (!shouldUpdateOverflow)
    {
      return;
    }

    if (auto *lineLayout = InlineLayout())
    {
      lineLayout->UpdateOverflow();
    }
  }

  void RenderBlockFlow::LayoutInlineChildren(RelayoutChildren relayoutChildren, LayoutUnit previousHeight,
                                             LayoutUnit &repaintLogicalTop,
                                             LayoutUnit &repaintLogicalBottom) noexcept
  {
    ComputeAndSetLineLayoutPath();
    // Without SVG text (the legacy line layout), inline content always takes the inline path.
    LayoutInlineContent(relayoutChildren, previousHeight, repaintLogicalTop, repaintLogicalBottom);
  }

  void RenderBlockFlow::PerformBlockStepSizing(RenderBox &child,
                                               LayoutUnit blockStepSizeForChild) const noexcept
  {
    krys_debug_assert(BlockStepSizing::ChildHasSupportedStyle(child.Style()));

    auto extraSpace =
      BlockStepSizing::ComputeExtraSpace(blockStepSizeForChild, LogicalMarginBoxHeightForChild(child));
    if (!extraSpace)
    {
      return;
    }

    switch (child.Style().GetBlockStepInsert())
    {
      case BlockStepInsert::MarginBox:
        BlockStepSizing::DistributeExtraSpaceToChildMargins(child, extraSpace, Style().GetWritingMode());
        break;
      case BlockStepInsert::ContentBox:
        BlockStepSizing::DistributeExtraSpaceToChildContentArea(child, extraSpace, Style().GetWritingMode());
        break;
      case BlockStepInsert::PaddingBox:
        BlockStepSizing::DistributeExtraSpaceToChildPadding(child, extraSpace, Style().GetWritingMode());
        break;
    }
  }

  void RenderBlockFlow::LayoutBlockChild(RenderBox &child, MarginInfo &marginInfo,
                                         LayoutUnit &previousFloatLogicalBottom,
                                         LayoutUnit &maxFloatLogicalBottom) noexcept
  {
    LayoutUnit oldPosMarginBefore = MaxPositiveMarginBefore();
    LayoutUnit oldNegMarginBefore = MaxNegativeMarginBefore();

    // The child is a normal flow object. Compute the margins we will use for collapsing now.
    child.ComputeAndSetBlockDirectionMargins(*this);

    // Try to guess our correct logical top position. In most cases this guess will
    // be correct. Only if we're wrong (when we compute the real logical top position)
    // will we have to potentially relayout.
    LayoutUnit estimateWithoutPagination;
    LayoutUnit logicalTopEstimate = EstimateLogicalTopPosition(child, marginInfo, estimateWithoutPagination);

    // Cache our old rect so that we can dirty the proper repaint rects if the child moves.
    LayoutRect oldRect = child.FrameRect();
    LayoutUnit oldLogicalTop = LogicalTopForChild(child);

#if KRYS_DEBUG
    LayoutSize oldLayoutDelta = View().FrameView().LayoutContext().LayoutDelta();
#endif
    // Position the child as though it didn't collapse with the top.
    SetLogicalTopForChild(child, logicalTopEstimate, ApplyLayoutDelta);
    // FIXME: Estimate the child's fragment range (estimateFragmentRangeForBoxChild), once there are
    // fragmented flows.

    auto *childBlockFlow = DynamicDowncast<RenderBlockFlow>(child);
    bool markDescendantsWithFloats = false;
    if (logicalTopEstimate != oldLogicalTop && !child.AvoidsFloats() && childBlockFlow
        && childBlockFlow->ContainsFloats())
    {
      markDescendantsWithFloats = true;
    }
    else if (logicalTopEstimate.MightBeSaturated()) KRYS_UNLIKELY
    {
      // logicalTopEstimate, returned by estimateLogicalTopPosition, might be saturated for
      // very large elements. If it does the comparison with oldLogicalTop might yield a
      // false negative as adding and removing margins, borders etc from a saturated number
      // might yield incorrect results. If this is the case always mark for layout.
      markDescendantsWithFloats = true;
    }
    else if (!child.AvoidsFloats() || child.ShrinkToAvoidFloats())
    {
      // If an element might be affected by the presence of floats, then always mark it for
      // layout.
      LayoutUnit fb = std::max(previousFloatLogicalBottom, LowestFloatLogicalBottom());
      if (fb > logicalTopEstimate)
      {
        markDescendantsWithFloats = true;
      }
    }

    if (childBlockFlow)
    {
      if (markDescendantsWithFloats)
      {
        childBlockFlow->MarkAllDescendantsWithFloatsForLayout();
      }
      if (!child.IsWritingModeRoot())
      {
        previousFloatLogicalBottom =
          std::max(previousFloatLogicalBottom, oldLogicalTop + childBlockFlow->LowestFloatLogicalBottom());
      }
    }

    // FIXME: Mark the child for relayout when its pagination changed (markForPaginationRelayoutIfNeeded),
    // once there's pagination.

    bool childHadLayout = child.EverHadLayout();
    bool childNeededLayout = child.NeedsLayout();
    if (childNeededLayout)
    {
      child.Layout();
    }

    const auto &childStyle = child.Style();
    if (auto blockStepSizeForChild = childStyle.GetBlockStepSize().TryLength();
        blockStepSizeForChild && BlockStepSizing::ChildHasSupportedStyle(childStyle))
    {
      PerformBlockStepSizing(child, LayoutUnit(blockStepSizeForChild->ResolveZoom(style::ZoomNeeded {})));
    }

    // Cache if we are at the top of the block right now.
    bool atBeforeSideOfBlock = marginInfo.AtBeforeSideOfBlock();

    // Now determine the correct ypos based off examination of collapsing margin
    // values.
    LayoutUnit logicalTopBeforeClear = CollapseMargins(child, marginInfo);

    // Now check for clear.
    LayoutUnit logicalTopAfterClear =
      ClearFloatsIfNeeded(child, marginInfo, oldPosMarginBefore, oldNegMarginBefore, logicalTopBeforeClear);

    // FIXME: Adjust the child for pagination (adjustBlockChildForPagination), once there's pagination.
    KRYS_MAYBE_UNUSED auto atBeforeSideOfBlockAndNotCleared =
      atBeforeSideOfBlock && logicalTopBeforeClear == logicalTopAfterClear;

    SetLogicalTopForChild(child, logicalTopAfterClear, ApplyLayoutDelta);

    // Now we have a final top position. See if it really does end up being different from our estimate.
    // clearFloatsIfNeeded can also mark the child as needing a layout even though we didn't move. This
    // happens when collapseMargins dynamically adds overhanging floats because of a child with negative
    // margins.
    if (logicalTopAfterClear != logicalTopEstimate || child.NeedsLayout())
    {
      if (child.ShrinkToAvoidFloats())
      {
        // The child's width depends on the line width. When the child shifts to clear an item, its width can
        // change (because it has more available line width). So mark the item as dirty.
        child.SetChildNeedsLayout(MarkingBehavior::MarkOnlyThis);
      }

      if (childBlockFlow)
      {
        if (!child.AvoidsFloats() && childBlockFlow->ContainsFloats())
        {
          childBlockFlow->MarkAllDescendantsWithFloatsForLayout();
        }
      }
    }

    // FIXME: Relayout the child when its fragment range changed (updateFragmentRangeForBoxChild), once there
    // are fragmented flows.

    // In case our guess was wrong, relayout the child.
    child.LayoutIfNeeded();

    // We are no longer at the top of the block if we encounter a non-empty child.
    // This has to be done after checking for clear, so that margins can be reset if a clear occurred.
    if (marginInfo.AtBeforeSideOfBlock() && !child.IsSelfCollapsingBlock())
    {
      marginInfo.SetAtBeforeSideOfBlock(false);

      if (auto *layoutState = View().FrameView().LayoutContext().LayoutState();
          layoutState && layoutState->MarginTrimBlockStart())
      {
        layoutState->SetMarginTrimBlockStart(false);
      }
    }
    // Now place the child in the correct left position
    DetermineLogicalLeftPositionForChild(child, ApplyLayoutDelta);

    // Update our height now that the child has been placed in the correct position.
    SetLogicalHeight(LogicalHeight() + LogicalHeightForChildForFragmentation(child));

    // If the child has overhanging floats that intrude into following siblings (or possibly out
    // of this block), then the parent gets notified of the floats now.
    if (childBlockFlow && childBlockFlow->ContainsFloats())
    {
      maxFloatLogicalBottom =
        std::max(maxFloatLogicalBottom, AddOverhangingFloats(*childBlockFlow, !childNeededLayout));
    }

    LayoutSize childOffset = child.Location() - oldRect.Location();
    if (childOffset.Width() || childOffset.Height())
    {
      View().FrameView().LayoutContext().AddLayoutDelta(childOffset);

      // If the child moved, we have to repaint it as well as any floating/positioned
      // descendants. An exception is if we need a layout. In this case, we know we're going to
      // repaint ourselves (and the child) anyway.
      if (childHadLayout && !SelfNeedsLayout() && child.CheckForRepaintDuringLayout())
      {
        child.RepaintDuringLayoutIfMoved(oldRect);
      }
    }

    if (!childHadLayout && child.CheckForRepaintDuringLayout())
    {
      child.Repaint();
      child.RepaintOverhangingFloats(true);
    }

    // FIXME: Apply an after page or column break, once there's pagination.

#if KRYS_DEBUG
    krys_debug_assert(View().FrameView().LayoutContext().LayoutDelta() == oldLayoutDelta);
#endif
  }

  void RenderBlockFlow::AdjustOutOfFlowBlock(RenderBox &child, const MarginInfo &marginInfo) noexcept
  {
    bool isHorizontal = IsHorizontalWritingMode();
    bool hasStaticBlockPosition = child.Style().HasStaticBlockPosition(isHorizontal);

    LayoutUnit logicalTop = LogicalHeight();
    UpdateStaticInlinePositionForChild(child, logicalTop);

    if (!marginInfo.CanCollapseWithMarginBefore())
    {
      // Positioned blocks don't collapse margins, so add the margin provided by
      // the container now. The child's own margin is added later when calculating its logical top.
      LayoutUnit collapsedBeforePos = marginInfo.PositiveMargin();
      LayoutUnit collapsedBeforeNeg = marginInfo.NegativeMargin();
      logicalTop += collapsedBeforePos - collapsedBeforeNeg;
    }

    RenderLayer *childLayer = child.Layer();
    if (childLayer->StaticBlockPosition() != logicalTop)
    {
      childLayer->SetStaticBlockPosition(logicalTop);
      if (hasStaticBlockPosition)
      {
        child.SetChildNeedsLayout(MarkingBehavior::MarkOnlyThis);
      }
    }
  }

  void RenderBlockFlow::UpdateStaticInlinePositionForChild(RenderBox &child, LayoutUnit logicalTop) noexcept
  {
    if (child.Style().IsOriginalDisplayInlineType())
    {
      SetStaticInlinePositionForChild(child, StaticInlinePositionForOriginalDisplayInline(logicalTop));
    }
    else
    {
      SetStaticInlinePositionForChild(child, StartOffsetForContent());
    }
  }

  void RenderBlockFlow::SetStaticInlinePositionForChild(RenderBox &child, LayoutUnit inlinePosition) noexcept
  {
    // FIXME: Shift the inline position to exclude the fragment offset, once there are fragmented flows.
    child.Layer()->SetStaticInlinePosition(inlinePosition);
  }

  LayoutUnit RenderBlockFlow::StaticInlinePositionForOriginalDisplayInline(LayoutUnit logicalTop) noexcept
  {
    style::TextAlign textAlign = Style().GetTextAlign();

    float logicalLeft = LogicalLeftOffsetForLine(logicalTop).ToFloat();
    float logicalRight = LogicalRightOffsetForLine(logicalTop).ToFloat();

    auto writingMode = Style().GetWritingMode();
    bool isRightAligned = false;
    switch (textAlign)
    {
      case style::TextAlign::Left:
      case style::TextAlign::WebKitLeft:   break;
      case style::TextAlign::Right:
      case style::TextAlign::WebKitRight:  isRightAligned = true; break;
      case style::TextAlign::Center:
      case style::TextAlign::WebKitCenter: logicalLeft += (logicalRight - logicalLeft) / 2; break;
      case style::TextAlign::Justify:
      case style::TextAlign::Start:
        if (writingMode.IsBidiRTL())
        {
          isRightAligned = true;
        }
        break;
      case style::TextAlign::End:
        if (writingMode.IsBidiLTR())
        {
          isRightAligned = true;
        }
        break;
    }

    if (isRightAligned == writingMode.IsLogicalLeftLineLeft())
    {
      logicalLeft = logicalRight;
    }

    if (!writingMode.IsLogicalLeftInlineStart())
    {
      return LayoutUnit(LogicalWidth().ToFloat() - logicalLeft);
    }

    return LayoutUnit(logicalLeft);
  }

  void RenderBlockFlow::DetermineLogicalLeftPositionForChild(RenderBox &child,
                                                             ApplyLayoutDeltaMode applyDelta) noexcept
  {
    LayoutUnit startPosition = BorderAndPaddingStart();
    LayoutUnit initialStartPosition = startPosition;
    auto verticalScrollbarWidthClampedToContentBox = std::min<LayoutUnit>(
      LayoutUnit(VerticalScrollbarWidth()), std::max(0_lu, LogicalWidth() - BorderAndPaddingLogicalWidth()));
    auto writingMode = Style().GetWritingMode();
    if ((Style().ShouldPlaceVerticalScrollbarOnLeft() || Style().GetScrollbarGutter().IsStableBothEdges())
        && IsHorizontalWritingMode())
    {
      startPosition +=
        (writingMode.IsLogicalLeftInlineStart() ? 1 : -1) * verticalScrollbarWidthClampedToContentBox;
    }
    if (Style().GetScrollbarGutter().IsStableBothEdges() && !IsHorizontalWritingMode())
    {
      startPosition += (writingMode.IsLogicalLeftInlineStart() ? 1 : -1) * HorizontalScrollbarHeight();
    }
    LayoutUnit totalAvailableLogicalWidth = BorderAndPaddingLogicalWidth() + ContentBoxLogicalWidth();

    LayoutUnit childMarginStart = MarginStartForChild(child);
    LayoutUnit newPosition = startPosition + childMarginStart;

    LayoutUnit positionToAvoidFloats;

    if (child.AvoidsFloats() && ContainsFloats())
    {
      positionToAvoidFloats = StartOffsetForLine(LogicalTopForChild(child), LogicalHeightForChild(child));
    }

    // If the child has an offset from the content edge to avoid floats then use that, otherwise let any
    // negative margin pull it back over the content edge or any positive margin push it out. If the child is
    // being centred then the margin calculated to do that has factored in any offset required to avoid
    // floats, so use it if necessary.

    if (Style().GetTextAlign() == style::TextAlign::WebKitCenter
        || child.Style().MarginStart(writingMode).IsAuto())
    {
      newPosition = std::max(newPosition, positionToAvoidFloats + childMarginStart);
    }
    else if (positionToAvoidFloats > initialStartPosition)
    {
      newPosition = std::max(newPosition, positionToAvoidFloats);
    }

    SetLogicalLeftForChild(child,
                           writingMode.IsLogicalLeftInlineStart()
                             ? newPosition
                             : totalAvailableLogicalWidth - newPosition - LogicalWidthForChild(child),
                           applyDelta);
  }

  MarginValues RenderBlockFlow::MarginValuesForChild(RenderBox &child) const noexcept
  {
    LayoutUnit childBeforePositive;
    LayoutUnit childBeforeNegative;
    LayoutUnit childAfterPositive;
    LayoutUnit childAfterNegative;

    LayoutUnit beforeMargin;
    LayoutUnit afterMargin;

    auto *childRenderBlock = DynamicDowncast<RenderBlockFlow>(child);

    // If the child has the same directionality as we do, then we can just return its
    // margins in the same direction.
    if (!child.IsWritingModeRoot())
    {
      if (childRenderBlock)
      {
        childBeforePositive = childRenderBlock->MaxPositiveMarginBefore();
        childBeforeNegative = childRenderBlock->MaxNegativeMarginBefore();
        childAfterPositive = childRenderBlock->MaxPositiveMarginAfter();
        childAfterNegative = childRenderBlock->MaxNegativeMarginAfter();
      }
      else
      {
        beforeMargin = child.MarginBefore();
        afterMargin = child.MarginAfter();
      }
    }
    else if (child.IsHorizontalWritingMode() == IsHorizontalWritingMode())
    {
      // The child has a different directionality. If the child is parallel, then it's just
      // flipped relative to us. We can use the margins for the opposite edges.
      if (childRenderBlock)
      {
        childBeforePositive = childRenderBlock->MaxPositiveMarginAfter();
        childBeforeNegative = childRenderBlock->MaxNegativeMarginAfter();
        childAfterPositive = childRenderBlock->MaxPositiveMarginBefore();
        childAfterNegative = childRenderBlock->MaxNegativeMarginBefore();
      }
      else
      {
        beforeMargin = child.MarginAfter();
        afterMargin = child.MarginBefore();
      }
    }
    else
    {
      // The child is perpendicular to us, which means its margins don't collapse but are on the
      // "logical left/right" sides of the child box. We can just return the raw margin in this case.
      beforeMargin = MarginBeforeForChild(child);
      afterMargin = MarginAfterForChild(child);
    }

    // Resolve uncollapsing margins into their positive/negative buckets.
    if (beforeMargin)
    {
      if (beforeMargin > 0)
      {
        childBeforePositive = beforeMargin;
      }
      else
      {
        childBeforeNegative = -beforeMargin;
      }
    }
    if (afterMargin)
    {
      if (afterMargin > 0)
      {
        childAfterPositive = afterMargin;
      }
      else
      {
        childAfterNegative = -afterMargin;
      }
    }

    return MarginValues(childBeforePositive, childBeforeNegative, childAfterPositive, childAfterNegative);
  }

  bool RenderBlockFlow::ChildrenPreventSelfCollapsing() const noexcept
  {
    if (!ChildrenInline())
    {
      return RenderBlock::ChildrenPreventSelfCollapsing();
    }

    if (InlineLayout())
    {
      return !InlineLayout()->IsSelfCollapsingContent();
    }

    // Containers with no children.
    return false;
  }

  LayoutUnit RenderBlockFlow::CollapseMargins(RenderBox &child, MarginInfo &marginInfo) noexcept
  {
    auto beforeCollapseLogicalTop = LogicalHeight();
    auto logicalTop = CollapseMarginsWithChildInfo(&child, marginInfo);
    auto addIntrudingFloatsFromPreviousBlocks = [&]
    {
      for (auto *previousSibling = child.PreviousSibling(); previousSibling;
           previousSibling = previousSibling->PreviousSibling())
      {
        auto *previousBlockSibling = DynamicDowncast<RenderBlockFlow>(previousSibling);
        if (!previousBlockSibling || previousBlockSibling->CreatesNewFormattingContext())
        {
          continue;
        }
        if (previousBlockSibling->LogicalTop() + previousBlockSibling->LowestFloatLogicalBottom()
            <= logicalTop)
        {
          break;
        }
        // If |child| is a self-collapsing block it may have collapsed into a previous sibling and although it
        // hasn't reduced the height of the parent yet any floats from the parent will now overhang.
        auto oldLogicalHeight = LogicalHeight();
        SetLogicalHeight(logicalTop);
        if (previousBlockSibling->ContainsFloats() && !previousBlockSibling->AvoidsFloats())
        {
          (void)AddOverhangingFloats(*previousBlockSibling, false);
        }
        SetLogicalHeight(oldLogicalHeight);
      }
    };
    addIntrudingFloatsFromPreviousBlocks();
    // If |child|'s previous sibling is or contains a self-collapsing block that cleared a float and margin
    // collapsing resulted in |child| moving up into the margin area of the self-collapsing block then the
    // float it clears is now intruding into |child|. Layout again so that we can look for floats in the
    // parent that overhang |child|'s new logical top.
    auto logicalTopIntrudesIntoFloat = logicalTop < beforeCollapseLogicalTop;
    if (logicalTopIntrudesIntoFloat && ContainsFloats() && !child.AvoidsFloats()
        && LowestFloatLogicalBottom() > logicalTop)
    {
      child.SetNeedsLayout();
    }
    return logicalTop;
  }

  Maybe<LayoutUnit> RenderBlockFlow::SelfCollapsingMarginBeforeWithClear(RenderObject *candidate) noexcept
  {
    auto *candidateBlockFlow = DynamicDowncast<RenderBlockFlow>(candidate);
    if (!candidateBlockFlow)
    {
      return null;
    }

    if (!candidateBlockFlow->IsSelfCollapsingBlock())
    {
      return null;
    }

    if (RenderStyle::GetUsedClear(*candidateBlockFlow) == UsedClear::None || !ContainsFloats())
    {
      return null;
    }

    auto clear = ComputedClearDeltaForChild(*candidateBlockFlow, candidateBlockFlow->LogicalHeight());
    // Just because a block box has the clear property set, it does not mean we always get clearance (e.g.
    // when the box is below the cleared floats)
    if (clear < candidateBlockFlow->LogicalBottom())
    {
      return null;
    }

    return MarginValuesForChild(*candidateBlockFlow).PositiveMarginBefore();
  }

  LayoutUnit RenderBlockFlow::CollapseMarginsWithChildInfo(RenderBox *child, MarginInfo &marginInfo) noexcept
  {
    bool childIsSelfCollapsing = child ? child->IsSelfCollapsingBlock() : false;
    bool beforeQuirk = child ? HasMarginBeforeQuirk(*child) : false;
    bool afterQuirk = child ? HasMarginAfterQuirk(*child) : false;
    auto trimChildBlockMargins = [&]()
    {
      auto *childBlockFlow = DynamicDowncast<RenderBlockFlow>(child);
      if (childBlockFlow)
      {
        childBlockFlow->SetMaxMarginBeforeValues(0_lu, 0_lu);
      }
      SetTrimmedMarginForChild(*child, style::MarginTrimSide::BlockStart);

      // The margin after for a self collapsing child should also be trimmed so it does not
      // influence the margins of the first non collapsing child
      if (childIsSelfCollapsing)
      {
        if (childBlockFlow)
        {
          childBlockFlow->SetMaxMarginAfterValues(0_lu, 0_lu);
        }
        SetTrimmedMarginForChild(*child, style::MarginTrimSide::BlockEnd);
      }
    };
    if (View().FrameView().LayoutContext().LayoutState()->MarginTrimBlockStart())
    {
      krys_debug_assert(marginInfo.AtBeforeSideOfBlock());
      trimChildBlockMargins();
    }

    // Get the four margin values for the child and cache them.
    MarginValues childMargins = child ? MarginValuesForChild(*child) : MarginValues(0_lu, 0_lu, 0_lu, 0_lu);
    // Get our max pos and neg top margins.
    LayoutUnit posTop = childMargins.PositiveMarginBefore();
    LayoutUnit negTop = childMargins.NegativeMarginBefore();

    // For self-collapsing blocks, collapse our bottom margins into our
    // top to get new posTop and negTop values.
    if (childIsSelfCollapsing)
    {
      posTop = std::max(posTop, childMargins.PositiveMarginAfter());
      negTop = std::max(negTop, childMargins.NegativeMarginAfter());
    }

    if (marginInfo.CanCollapseWithMarginBefore())
    {
      // This child is collapsing with the top of the
      // block. If it has larger margin values, then we need to update
      // our own maximal values.
      if (!GetDocument().InQuirksMode() || !marginInfo.QuirkContainer() || !beforeQuirk)
      {
        SetMaxMarginBeforeValues(std::max(posTop, MaxPositiveMarginBefore()),
                                 std::max(negTop, MaxNegativeMarginBefore()));
      }

      // The minute any of the margins involved isn't a quirk, don't
      // collapse it away, even if the margin is smaller (www.webreference.com
      // has an example of this, a <dt> with 0.8em author-specified inside
      // a <dl> inside a <td>.
      if (!marginInfo.DeterminedMarginBeforeQuirk() && !beforeQuirk && (posTop - negTop))
      {
        SetHasMarginBeforeQuirk(false);
        marginInfo.SetDeterminedMarginBeforeQuirk(true);
      }

      if (!marginInfo.DeterminedMarginBeforeQuirk() && beforeQuirk && !MarginBefore())
      {
        // We have no top margin and our top child has a quirky margin.
        // We will pick up this quirky margin and pass it through.
        // This deals with the <td><div><p> case.
        // Don't do this for a block that split two inlines though. You do
        // still apply margins in this case.
        SetHasMarginBeforeQuirk(true);
      }
    }

    if (marginInfo.QuirkContainer() && marginInfo.AtBeforeSideOfBlock() && (posTop - negTop))
    {
      marginInfo.SetHasMarginBeforeQuirk(beforeQuirk);
    }

    LayoutUnit beforeCollapseLogicalTop = LogicalHeight();
    LayoutUnit logicalTop = beforeCollapseLogicalTop;
    // If the child's previous sibling is a self-collapsing block that cleared a float then its top border
    // edge has been set at the bottom border edge of the float. Since we want to collapse the child's top
    // margin with the self-collapsing block's top and bottom margins we need to adjust our parent's height to
    // match the margin top of the self-collapsing block. If the resulting collapsed margin leaves the child
    // still intruding into the float then we will want to clear it.
    if (!marginInfo.CanCollapseWithMarginBefore())
    {
      if (auto value = SelfCollapsingMarginBeforeWithClear(child->PreviousSibling()))
      {
        SetLogicalHeight(LogicalHeight() - *value);
      }
    }

    if (childIsSelfCollapsing)
    {
      // This child has no height. We need to compute our
      // position before we collapse the child's margins together,
      // so that we can get an accurate position for the zero-height block.
      LayoutUnit collapsedBeforePos =
        std::max(marginInfo.PositiveMargin(), childMargins.PositiveMarginBefore());
      LayoutUnit collapsedBeforeNeg =
        std::max(marginInfo.NegativeMargin(), childMargins.NegativeMarginBefore());
      marginInfo.SetMargin(collapsedBeforePos, collapsedBeforeNeg);

      // Now collapse the child's margins together, which means examining our
      // bottom margin values as well.
      marginInfo.SetPositiveMarginIfLarger(childMargins.PositiveMarginAfter());
      marginInfo.SetNegativeMarginIfLarger(childMargins.NegativeMarginAfter());

      if (!marginInfo.CanCollapseWithMarginBefore())
      {
        // We need to make sure that the position of the self-collapsing block
        // is correct, since it could have overflowing content
        // that needs to be positioned correctly (e.g., a block that
        // had a specified height of 0 but that actually had subcontent).
        logicalTop = LogicalHeight() + collapsedBeforePos - collapsedBeforeNeg;
      }
    }
    else
    {
      if (!marginInfo.AtBeforeSideOfBlock()
          || (!marginInfo.CanCollapseMarginBeforeWithChildren()
              && (!GetDocument().InQuirksMode() || !marginInfo.QuirkContainer()
                  || !marginInfo.HasMarginBeforeQuirk())))
      {
        // We're collapsing with a previous sibling's margins and not
        // with the top of the block.
        SetLogicalHeight(LogicalHeight() + std::max(marginInfo.PositiveMargin(), posTop)
                         - std::max(marginInfo.NegativeMargin(), negTop));
        logicalTop = LogicalHeight();
      }

      marginInfo.SetPositiveMargin(childMargins.PositiveMarginAfter());
      marginInfo.SetNegativeMargin(childMargins.NegativeMarginAfter());

      if (marginInfo.Margin())
      {
        marginInfo.SetHasMarginAfterQuirk(afterQuirk);
      }
    }

    // FIXME: If margins would pull us past the top of the next page, then we need to pull back and pretend
    // like the margins collapsed into the page edge, once there's pagination.

    return logicalTop;
  }

  bool RenderBlockFlow::IsChildEligibleForMarginTrim(style::MarginTrimSide marginTrimSide,
                                                     const RenderBox &child) const noexcept
  {
    krys_debug_assert(Style().GetMarginTrim().Contains(marginTrimSide));
    if (!child.Style().IsDisplayBlockLevel())
    {
      return false;
    }
    // https://drafts.csswg.org/css-box-4/#margin-trim-block
    // 3.3.1. Trimming Block Container Content
    // For block containers specifically, margin-trim discards:
    switch (marginTrimSide)
    {
      case style::MarginTrimSide::BlockStart:
        // The block-start margin of a block-level first child, when trimming at the block-start edge.
        return FirstInFlowChildBox() == &child;
      case style::MarginTrimSide::BlockEnd:
        // The block-end margin of a block-level last child, when trimming at the block-end edge.
        return LastInFlowChildBox() == &child;
      case style::MarginTrimSide::InlineStart:
      case style::MarginTrimSide::InlineEnd:
        // It has no effect on the inline-axis margins of block-level descendants, nor on any margins of
        // inline-level descendants.
        return false;
    }
    krys_debug_assert(false);
    return false;
  }

  LayoutUnit RenderBlockFlow::ClearFloatsIfNeeded(RenderBox &child, MarginInfo &marginInfo,
                                                  LayoutUnit oldTopPosMargin, LayoutUnit oldTopNegMargin,
                                                  LayoutUnit yPos) noexcept
  {
    LayoutUnit heightIncrease = ComputedClearDeltaForChild(child, yPos);
    if (!heightIncrease)
    {
      return yPos;
    }

    if (child.IsSelfCollapsingBlock())
    {
      // For self-collapsing blocks that clear, they can still collapse their
      // margins with following siblings. Reset the current margins to represent
      // the self-collapsing block's margins only.
      MarginValues childMargins = MarginValuesForChild(child);
      marginInfo.SetPositiveMargin(
        std::max(childMargins.PositiveMarginBefore(), childMargins.PositiveMarginAfter()));
      marginInfo.SetNegativeMargin(
        std::max(childMargins.NegativeMarginBefore(), childMargins.NegativeMarginAfter()));

      // CSS2.1 states:
      // "If the top and bottom margins of an element with clearance are adjoining, its margins collapse with
      // the adjoining margins of following siblings but that resulting margin does not collapse with the
      // bottom margin of the parent block." So the parent's bottom margin cannot collapse through this block
      // or any subsequent self-collapsing blocks. Check subsequent siblings for a block with height - if none
      // is found then don't allow the margins to collapse with the parent.
      bool wouldCollapseMarginsWithParent = marginInfo.CanCollapseMarginAfterWithChildren();
      for (RenderBox *curr = child.NextSiblingBox(); curr && wouldCollapseMarginsWithParent;
           curr = curr->NextSiblingBox())
      {
        if (!curr->IsFloatingOrOutOfFlowPositioned() && !curr->IsSelfCollapsingBlock())
        {
          wouldCollapseMarginsWithParent = false;
        }
      }
      if (wouldCollapseMarginsWithParent)
      {
        marginInfo.SetCanCollapseMarginAfterWithChildren(false);
      }

      // For now set the border-top of |child| flush with the bottom border-edge of the float so it can layout
      // any floating or positioned children of its own at the correct vertical position. If subsequent
      // siblings attempt to collapse with |child|'s margins in |collapseMargins| we will adjust the height of
      // the parent to |child|'s margin top (which if it is positive sits up 'inside' the float it's clearing)
      // so that all three margins can collapse at the correct vertical position. Per CSS2.1 we need to ensure
      // that any negative margin-top clears |child| beyond the bottom border-edge of the float so that the
      // top border edge of the child (i.e. its clearance)  is at a position that satisfies the equation: "the
      // amount of clearance is set so that clearance + margin-top = [height of float], i.e., clearance =
      // [height of float] - margin-top".
      SetLogicalHeight(child.LogicalTop() + childMargins.NegativeMarginBefore());
    }
    else
    {
      // Increase our height by the amount we had to clear.
      SetLogicalHeight(LogicalHeight() + heightIncrease);
    }

    if (marginInfo.CanCollapseWithMarginBefore())
    {
      // We can no longer collapse with the top of the block since a clear
      // occurred. The empty blocks collapse into the cleared block.
      // https://www.w3.org/TR/CSS2/visuren.html#clearance
      // "CSS2.1 - Computing the clearance of an element on which 'clear' is set is done..."
      SetMaxMarginBeforeValues(oldTopPosMargin, oldTopNegMargin);
      marginInfo.SetAtBeforeSideOfBlock(false);
    }

    return yPos + heightIncrease;
  }

  void RenderBlockFlow::MarginBeforeEstimateForChild(RenderBox &child, LayoutUnit &positiveMarginBefore,
                                                     LayoutUnit &negativeMarginBefore) const noexcept
  {
    // Give up if in quirks mode and we're a body/table cell and the top margin of the child box is quirky.
    // Give up if the child specified -webkit-margin-collapse: separate that prevents collapsing.
    if (GetDocument().InQuirksMode() && HasMarginBeforeQuirk(child) && (IsRenderTableCell() || IsBody()))
    {
      return;
    }

    LayoutUnit beforeChildMargin = MarginBeforeForChild(child);
    positiveMarginBefore = std::max(positiveMarginBefore, beforeChildMargin);
    negativeMarginBefore = std::max(negativeMarginBefore, -beforeChildMargin);

    auto *childBlock = DynamicDowncast<RenderBlockFlow>(child);
    if (!childBlock)
    {
      return;
    }

    if (childBlock->ChildrenInline() || childBlock->IsWritingModeRoot())
    {
      return;
    }

    if (!MarginInfo {*childBlock}.CanCollapseMarginBeforeWithChildren())
    {
      return;
    }

    RenderBox *grandchildBox = childBlock->FirstChildBox();
    for (; grandchildBox; grandchildBox = grandchildBox->NextSiblingBox())
    {
      if (!grandchildBox->IsFloatingOrOutOfFlowPositioned())
      {
        break;
      }
    }

    if (!grandchildBox)
    {
      return;
    }

    // Make sure to update the block margins now for the grandchild box so that we're looking at current
    // values.
    if (grandchildBox->NeedsLayout())
    {
      grandchildBox->ComputeAndSetBlockDirectionMargins(*this);
      if (auto *grandchildBlock = DynamicDowncast<RenderBlock>(*grandchildBox))
      {
        grandchildBlock->SetHasMarginBeforeQuirk(grandchildBox->Style().MarginBefore().HasQuirk());
        grandchildBlock->SetHasMarginAfterQuirk(grandchildBox->Style().MarginAfter().HasQuirk());
      }
    }

    // If we have a 'clear' value but also have a margin we may not actually require clearance to move past
    // any floats. If that's the case we want to be sure we estimate the correct position including margins
    // after any floats rather than use 'clearance' later which could give us the wrong position.
    if (RenderStyle::GetUsedClear(*grandchildBox) != UsedClear::None
        && !childBlock->MarginBeforeForChild(*grandchildBox))
    {
      return;
    }

    // Collapse the margin of the grandchild box with our own to produce an estimate.
    childBlock->MarginBeforeEstimateForChild(*grandchildBox, positiveMarginBefore, negativeMarginBefore);
  }

  LayoutUnit RenderBlockFlow::EstimateLogicalTopPosition(RenderBox &child, const MarginInfo &marginInfo,
                                                         LayoutUnit &estimateWithoutPagination) noexcept
  {
    // FIXME: We need to eliminate the estimation of vertical position, because when it's wrong we sometimes
    // trigger a pathological relayout if there are intruding floats.
    LayoutUnit logicalTopEstimate = LogicalHeight();
    if (!marginInfo.CanCollapseWithMarginBefore())
    {
      LayoutUnit positiveMarginBefore;
      LayoutUnit negativeMarginBefore;
      if (child.SelfNeedsLayout())
      {
        // Try to do a basic estimation of how the collapse is going to go.
        MarginBeforeEstimateForChild(child, positiveMarginBefore, negativeMarginBefore);
      }
      else
      {
        // Use the cached collapsed margin values from a previous layout. Most of the time they
        // will be right.
        MarginValues marginValues = MarginValuesForChild(child);
        positiveMarginBefore = std::max(positiveMarginBefore, marginValues.PositiveMarginBefore());
        negativeMarginBefore = std::max(negativeMarginBefore, marginValues.NegativeMarginBefore());
      }

      // Collapse the result with our current margins.
      logicalTopEstimate += std::max(marginInfo.PositiveMargin(), positiveMarginBefore)
                            - std::max(marginInfo.NegativeMargin(), negativeMarginBefore);
    }

    // FIXME: Adjust logicalTopEstimate down to the next page if the margins are so large that we don't fit on
    // the current page, once there's pagination.

    logicalTopEstimate += ComputedClearDeltaForChild(child, logicalTopEstimate);

    estimateWithoutPagination = logicalTopEstimate;

    // FIXME: Apply the child's before break, shift an unsplittable child to the next page and add the
    // pagination strut, once there's pagination.

    return logicalTopEstimate;
  }

  void RenderBlockFlow::SetCollapsedBottomMargin(const MarginInfo &marginInfo) noexcept
  {
    if (marginInfo.CanCollapseWithMarginAfter() && !marginInfo.CanCollapseWithMarginBefore())
    {
      // Update our max pos/neg bottom margins, since we collapsed our bottom margins
      // with our children.
      auto shouldTrimBlockEndMargin = Style().GetMarginTrim().Contains(style::MarginTrimSide::BlockEnd);
      auto propagatedPositiveMargin = shouldTrimBlockEndMargin ? 0_lu : marginInfo.PositiveMargin();
      auto propagatedNegativeMargin = shouldTrimBlockEndMargin ? 0_lu : marginInfo.NegativeMargin();
      SetMaxMarginAfterValues(std::max(MaxPositiveMarginAfter(), propagatedPositiveMargin),
                              std::max(MaxNegativeMarginAfter(), propagatedNegativeMargin));

      if (!marginInfo.HasMarginAfterQuirk())
      {
        SetHasMarginAfterQuirk(false);
      }

      if (marginInfo.HasMarginAfterQuirk() && !MarginAfter())
      {
        // We have no bottom margin and our last child has a quirky margin.
        // We will pick up this quirky margin and pass it through.
        // This deals with the <td><div><p> case.
        SetHasMarginAfterQuirk(true);
      }
    }
  }

  LayoutUnit RenderBlockFlow::HandleAfterSideOfBlock(MarginInfo &marginInfo,
                                                     LayoutUnit contentBoxLogicalHeight) noexcept
  {
    marginInfo.SetAtAfterSideOfBlock(true);

    // If our last child was a self-collapsing block with clearance then our logical height is flush with the
    // bottom edge of the float that the child clears. The correct vertical position for the margin-collapsing
    // we want to perform now is at the child's margin-top - so adjust our height to that position.
    auto borderBoxLogicalHeight = BorderAndPaddingBefore() + contentBoxLogicalHeight;
    if (auto selfCollapsingMarginBeforeWithClear = this->SelfCollapsingMarginBeforeWithClear(LastChild()))
    {
      borderBoxLogicalHeight -= *selfCollapsingMarginBeforeWithClear;
    }

    // If we can't collapse with children then add in the bottom margin.
    if (!marginInfo.CanCollapseWithMarginAfter() && !marginInfo.CanCollapseWithMarginBefore()
        && (!GetDocument().InQuirksMode() || !marginInfo.QuirkContainer()
            || !marginInfo.HasMarginAfterQuirk()))
    {
      borderBoxLogicalHeight += marginInfo.Margin();
    }

    // Now add in our bottom border/padding.
    borderBoxLogicalHeight += BorderAndPaddingAfter() + LayoutUnit(ScrollbarLogicalHeight());

    // Negative margins can cause our height to shrink below our minimal height (border/padding).
    // If this happens, ensure that the computed height is increased to the minimal height.
    borderBoxLogicalHeight = std::max(borderBoxLogicalHeight,
                                      BorderAndPaddingLogicalHeight() + LayoutUnit(ScrollbarLogicalHeight()));

    // Update our bottom collapsed margin info.
    SetCollapsedBottomMargin(marginInfo);

    return borderBoxLogicalHeight;
  }

  void RenderBlockFlow::SetMaxMarginBeforeValues(LayoutUnit pos, LayoutUnit neg) noexcept
  {
    if (!HasRareBlockFlowData())
    {
      if (pos == RenderBlockFlowRareData::PositiveMarginBeforeDefault(*this)
          && neg == RenderBlockFlowRareData::NegativeMarginBeforeDefault(*this))
      {
        return;
      }
      MaterializeRareBlockFlowData();
    }

    RareBlockFlowData()->Margins.SetPositiveMarginBefore(pos);
    RareBlockFlowData()->Margins.SetNegativeMarginBefore(neg);
  }

  void RenderBlockFlow::SetMaxMarginAfterValues(LayoutUnit pos, LayoutUnit neg) noexcept
  {
    if (!HasRareBlockFlowData())
    {
      if (pos == RenderBlockFlowRareData::PositiveMarginAfterDefault(*this)
          && neg == RenderBlockFlowRareData::NegativeMarginAfterDefault(*this))
      {
        return;
      }
      MaterializeRareBlockFlowData();
    }

    RareBlockFlowData()->Margins.SetPositiveMarginAfter(pos);
    RareBlockFlowData()->Margins.SetNegativeMarginAfter(neg);
  }

  void RenderBlockFlow::StyleDidChange(style::Difference diff, const RenderStyle *oldStyle) noexcept
  {
    RenderBlock::StyleDidChange(diff, oldStyle);

    // After our style changed, if we lose our ability to propagate floats into next sibling
    // blocks, then we need to find the top most parent containing that overhanging float and
    // then mark its descendants with floats for layout and clear all floats from its next
    // sibling blocks that exist in our floating objects list. See bug 56299 and 62875.
    bool canPropagateFloatIntoSibling = !IsFloatingOrOutOfFlowPositioned() && !AvoidsFloats();
    if (diff == style::DifferenceResult::Layout && s_canPropagateFloatIntoSibling
        && !canPropagateFloatIntoSibling && HasOverhangingFloats())
    {
      RenderBlockFlow *parentBlock = this;
      for (auto &ancestor : AncestorsOfType<RenderBlockFlow>(*this))
      {
        if (ancestor.IsRenderView())
        {
          break;
        }
        if (ancestor.HasOverhangingFloats())
        {
          for (const auto &floatingObject : _floatingObjects->Set())
          {
            if (!floatingObject->Renderer())
            {
              continue;
            }
            if (ancestor.HasOverhangingFloat(*floatingObject->Renderer()))
            {
              parentBlock = &ancestor;
              break;
            }
          }
        }
      }

      parentBlock->MarkAllDescendantsWithFloatsForLayout();
      parentBlock->MarkSiblingsWithFloatsForLayout();
    }

    if (diff == style::DifferenceResult::Layout && SelfNeedsLayout() && ChildrenInline())
    {
      for (auto walker = InlineWalker(*this); !walker.AtEnd(); walker.Advance())
      {
        walker.Current()->SetNeedsPreferredWidthsUpdate();
      }
    }

    // FIXME: The styles of the multi-column flow's children, once there's multi-column layout.
  }

  void RenderBlockFlow::StyleWillChange(style::Difference diff, const RenderStyle &newStyle) noexcept
  {
    const RenderStyle *oldStyle = HasInitializedStyle() ? &Style() : nullptr;
    s_canPropagateFloatIntoSibling = oldStyle ? !IsFloatingOrOutOfFlowPositioned() && !AvoidsFloats() : false;

    if (oldStyle)
    {
      auto oldPosition = oldStyle->GetPosition();
      auto newPosition = newStyle.GetPosition();

      if (Parent() && diff == style::DifferenceResult::Layout && oldPosition != newPosition)
      {
        if (ContainsFloats() && !IsFloating() && !IsOutOfFlowPositioned() && newStyle.HasOutOfFlowPosition())
        {
          MarkAllDescendantsWithFloatsForLayout();
        }
      }
    }

    RenderBlock::StyleWillChange(diff, newStyle);
  }

  void RenderBlockFlow::AddFloatsToNewParent(RenderBlockFlow &toBlockFlow) const noexcept
  {
    // When a portion of the render tree is being detached, anonymous blocks
    // will be combined as their children are deleted. In this process, the
    // anonymous block later in the tree is merged into the one preceeding it.
    // It can happen that the later block (this) contains floats that the
    // previous block (toBlockFlow) did not contain, and thus are not in the
    // floating objects list for toBlockFlow. This can result in toBlockFlow
    // containing floats that are not in its floating objects list, but are in
    // the floating objects lists of siblings and parents. This can cause
    // problems when the float itself is deleted, since the deletion code
    // assumes that if a float is not in its containing block's floating
    // objects list, it isn't in any floating objects list. In order to
    // preserve this condition (removing it has serious performance
    // implications), we need to copy the floating objects from the old block
    // (this) to the new block (toBlockFlow). The float's metrics will likely
    // all be wrong, but since toBlockFlow is already marked for layout, this
    // will get fixed before anything gets displayed.
    // See bug https://bugs.webkit.org/show_bug.cgi?id=115566
    if (!_floatingObjects)
    {
      return;
    }

    // FIXME: Content skipped by content-visibility (isSkippedContentForLayout), once layout skips content.

    if (!toBlockFlow._floatingObjects)
    {
      toBlockFlow.CreateFloatingObjects();
    }

    for (const auto &floatingObject : _floatingObjects->Set())
    {
      if (!floatingObject->Renderer())
      {
        continue;
      }
      if (toBlockFlow.ContainsFloat(*floatingObject->Renderer()))
      {
        continue;
      }
      toBlockFlow._floatingObjects->Add(floatingObject->CloneForNewParent());
    }
  }

  void RenderBlockFlow::ComputeOverflow(LayoutUnit oldClientAfterEdge,
                                        EnumSet<ComputeOverflowOptions> options) noexcept
  {
    RenderBlock::ComputeOverflow(oldClientAfterEdge, options);

    auto addOverflowFromFloatsIfApplicable = [&]
    {
      if (!_floatingObjects)
      {
        return;
      }
      // FIXME: Not in a multi-column flow, once there are columns.
      auto shouldIncludeFloats = options.Contains(ComputeOverflowOptions::RecomputeFloats)
                                 || CreatesNewFormattingContext() || HasSelfPaintingLayer();
      if (!shouldIncludeFloats)
      {
        return;
      }

      for (const auto &floatingObject : _floatingObjects->Set())
      {
        if (floatingObject->IsDescendant())
        {
          AddOverflowFromFloatBox(*floatingObject);
        }
      }
    };
    addOverflowFromFloatsIfApplicable();
  }

  bool RenderBlockFlow::ContainsFloat(const RenderBox &renderer) const noexcept
  {
    return _floatingObjects && _floatingObjects->Set().contains(renderer);
  }

  bool RenderBlockFlow::SubtreeContainsFloat(const RenderBox &renderer) const noexcept
  {
    if (ContainsFloat(renderer))
    {
      return true;
    }

    for (const auto &blockFlow : DescendantsOfType<RenderBlockFlow>(*this))
    {
      if (blockFlow.ContainsFloat(renderer))
      {
        return true;
      }
    }

    return false;
  }

  bool RenderBlockFlow::SubtreeContainsFloats() const noexcept
  {
    if (ContainsFloats())
    {
      return true;
    }

    for (const auto &blockFlow : DescendantsOfType<RenderBlockFlow>(*this))
    {
      if (blockFlow.ContainsFloats())
      {
        return true;
      }
    }

    return false;
  }

  void RenderBlockFlow::AdjustFloatingBlock(const MarginInfo &marginInfo) noexcept
  {
    // The float should be positioned taking into account the bottom margin
    // of the previous flow. We add that margin into the height, get the
    // float positioned properly, and then subtract the margin out of the
    // height again. In the case of self-collapsing blocks, we always just
    // use the top margins, since the self-collapsing block collapsed its
    // own bottom margin into its top margin.
    //
    // Note also that the previous flow may collapse its margin into the top of
    // our block. If this is the case, then we do not add the margin in to our
    // height when computing the position of the float. This condition can be tested
    // for by simply calling canCollapseWithMarginBefore. See
    // http://www.hixie.ch/tests/adhoc/css/box/block/margin-collapse/046.html for
    // an example of this scenario.
    LayoutUnit marginOffset = marginInfo.CanCollapseWithMarginBefore() ? 0_lu : marginInfo.Margin();
    SetLogicalHeight(LogicalHeight() + marginOffset);
    PositionNewFloats();
    SetLogicalHeight(LogicalHeight() - marginOffset);
  }

  void RenderBlockFlow::CreateFloatingObjects() noexcept
  {
    _floatingObjects = CreateUnique<FloatingObjects>(*this);
  }

  void RenderBlockFlow::RemoveFloatingObjects() noexcept
  {
    if (!_floatingObjects)
    {
      return;
    }

    MarkSiblingsWithFloatsForLayout();

    _floatingObjects->Clear();
  }

  void RenderBlockFlow::InsertFloatingBoxAndMarkForLayout(RenderBox &floatBox) noexcept
  {
    // Our location is irrelevant if we're unsplittable or no pagination is in effect. Just lay out the float.
    auto *layoutState = View().FrameView().LayoutContext().LayoutState();
    bool isChildRenderBlock = floatBox.IsRenderBlock();
    if (isChildRenderBlock && !floatBox.NeedsLayout() && layoutState->PageLogicalHeightChanged())
    {
      floatBox.SetChildNeedsLayout(MarkingBehavior::MarkOnlyThis);
    }

    bool needsBlockDirectionLocationSetBeforeLayout =
      isChildRenderBlock && layoutState->NeedsBlockDirectionLocationSetBeforeLayout();
    if (!needsBlockDirectionLocationSetBeforeLayout || IsWritingModeRoot())
    {
      // We are unsplittable if we're a block flow root.
      floatBox.LayoutIfNeeded();
    }
    else
    {
      floatBox.UpdateLogicalWidth();
      floatBox.ComputeAndSetBlockDirectionMargins(*this);
    }

    auto &floatingObject = InsertFloatingBox(floatBox);
    SetLogicalWidthForFloat(floatingObject, LogicalWidthForChild(floatBox) + MarginStartForChild(floatBox)
                                              + MarginEndForChild(floatBox));
  }

  FloatingObject &RenderBlockFlow::InsertFloatingBox(RenderBox &floatBox) noexcept
  {
    krys_debug_assert(floatBox.IsFloating());

    if (!_floatingObjects)
    {
      CreateFloatingObjects();
    }

    if (auto *floatingObject = _floatingObjects->Set().find(floatBox))
    {
      return *floatingObject;
    }

    return *_floatingObjects->Add(FloatingObject::Create(floatBox));
  }

  void RenderBlockFlow::RemoveFloatingBox(RenderBox &floatBox) noexcept
  {
    if (!_floatingObjects)
    {
      return;
    }

    if (auto *floatingObject = _floatingObjects->Set().find(floatBox))
    {
      _floatingObjects->Remove(floatingObject);
    }
  }

  LayoutUnit RenderBlockFlow::LogicalLeftOffsetForPositioningFloat(LayoutUnit logicalTop,
                                                                   LayoutUnit fixedOffset,
                                                                   LayoutUnit *heightRemaining) const noexcept
  {
    LayoutUnit offset = fixedOffset;
    if (_floatingObjects && _floatingObjects->HasLeftObjects())
    {
      offset =
        _floatingObjects->LogicalLeftOffsetForPositioningFloat(fixedOffset, logicalTop, heightRemaining);
    }
    // The line grid adjustment (adjustLogicalLeftOffsetForLine) was dropped with -webkit-line-align.
    return offset;
  }

  LayoutUnit
    RenderBlockFlow::LogicalRightOffsetForPositioningFloat(LayoutUnit logicalTop, LayoutUnit fixedOffset,
                                                           LayoutUnit *heightRemaining) const noexcept
  {
    LayoutUnit offset = fixedOffset;
    if (_floatingObjects && _floatingObjects->HasRightObjects())
    {
      offset =
        _floatingObjects->LogicalRightOffsetForPositioningFloat(fixedOffset, logicalTop, heightRemaining);
    }
    // The line grid adjustment (adjustLogicalRightOffsetForLine) was dropped with -webkit-line-align.
    return offset;
  }

  void RenderBlockFlow::ComputeLogicalLocationForFloat(FloatingObject &floatingObject,
                                                       LayoutUnit &logicalTopOffset) noexcept
  {
    if (!floatingObject.Renderer())
    {
      return;
    }

    auto &childBox = *floatingObject.Renderer();
    LayoutUnit logicalLeftOffset = LogicalLeftOffsetForContent();   // Constant part of left offset.
    LayoutUnit logicalRightOffset = LogicalRightOffsetForContent(); // Constant part of right offset.

    LayoutUnit floatLogicalWidth = std::min(LogicalWidthForFloat(floatingObject),
                                            logicalRightOffset - logicalLeftOffset); // The width we look for.

    LayoutUnit floatLogicalLeft;

    // FIXME: Re-evaluate the offsets inside a fragmented flow, once there are fragmented flows.
    bool isInitialLetter = childBox.Style().GetPseudoElementType() == PseudoElementType::FirstLetter
                           && childBox.Style().InitialLetter().Drop() > 0;

    if (isInitialLetter)
    {
      if (auto lowestInitialLetterLogicalBottom = this->LowestInitialLetterLogicalBottom())
      {
        auto letterClearance = *lowestInitialLetterLogicalBottom - logicalTopOffset;
        if (letterClearance > 0)
        {
          logicalTopOffset += letterClearance;
          SetLogicalHeight(LogicalHeight() + letterClearance);
        }
      }
    }

    if (RenderStyle::GetUsedFloat(childBox) == UsedFloat::Left)
    {
      LayoutUnit heightRemainingLeft = 1_lu;
      LayoutUnit heightRemainingRight = 1_lu;
      floatLogicalLeft =
        LogicalLeftOffsetForPositioningFloat(logicalTopOffset, logicalLeftOffset, &heightRemainingLeft);
      while (
        LogicalRightOffsetForPositioningFloat(logicalTopOffset, logicalRightOffset, &heightRemainingRight)
          - floatLogicalLeft
        < floatLogicalWidth)
      {
        logicalTopOffset += std::min(heightRemainingLeft, heightRemainingRight);
        floatLogicalLeft =
          LogicalLeftOffsetForPositioningFloat(logicalTopOffset, logicalLeftOffset, &heightRemainingLeft);
      }
      floatLogicalLeft = std::max(logicalLeftOffset - BorderAndPaddingLogicalLeft(), floatLogicalLeft);
    }
    else
    {
      LayoutUnit heightRemainingLeft = 1_lu;
      LayoutUnit heightRemainingRight = 1_lu;
      floatLogicalLeft =
        LogicalRightOffsetForPositioningFloat(logicalTopOffset, logicalRightOffset, &heightRemainingRight);
      while (
        floatLogicalLeft
          - LogicalLeftOffsetForPositioningFloat(logicalTopOffset, logicalLeftOffset, &heightRemainingLeft)
        < floatLogicalWidth)
      {
        logicalTopOffset += std::min(heightRemainingLeft, heightRemainingRight);
        floatLogicalLeft =
          LogicalRightOffsetForPositioningFloat(logicalTopOffset, logicalRightOffset, &heightRemainingRight);
      }
      // Use the original width of the float here, since the local variable
      // |floatLogicalWidth| was capped to the available line width. See
      // fast/block/float/clamped-right-float.html.
      floatLogicalLeft -= LogicalWidthForFloat(floatingObject);
    }

    LayoutUnit childLogicalLeftMargin = Style().GetWritingMode().IsLogicalLeftInlineStart()
                                          ? MarginStartForChild(childBox)
                                          : MarginEndForChild(childBox);
    LayoutUnit childBeforeMargin = MarginBeforeForChild(childBox);

    if (isInitialLetter)
    {
      AdjustInitialLetterPosition(childBox, logicalTopOffset, childBeforeMargin);
    }

    SetLogicalLeftForFloat(floatingObject, floatLogicalLeft);
    SetLogicalLeftForChild(childBox, floatLogicalLeft + childLogicalLeftMargin);

    SetLogicalTopForFloat(floatingObject, logicalTopOffset);
    SetLogicalTopForChild(childBox, logicalTopOffset + childBeforeMargin);

    SetLogicalMarginsForFloat(floatingObject, childLogicalLeftMargin, childBeforeMargin);
  }

  void RenderBlockFlow::AdjustInitialLetterPosition(RenderBox &childBox, LayoutUnit &logicalTopOffset,
                                                    LayoutUnit &marginBeforeOffset) noexcept
  {
    const RenderStyle &style = FirstLineStyle();
    const FontMetrics &fontMetrics = style.MetricsOfPrimaryFont();
    if (!fontMetrics.CapHeight())
    {
      return;
    }

    LayoutUnit heightOfLine = LineHeight();
    LayoutUnit beforeMarginBorderPadding = childBox.BorderAndPaddingBefore() + childBox.MarginBefore();

    // Make an adjustment to align with the cap height of a theoretical block line.
    LayoutUnit adjustment = LayoutUnit(fontMetrics.IntAscent()) + (heightOfLine - fontMetrics.IntHeight()) / 2
                            - fontMetrics.IntCapHeight() - beforeMarginBorderPadding;
    logicalTopOffset += adjustment;

    // For sunken and raised caps, we have to make some adjustments. Test if we're sunken or raised
    // (dropHeightDelta will be positive for raised and negative for sunken).
    int32 dropHeightDelta = static_cast<int32>(childBox.Style().InitialLetter().Height())
                            - static_cast<int32>(childBox.Style().InitialLetter().Drop());

    // If we're sunken, the float needs to shift down but lines still need to avoid it. In order to do that we
    // increase the float's margin.
    if (dropHeightDelta < 0)
    {
      marginBeforeOffset += -dropHeightDelta * heightOfLine;
    }

    // If we're raised, then we actually have to grow the height of the block, since the lines have to be
    // pushed down as though we're placing empty lines beside the first letter.
    if (dropHeightDelta > 0)
    {
      SetLogicalHeight(LogicalHeight() + dropHeightDelta * heightOfLine);
    }
  }

  bool RenderBlockFlow::PositionNewFloats() noexcept
  {
    if (!_floatingObjects)
    {
      return false;
    }

    const FloatingObjectSet &floatingObjectSet = _floatingObjects->Set();
    if (floatingObjectSet.isEmpty())
    {
      return false;
    }

    // If all floats have already been positioned, then we have no work to do.
    if (floatingObjectSet.last()->IsPlaced())
    {
      return false;
    }

    // Move backwards through our floating object list until we find a float that has
    // already been positioned. Then we'll be able to move forward, positioning all of
    // the new floats that need it.
    auto it = floatingObjectSet.end();
    --it; // Go to last item.
    auto begin = floatingObjectSet.begin();
    FloatingObject *lastPlacedFloatingObject = nullptr;
    while (it != begin)
    {
      --it;
      if ((*it)->IsPlaced())
      {
        lastPlacedFloatingObject = it->get();
        ++it;
        break;
      }
    }

    LayoutUnit logicalTop = LogicalHeight();

    // The float cannot start above the top position of the last positioned float.
    if (lastPlacedFloatingObject)
    {
      logicalTop = std::max(LogicalTopForFloat(*lastPlacedFloatingObject), logicalTop);
    }

    auto end = floatingObjectSet.end();
    // Now walk through the set of unpositioned floats and place them.
    for (; it != end; ++it)
    {
      auto &floatingObject = *it->get();
      if (!floatingObject.Renderer())
      {
        continue;
      }
      // The containing block is responsible for positioning floats, so if we have floats in our
      // list that come from somewhere else, do not attempt to position them.
      auto &childBox = *floatingObject.Renderer();
      if (childBox.ContainingBlock() != this)
      {
        continue;
      }

      LayoutRect oldRect = childBox.FrameRect();
      auto childBoxUsedClear = RenderStyle::GetUsedClear(childBox);
      if (childBoxUsedClear == UsedClear::Left || childBoxUsedClear == UsedClear::Both)
      {
        logicalTop = std::max(LowestFloatLogicalBottom(FloatingObject::FloatLeft), logicalTop);
      }
      if (childBoxUsedClear == UsedClear::Right || childBoxUsedClear == UsedClear::Both)
      {
        logicalTop = std::max(LowestFloatLogicalBottom(FloatingObject::FloatRight), logicalTop);
      }

      ComputeLogicalLocationForFloat(floatingObject, logicalTop);

      // FIXME: The float's fragment range, and moving an unsplittable float or one with a pagination strut
      // down to the next page, once there's pagination.

      childBox.LayoutIfNeeded();

      SetLogicalHeightForFloat(floatingObject, LogicalHeightForChildForFragmentation(childBox)
                                                 + (LogicalTopForChild(childBox) - logicalTop)
                                                 + MarginAfterForChild(childBox));

      _floatingObjects->AddPlacedObject(&floatingObject);

      if (auto *shapeOutside = childBox.GetShapeOutsideInfo())
      {
        shapeOutside->InvalidateForSizeChangeIfNeeded();
      }
      // If the child moved, we have to repaint it.
      if (childBox.CheckForRepaintDuringLayout())
      {
        childBox.RepaintDuringLayoutIfMoved(oldRect);
      }
    }
    return true;
  }

  void RenderBlockFlow::RepaintOverhangingFloats(bool paintAllDescendants) noexcept
  {
    // Repaint any overhanging floats (if we know we're the one to paint them).
    // Otherwise, bail out.
    if (!HasOverhangingFloats())
    {
      return;
    }

    // FIXME: Avoid disabling LayoutState. At the very least, don't disable it for floats originating
    // in this block. Better yet would be to push extra state for the containers of other floats.
    LayoutStateDisabler layoutStateDisabler(View().FrameView().LayoutContext());
    for (const auto &floatingObject : _floatingObjects->Set())
    {
      if (!floatingObject->Renderer())
      {
        continue;
      }
      // Only repaint the object if it is overhanging, is not in its own layer, and
      // is our responsibility to paint (m_shouldPaint is set). When paintAllDescendants is true, the latter
      // condition is replaced with being a descendant of us.
      auto &renderer = *floatingObject->Renderer();
      if (LogicalBottomForFloat(*floatingObject) > LogicalHeight() && !renderer.HasSelfPaintingLayer()
          && (floatingObject->PaintsFloat() || (paintAllDescendants && renderer.IsDescendantOf(this))))
      {
        renderer.Repaint();
        renderer.RepaintOverhangingFloats(false);
      }
    }
  }

  void RenderBlockFlow::ClearFloats(UsedClear usedClear) noexcept
  {
    PositionNewFloats();
    // set y position
    LayoutUnit newY;
    switch (usedClear)
    {
      case UsedClear::Left:  newY = LowestFloatLogicalBottom(FloatingObject::FloatLeft); break;
      case UsedClear::Right: newY = LowestFloatLogicalBottom(FloatingObject::FloatRight); break;
      case UsedClear::Both:  newY = LowestFloatLogicalBottom(); break;
      case UsedClear::None:  break;
    }
    // FIXME: The float search tree has floored float box position (see
    // FloatingObjects::intervalForFloatingObject).
    newY = newY.Floor();
    if (Height() < newY)
    {
      SetLogicalHeight(newY);
    }
  }

  LayoutUnit RenderBlockFlow::LogicalLeftFloatOffsetForLine(LayoutUnit logicalTop, LayoutUnit fixedOffset,
                                                            LayoutUnit logicalHeight) const noexcept
  {
    if (_floatingObjects && _floatingObjects->HasLeftObjects())
    {
      return _floatingObjects->LogicalLeftOffset(fixedOffset, logicalTop, logicalHeight);
    }

    return fixedOffset;
  }

  LayoutUnit RenderBlockFlow::LogicalRightFloatOffsetForLine(LayoutUnit logicalTop, LayoutUnit fixedOffset,
                                                             LayoutUnit logicalHeight) const noexcept
  {
    if (_floatingObjects && _floatingObjects->HasRightObjects())
    {
      return _floatingObjects->LogicalRightOffset(fixedOffset, logicalTop, logicalHeight);
    }

    return fixedOffset;
  }

  LayoutUnit RenderBlockFlow::NextFloatLogicalBottomBelow(LayoutUnit logicalHeight) const noexcept
  {
    if (!_floatingObjects)
    {
      return logicalHeight;
    }

    return _floatingObjects->FindNextFloatLogicalBottomBelow(logicalHeight);
  }

  LayoutUnit RenderBlockFlow::NextFloatLogicalBottomBelowForBlock(LayoutUnit logicalHeight) const noexcept
  {
    if (!_floatingObjects)
    {
      return logicalHeight;
    }

    return _floatingObjects->FindNextFloatLogicalBottomBelowForBlock(logicalHeight);
  }

  LayoutUnit RenderBlockFlow::LowestFloatLogicalBottom(FloatingObject::Type floatType) const noexcept
  {
    if (!_floatingObjects)
    {
      return 0_lu;
    }
    LayoutUnit lowestFloatBottom;
    for (const auto &floatingObject : _floatingObjects->Set())
    {
      if (floatingObject->IsPlaced() && (floatingObject->GetType() & floatType))
      {
        lowestFloatBottom = std::max(lowestFloatBottom, LogicalBottomForFloat(*floatingObject));
      }
    }
    return lowestFloatBottom;
  }

  Maybe<LayoutUnit> RenderBlockFlow::LowestInitialLetterLogicalBottom() const noexcept
  {
    if (!_floatingObjects)
    {
      return null;
    }
    auto lowestFloatBottom = Maybe<LayoutUnit> {};
    for (const auto &floatingObject : _floatingObjects->Set())
    {
      if (!floatingObject->Renderer())
      {
        continue;
      }
      if (floatingObject->IsPlaced()
          && floatingObject->Renderer()->Style().GetPseudoElementType() == PseudoElementType::FirstLetter
          && floatingObject->Renderer()->Style().InitialLetter().Drop() > 0)
      {
        lowestFloatBottom =
          std::max(lowestFloatBottom.value_or(0_lu), LogicalBottomForFloat(*floatingObject));
      }
    }
    return lowestFloatBottom;
  }

  namespace
  {
    KRYS_NODISCARD RenderLayer *EnclosingFloatPaintingLayer(const RenderBox &renderer) noexcept
    {
      for (const RenderElement *ancestor = &renderer; ancestor; ancestor = ancestor->Parent())
      {
        if (auto *box = DynamicDowncast<RenderBox>(*ancestor);
            box && box->Layer() && box->Layer()->IsSelfPaintingLayer())
        {
          return box->Layer();
        }
      }
      return nullptr;
    }
  }

  LayoutUnit RenderBlockFlow::AddOverhangingFloats(RenderBlockFlow &child,
                                                   bool makeChildPaintOtherFloats) noexcept
  {
    // Prevent floats from being added to the canvas by the root element, e.g., <html>.
    if (!child.ContainsFloats() || child.CreatesNewFormattingContext())
    {
      return 0_lu;
    }

    LayoutUnit childLogicalTop = child.LogicalTop();
    LayoutUnit childLogicalLeft = child.LogicalLeft();
    LayoutUnit lowestFloatLogicalBottom;

    // Floats that will remain the child's responsibility to paint should factor into its
    // overflow.
    auto blockHasOverflowClip =
      EffectiveOverflowX() == Overflow::Clip || EffectiveOverflowY() == Overflow::Clip;
    for (const auto &floatingObject : child._floatingObjects->Set())
    {
      if (!floatingObject->Renderer())
      {
        continue;
      }
      LayoutUnit floatLogicalBottom =
        std::min(LogicalBottomForFloat(*floatingObject), LayoutUnit::Max() - childLogicalTop);
      LayoutUnit logicalBottom = childLogicalTop + floatLogicalBottom;
      lowestFloatLogicalBottom = std::max(lowestFloatLogicalBottom, logicalBottom);
      auto &renderer = *floatingObject->Renderer();

      if (logicalBottom > LogicalHeight())
      {
        // If the object is not in the list, we add it now.
        if (!ContainsFloat(renderer))
        {
          LayoutSize offset = IsHorizontalWritingMode() ? LayoutSize(-childLogicalLeft, -childLogicalTop)
                                                        : LayoutSize(-childLogicalTop, -childLogicalLeft);
          bool shouldPaint = false;

          // The nearest enclosing layer always paints the float (so that zindex and stacking
          // behaves properly). We always want to propagate the desire to paint the float as
          // far out as we can, to the outermost block that overlaps the float, stopping only
          // if we hit a self-painting layer boundary.
          if (!floatingObject->HasAncestorWithOverflowClip()
              && EnclosingFloatPaintingLayer(renderer) == EnclosingFloatPaintingLayer(*this))
          {
            floatingObject->SetPaintsFloat(false);
            shouldPaint = true;
          }
          // We create the floating object list lazily.
          if (!_floatingObjects)
          {
            CreateFloatingObjects();
          }

          _floatingObjects->Add(floatingObject->CopyToNewContainer(
            offset, shouldPaint, true,
            floatingObject->HasAncestorWithOverflowClip() || blockHasOverflowClip));
        }
      }
      else
      {
        if (makeChildPaintOtherFloats && !floatingObject->PaintsFloat() && !renderer.HasSelfPaintingLayer()
            && renderer.IsDescendantOf(&child)
            && EnclosingFloatPaintingLayer(renderer) == EnclosingFloatPaintingLayer(child))
        {
          // The float is not overhanging from this block, so if it is a descendant of the child, the child
          // should paint it (the other case is that it is intruding into the child), unless it has its own
          // layer or enclosing layer. If makeChildPaintOtherFloats is false, it means that the child must
          // already know about all the floats it should paint.
          floatingObject->SetPaintsFloat(true);
        }

        // Since the float doesn't overhang, it didn't get put into our list. We need to add its overflow in
        // to the child now.
        if (floatingObject->IsDescendant())
        {
          child.AddOverflowFromFloatBox(*floatingObject);
        }
      }
    }
    return lowestFloatLogicalBottom;
  }

  bool RenderBlockFlow::HasOverhangingFloat(RenderBox &renderer) const noexcept
  {
    if (!_floatingObjects || !Parent())
    {
      return false;
    }

    auto *floatingObject = _floatingObjects->Set().find(renderer);
    if (!floatingObject)
    {
      return false;
    }

    return LogicalBottomForFloat(*floatingObject) > LogicalHeight();
  }

  void RenderBlockFlow::AddIntrudingFloats(RenderBlockFlow *previousBlock, RenderBlockFlow *container,
                                           LayoutUnit logicalLeftOffset, LayoutUnit logicalTopOffset) noexcept
  {
    krys_debug_assert(!AvoidsFloats());

    // If we create our own block formatting context then our contents don't interact with floats outside it,
    // even those from our parent.
    if (CreatesNewFormattingContext())
    {
      return;
    }

    // If the parent or previous sibling doesn't have any floats to add, don't bother.
    if (!previousBlock->_floatingObjects)
    {
      return;
    }

    logicalLeftOffset += MarginLogicalLeft();

    for (const auto &previousBlockFloatingObject : previousBlock->_floatingObjects->Set())
    {
      if (!previousBlockFloatingObject->Renderer())
      {
        continue;
      }

      if (LogicalBottomForFloat(*previousBlockFloatingObject) > logicalTopOffset)
      {
        if (!_floatingObjects || !_floatingObjects->Set().contains(*previousBlockFloatingObject->Renderer()))
        {
          // We create the floating object list lazily.
          if (!_floatingObjects)
          {
            CreateFloatingObjects();
          }

          // Applying the child's margin makes no sense in the case where the child was passed in.
          // since this margin was added already through the modification of the |logicalLeftOffset| variable
          // above. |logicalLeftOffset| will equal the margin in this case, so it's already been taken
          // into account. Only apply this code if previousBlock is the parent, since otherwise the left
          // margin will get applied twice.
          LayoutSize offset =
            IsHorizontalWritingMode()
              ? LayoutSize(logicalLeftOffset
                             - (previousBlock != container ? previousBlock->MarginLeft() : 0_lu),
                           logicalTopOffset)
              : LayoutSize(logicalTopOffset,
                           logicalLeftOffset
                             - (previousBlock != container ? previousBlock->MarginTop() : 0_lu));

          _floatingObjects->Add(previousBlockFloatingObject->CopyToNewContainer(offset));
        }
      }
    }
  }

  void RenderBlockFlow::MarkAllDescendantsWithFloatsForLayout(RenderBox *floatToRemove,
                                                              bool inLayout) noexcept
  {
    if (!EverHadLayout() && !ContainsFloats())
    {
      return;
    }

    MarkingBehavior markParents =
      inLayout ? MarkingBehavior::MarkOnlyThis : MarkingBehavior::MarkContainingBlockChain;
    SetChildNeedsLayout(markParents);

    if (floatToRemove)
    {
      // FIXME: Clear the layout bits of content skipped by content-visibility instead, once layout skips
      // content.
      RemoveFloatingBox(*floatToRemove);
    }
    else if (ChildrenInline())
    {
      return;
    }

    // Iterate over our block children and mark them as needed.
    for (auto walker = InlineWalker(*this); !walker.AtEnd(); walker.Advance())
    {
      auto *block = DynamicDowncast<RenderBlock>(walker.Current());
      if (!block)
      {
        continue;
      }

      if (!floatToRemove && block->IsFloatingOrOutOfFlowPositioned())
      {
        continue;
      }
      auto *blockFlow = DynamicDowncast<RenderBlockFlow>(*block);
      if (!blockFlow)
      {
        if (block->ShrinkToAvoidFloats() && block->EverHadLayout())
        {
          block->SetChildNeedsLayout(markParents);
        }
        continue;
      }
      auto shouldCheckSubtree = IsSkippedContentRootOrSkippedContent(*blockFlow)
                                || (floatToRemove ? blockFlow->SubtreeContainsFloat(*floatToRemove)
                                                  : blockFlow->SubtreeContainsFloats())
                                || blockFlow->ShrinkToAvoidFloats();
      if (shouldCheckSubtree)
      {
        blockFlow->MarkAllDescendantsWithFloatsForLayout(floatToRemove, inLayout);
      }
    }
  }

  void RenderBlockFlow::MarkSiblingsWithFloatsForLayout(RenderBox *floatToRemove) noexcept
  {
    krys_debug_assert(!floatToRemove || floatToRemove->IsFloating());

    auto markSiblingsWithIntrusiveFloatForLayoutIfApplicable = [&](RenderBox &floatBoxToRemove)
    {
      for (auto *nextSibling = this->NextSibling(); nextSibling; nextSibling = nextSibling->NextSibling())
      {
        auto *nextSiblingBlockFlow = DynamicDowncast<RenderBlockFlow>(*nextSibling);
        if (!nextSiblingBlockFlow)
        {
          continue;
        }
        auto shouldCheckSubtree = IsSkippedContentRootOrSkippedContent(*nextSiblingBlockFlow)
                                  || nextSiblingBlockFlow->ContainsFloat(floatBoxToRemove);
        if (shouldCheckSubtree)
        {
          nextSiblingBlockFlow->MarkAllDescendantsWithFloatsForLayout(&floatBoxToRemove);
        }
      }
    };

    if (floatToRemove)
    {
      markSiblingsWithIntrusiveFloatForLayoutIfApplicable(*floatToRemove);
      return;
    }

    if (!_floatingObjects)
    {
      return;
    }

    for (const auto &floatingObject : _floatingObjects->Set())
    {
      if (!floatingObject->Renderer())
      {
        continue;
      }
      markSiblingsWithIntrusiveFloatForLayoutIfApplicable(*floatingObject->Renderer());
    }
  }

  LayoutPoint RenderBlockFlow::FlipFloatForWritingModeForChild(const FloatingObject &child,
                                                               const LayoutPoint &point) const noexcept
  {
    if (!child.Renderer())
    {
      return point;
    }

    if (!Style().GetWritingMode().IsBlockFlipped())
    {
      return point;
    }

    // This is similar to RenderBox::flipForWritingModeForChild. We have to subtract out our left/top offsets
    // twice, since it's going to get added back in. We hide this complication here so that the calling code
    // looks normal for the unflipped case.
    if (IsHorizontalWritingMode())
    {
      return LayoutPoint(point.X(), point.Y() + Height() - child.Renderer()->Height()
                                      - 2 * child.LocationOffsetOfBorderBox().Height());
    }
    return LayoutPoint(point.X() + Width() - child.Renderer()->Width()
                         - 2 * child.LocationOffsetOfBorderBox().Width(),
                       point.Y());
  }

  LayoutUnit RenderBlockFlow::ComputedClearDeltaForChild(RenderBox &child, LayoutUnit logicalTop) noexcept
  {
    // There is no need to compute clearance if we have no floats.
    if (!ContainsFloats())
    {
      return 0_lu;
    }

    // At least one float is present. We need to perform the clearance computation.
    UsedClear usedClear = RenderStyle::GetUsedClear(child);
    bool clearSet = usedClear != UsedClear::None;
    LayoutUnit logicalBottom;
    switch (usedClear)
    {
      case UsedClear::None:  break;
      case UsedClear::Left:  logicalBottom = LowestFloatLogicalBottom(FloatingObject::FloatLeft); break;
      case UsedClear::Right: logicalBottom = LowestFloatLogicalBottom(FloatingObject::FloatRight); break;
      case UsedClear::Both:  logicalBottom = LowestFloatLogicalBottom(); break;
    }

    // We also clear floats if we are too big to sit on the same line as a float (and wish to avoid floats by
    // default).
    LayoutUnit result = clearSet ? std::max<LayoutUnit>(0_lu, logicalBottom - logicalTop) : 0_lu;
    if (!result && child.AvoidsFloats())
    {
      LayoutUnit newLogicalTop = logicalTop;
      while (true)
      {
        LayoutUnit availableLogicalWidthAtNewLogicalTopOffset =
          AvailableLogicalWidthForLine(newLogicalTop, LogicalHeightForChild(child));
        if (availableLogicalWidthAtNewLogicalTopOffset == AvailableLogicalWidthForContent())
        {
          return newLogicalTop - logicalTop;
        }

        LayoutRect borderBox = child.BorderBoxRect();
        LayoutUnit childLogicalWidthAtOldLogicalTopOffset =
          IsHorizontalWritingMode() ? borderBox.Width() : borderBox.Height();

        // FIXME: None of this is right for perpendicular writing-mode children.
        LayoutUnit childOldLogicalWidth = child.LogicalWidth();
        LayoutUnit childOldMarginLeft = child.MarginLeft();
        LayoutUnit childOldMarginRight = child.MarginRight();
        LayoutUnit childOldLogicalTop = child.LogicalTop();

        child.SetLogicalTop(newLogicalTop);
        child.UpdateLogicalWidth();
        borderBox = child.BorderBoxRect();
        LayoutUnit childLogicalWidthAtNewLogicalTopOffset =
          IsHorizontalWritingMode() ? borderBox.Width() : borderBox.Height();

        child.SetLogicalTop(childOldLogicalTop);
        child.SetLogicalWidth(childOldLogicalWidth);
        child.SetMarginLeft(childOldMarginLeft);
        child.SetMarginRight(childOldMarginRight);

        if (childLogicalWidthAtNewLogicalTopOffset <= availableLogicalWidthAtNewLogicalTopOffset)
        {
          // Even though we may not be moving, if the logical width did shrink because of the presence of new
          // floats, then we need to force a relayout as though we shifted. This happens because of the
          // dynamic addition of overhanging floats from previous siblings when negative margins exist on a
          // child (see the addOverhangingFloats call at the end of collapseMargins).
          if (childLogicalWidthAtOldLogicalTopOffset != childLogicalWidthAtNewLogicalTopOffset)
          {
            child.SetChildNeedsLayout(MarkingBehavior::MarkOnlyThis);
          }
          return newLogicalTop - logicalTop;
        }

        newLogicalTop = NextFloatLogicalBottomBelowForBlock(newLogicalTop);
        krys_debug_assert(newLogicalTop >= logicalTop);
        if (newLogicalTop < logicalTop)
        {
          break;
        }
      }
      krys_debug_assert(false);
    }
    return result;
  }

  void RenderBlockFlow::AddOverflowFromInFlowChildren(EnumSet<ComputeOverflowOptions> options) noexcept
  {
    if (ChildrenInline())
    {
      if (auto *inlineLayout = InlineLayout())
      {
        inlineLayout->CollectOverflow();
      }
      // FIXME: Propagate the visual overflow to the containing fragments, once there are fragmented flows.
      return;
    }
    RenderBlock::AddOverflowFromInFlowChildren(options);
  }

  namespace
  {
    KRYS_NODISCARD float LineHeightForEmptyContent(const RenderStyle &style,
                                                   bool shouldNotRoundToIntegral) noexcept
    {
      const auto &fontMetrics = style.MetricsOfPrimaryFont();
      auto ascent =
        shouldNotRoundToIntegral ? fontMetrics.Ascent() : static_cast<float>(fontMetrics.IntAscent());
      auto fontHeight =
        shouldNotRoundToIntegral ? fontMetrics.Height() : static_cast<float>(fontMetrics.IntHeight());
      return ascent + (style.ComputedLineHeight() - fontHeight) / 2.f;
    }
  }

  Maybe<LayoutUnit> RenderBlockFlow::FirstLineBaseline() const noexcept
  {
    if (IsWritingModeRoot() && !IsGridItem() && !IsFlexItem())
    {
      return null;
    }

    if (ShouldApplyLayoutContainment())
    {
      return null;
    }

    if (!ChildrenInline())
    {
      return RenderBlock::FirstLineBaseline();
    }

    if (auto *lineLayout = InlineLayout())
    {
      return lineLayout->FirstLineBaseline();
    }

    if (HasLineIfEmpty())
    {
      // FIXME: Don't round when subpixel inline layout is enabled.
      return LayoutUnit {BorderAndPaddingBefore() + LineHeightForEmptyContent(FirstLineStyle(), false)};
    }

    return null;
  }

  Maybe<LayoutUnit> RenderBlockFlow::LastLineBaseline() const noexcept
  {
    if (IsWritingModeRoot() && !IsGridItem() && !IsFlexItem())
    {
      return null;
    }

    if (ShouldApplyLayoutContainment())
    {
      return null;
    }

    if (!ChildrenInline())
    {
      return RenderBlock::LastLineBaseline();
    }

    if (auto *lineLayout = InlineLayout())
    {
      return lineLayout->LastLineBaseline();
    }

    if (HasLineIfEmpty())
    {
      // FIXME: Don't round when subpixel inline layout is enabled.
      return LayoutUnit {BorderAndPaddingBefore() + LineHeightForEmptyContent(Style(), false)};
    }

    return null;
  }

  RenderBlockFlowRareData &RenderBlockFlow::EnsureRareBlockFlowData() noexcept
  {
    if (HasRareBlockFlowData())
    {
      return *_rareBlockFlowData;
    }
    MaterializeRareBlockFlowData();
    return *_rareBlockFlowData;
  }

  void RenderBlockFlow::MaterializeRareBlockFlowData() noexcept
  {
    krys_debug_assert(!HasRareBlockFlowData());
    _rareBlockFlowData = CreateUnique<RenderBlockFlowRareData>(*this);
  }

  LayoutOptionalOutsets RenderBlockFlow::AllowedLayoutOverflow() const noexcept
  {
    LayoutOptionalOutsets allowance = RenderBox::AllowedLayoutOverflow();

    if (!Style().GetAlignContent().IsNormal())
    {
      if (HasRareBlockFlowData())
      {
        if (IsHorizontalWritingMode())
        {
          allowance.Top() = -RareBlockFlowData()->AlignContentShift;
        }
        else
        {
          allowance.Left() = -RareBlockFlowData()->AlignContentShift;
        }
      }
    }

    // FIXME: Flip the allowance for a reversed column progression, once there's multi-column layout.

    return allowance;
  }

  void RenderBlockFlow::SetChildrenInline(bool value) noexcept
  {
    if (ChildrenInline() && !value)
    {
      SetLineLayoutPath(LineLayoutPath::UndeterminedPath);
      _lineLayout = nullptr;
    }

    RenderBlock::SetChildrenInline(value);
  }

  RenderBlockFlow::BlockPositionAndMargin
    RenderBlockFlow::LayoutBlockChildFromInlineLayout(RenderBox &child, LayoutUnit contentHeight,
                                                      MarginInfo marginInfo) noexcept
  {
    // Render tree uses block height to track the child block layout position. Set it to the current position
    // before calling LayoutBlockChild.
    SetLogicalHeight(contentHeight);

    auto previousFloatLogicalBottom = LayoutUnit {};
    auto maxFloatLogicalBottom = LayoutUnit {};
    LayoutBlockChild(child, marginInfo, previousFloatLogicalBottom, maxFloatLogicalBottom);
    return {child.LogicalTop(), LogicalHeight(), marginInfo};
  }

  void RenderBlockFlow::ComputeAndSetLineLayoutPath() noexcept
  {
    if (GetLineLayoutPath() != LineLayoutPath::UndeterminedPath)
    {
      return;
    }
    // Without SVG text, inline content always uses the inline layout.
    krys_debug_assert(layout_integration::LineLayout::CanUseFor(*this));
    SetLineLayoutPath(LineLayoutPath::InlinePath);
  }

  bool RenderBlockFlow::HasContentfulInlineOrBlockLine() const noexcept
  {
    return InlineLayout() ? InlineLayout()->HasContentfulInlineOrBlockLine() : false;
  }

  bool RenderBlockFlow::HasContentfulInlineLine() const noexcept
  {
    return InlineLayout() ? InlineLayout()->HasContentfulInlineLine() : false;
  }

  bool RenderBlockFlow::HasBlocksInInlineLayout() const noexcept
  {
    auto *inlineLayout = InlineLayout();
    return inlineLayout && inlineLayout->HasBlocks();
  }

  void RenderBlockFlow::InvalidateLineLayout(InvalidationReason invalidationReason) noexcept
  {
    if (GetLineLayoutPath() == LineLayoutPath::UndeterminedPath)
    {
      return;
    }

    auto issueNeedsLayoutIfApplicable = [&]
    {
      if (SelfNeedsLayout() || NormalChildNeedsLayout())
      {
        return;
      }
      // FIXME: We should just kick off a subtree layout here (if needed at all).
      SetNeedsLayout();
    };

    if (InlineLayout())
    {
      krys_debug_assert(!_previousInlineLayoutContentTopAndBottomIncludingInkOverflow);
      _previousInlineLayoutContentTopAndBottomIncludingInkOverflow =
        InlineContentTopAndBottomIncludingInkOverflow();
    }

    switch (invalidationReason)
    {
      case InvalidationReason::InternalMove:
        // FIXME: Defer recomputing whether the element is ignored (the accessibility cache), once there is
        // one.
        break;
      case InvalidationReason::ContentChange:
      {
        // Since we eagerly remove the display content here, repaints issued between this invalidation
        // (triggered by style change/content mutation) and the subsequent layout would produce empty rects.
        Repaint();
        for (auto walker = InlineWalker(*this); !walker.AtEnd(); walker.Advance())
        {
          auto &renderer = *walker.Current();
          if (!renderer.EverHadLayout())
          {
            continue;
          }
          if (!renderer.IsInFlow() && InlineLayout()->Contains(Downcast<RenderElement>(renderer)))
          {
            renderer.Repaint();
          }
          renderer.SetNeedsPreferredWidthsUpdate();
        }
        issueNeedsLayoutIfApplicable();
        break;
      }
      case InvalidationReason::InsertionOrRemoval:
        SetLineLayoutPath(LineLayoutPath::UndeterminedPath);
        issueNeedsLayoutIfApplicable();
        break;
    }

    _lineLayout = nullptr;
  }

  size_t RenderBlockFlow::LineCount() const noexcept
  {
    if (!ChildrenInline())
    {
      krys_debug_assert(false);
      return 0;
    }
    if (InlineLayout())
    {
      return InlineLayout()->LineCount();
    }

    return 0;
  }

  bool RenderBlockFlow::LayoutSimpleBlockContentInInline(MarginInfo &marginInfo) noexcept
  {
    if (!InlineLayout())
    {
      return false;
    }

    for (auto walker = InlineWalker(*this); !walker.AtEnd(); walker.Advance())
    {
      krys_debug_assert(!walker.Current()->SelfNeedsLayout());

      auto *blockRenderer = DynamicDowncast<RenderBox>(*walker.Current());
      if (!blockRenderer || !blockRenderer->IsBlockLevelBox())
      {
        continue;
      }

      auto logicalHeight = blockRenderer->LogicalHeight();
      auto isEligibleForBlockOnlyLayout = [&]
      {
        // Do not interfere with margin collapsing.
        if (!blockRenderer->IsInFlow() || !logicalHeight)
        {
          return false;
        }
        if (auto *renderBlock = DynamicDowncast<RenderBlock>(*blockRenderer))
        {
          return !renderBlock->ContainsFloats();
        }
        return true;
      };
      if (!isEligibleForBlockOnlyLayout())
      {
        return false;
      }

      auto displayBox = inline_iterator::BoxFor(*blockRenderer);
      if (!displayBox)
      {
        krys_debug_assert(false);
        return false;
      }

      auto logicalTop = blockRenderer->LogicalTop();
      marginInfo = LayoutBlockChildFromInlineLayout(*blockRenderer, logicalTop, marginInfo).ChildMarginInfo;
      auto shouldFallbackToNormalInlineLayout = [&]
      {
        if (logicalHeight != blockRenderer->LogicalHeight())
        {
          return true;
        }
        if (auto *renderBlock = DynamicDowncast<RenderBlock>(*blockRenderer))
        {
          return renderBlock->ContainsFloats();
        }
        return false;
      };
      if (shouldFallbackToNormalInlineLayout())
      {
        return false;
      }
      blockRenderer->SetLogicalTop(logicalTop);
    }
    return true;
  }

  namespace
  {
    KRYS_NODISCARD bool
      HasSimpleStaticPositionForInlineLevelOutOfFlowChildrenByStyle(const RenderStyle &rootStyle) noexcept
    {
      if (rootStyle.GetTextAlign() != style::TextAlign::Start)
      {
        return false;
      }
      if (!rootStyle.GetTextIndent().Length.IsKnownZero())
      {
        return false;
      }
      return true;
    }
  }

  Pair<float, float> RenderBlockFlow::InlineContentTopAndBottomIncludingInkOverflow() const noexcept
  {
    if (_previousInlineLayoutContentTopAndBottomIncludingInkOverflow)
    {
      return *_previousInlineLayoutContentTopAndBottomIncludingInkOverflow;
    }

    if (!InlineLayout())
    {
      return {};
    }

    auto firstLineBox = inline_iterator::FirstLineBoxFor(*this);
    auto lastLineBox = inline_iterator::LastLineBoxFor(*this);
    if (!firstLineBox)
    {
      return {};
    }

    auto logicalTop = std::min(firstLineBox->LogicalTop(), firstLineBox->ContentLogicalTop());
    auto logicalBottom = std::max(lastLineBox->LogicalBottom(), lastLineBox->ContentLogicalBottom());

    if (!InlineLayout()->HasInkOverflow())
    {
      return {logicalTop, logicalBottom};
    }

    for (auto lineBox = firstLineBox; lineBox; lineBox.TraverseNext())
    {
      logicalTop = std::min(logicalTop, lineBox->InkOverflowLogicalTop());
      logicalBottom = std::max(logicalBottom, lineBox->InkOverflowLogicalBottom());
    }
    return {logicalTop, logicalBottom};
  }

  namespace
  {
    void SetFullRepaintOnParentInlineBoxLayerIfNeeded(const RenderText &renderer) noexcept
    {
      // Repaints (on self) are normally issued either during layout using LayoutRepainter inside ::layout()
      // functions (#1) or after layout, while recursing the layer tree (#2).
      // Additionally, repaint at the block level (#3) takes care of regular in-flow content.
      // However in case of text content, we don't have (#1), (#2) is primarily a geometry diff type of
      // repaint meaning no repaint happens unless content size changes (or full repaint bit is set on the
      // layer) and (#3) only works when the block container and the text content share the same layer. Here
      // we mark the parent inline box's layer dirty to trigger repaint at (#2).
      if (!renderer.NeedsLayout())
      {
        return;
      }
      auto *parent = renderer.Parent();
      if (!parent)
      {
        krys_debug_assert(false);
        return;
      }
      if (!parent->IsInline() || !parent->HasLayer())
      {
        return;
      }
      Downcast<RenderLayerModelObject>(*parent).Layer()->SetRepaintStatus(RepaintStatus::NeedsFullRepaint);
    }
  }

  RenderBlockFlow::InlineContentStatus
    RenderBlockFlow::MarkInlineContentDirtyForLayout(RelayoutChildren relayoutChildren) noexcept
  {
    auto contentNeedsNormalChildLayoutOnly = true;
    auto hasInFlowBlockLevelElement = false;
    auto hasSimpleOutOfFlowContentOnly = !HasLineIfEmpty();
    auto hasSimpleStaticPositionForInlineLevelOutOfFlowContentByStyle =
      HasSimpleStaticPositionForInlineLevelOutOfFlowChildrenByStyle(Style());

    for (auto walker = InlineWalker(*this); !walker.AtEnd(); walker.Advance())
    {
      auto &renderer = *walker.Current();
      auto *box = DynamicDowncast<RenderBox>(renderer);
      hasInFlowBlockLevelElement =
        hasInFlowBlockLevelElement || (box && box->IsBlockLevelBox() && box->IsInFlow());
      auto childNeedsLayout =
        relayoutChildren == RelayoutChildren::Yes || (box && box->HasRelativeDimensions());
      auto childNeedsPreferredWidthComputation =
        relayoutChildren == RelayoutChildren::Yes && box && box->ShouldInvalidatePreferredWidths();
      if (childNeedsLayout)
      {
        renderer.SetNeedsLayout(MarkingBehavior::MarkOnlyThis);
      }
      if (childNeedsPreferredWidthComputation)
      {
        renderer.SetNeedsPreferredWidthsUpdate(MarkingBehavior::MarkOnlyThis);
      }

      if (renderer.IsOutOfFlowPositioned())
      {
        renderer.ContainingBlock()->AddOutOfFlowBox(*box);
        // FIXME: This is only needed because of the synchronous layout call in
        // SetStaticPositionsForSimpleOutOfFlowContent which itself appears to be a workaround for a bad
        // subtree layout.
        auto &style = Downcast<RenderElement>(renderer).Style();
        auto hasParentRelativeHeightOrTop = [&]
        {
          if (style.LogicalHeight().IsPercentOrCalculated() || style.LogicalTop().IsPercentOrCalculated())
          {
            return true;
          }
          return !style.LogicalBottom().IsAuto();
        }();
        if (hasParentRelativeHeightOrTop)
        {
          hasSimpleOutOfFlowContentOnly = false;
        }

        if (hasSimpleOutOfFlowContentOnly && style.IsOriginalDisplayInlineType())
        {
          hasSimpleOutOfFlowContentOnly = hasSimpleStaticPositionForInlineLevelOutOfFlowContentByStyle;
        }
      }
      else
      {
        hasSimpleOutOfFlowContentOnly = false;
      }

      if (!renderer.NeedsLayout() && !renderer.NeedsPreferredLogicalWidthsUpdate())
      {
        continue;
      }

      if (auto *renderText = DynamicDowncast<RenderText>(renderer))
      {
        SetFullRepaintOnParentInlineBoxLayerIfNeeded(*renderText);
      }

      if (auto *inlineLevelBox = DynamicDowncast<RenderBox>(renderer))
      {
        // FIXME: Move this to where the actual content change happens and call it on the parent IFC.
        auto shouldTriggerFullLayout =
          inlineLevelBox->IsInline()
          && (inlineLevelBox->NeedsSimplifiedNormalFlowLayout() || inlineLevelBox->NormalChildNeedsLayout()
              || inlineLevelBox->OutOfFlowChildNeedsLayout())
          && InlineLayout();
        if (shouldTriggerFullLayout)
        {
          (void)InlineLayout()->BoxContentWillChange(*inlineLevelBox);
          contentNeedsNormalChildLayoutOnly = false;
        }
      }

      // Non inline box inline level elements (e.g. <img>) report self-needs-layout on style change, and
      // RenderText also reports self-needs-layout on content change. Inline boxes report
      // normal-child-needs-layout when their children need (any) layout.
      contentNeedsNormalChildLayoutOnly =
        contentNeedsNormalChildLayoutOnly
        && (!renderer.NeedsLayout() || renderer.NeedsNormalChildOrSimplifiedLayoutOnly());

      if (Is<RenderLineBreak>(renderer) || Is<RenderInline>(renderer) || Is<RenderText>(renderer))
      {
        renderer.ClearNeedsLayout();
      }

      // FIXME: Combine the text of text-combine-upright text (RenderCombineText::combineTextIfNeeded), once
      // there's text-combine-upright.
    }
    return {hasSimpleOutOfFlowContentOnly,
            hasInFlowBlockLevelElement ? Maybe<bool> {contentNeedsNormalChildLayoutOnly} : null};
  }

  void RenderBlockFlow::UpdateRepaintTopAndBottomAfterLayout(
    RelayoutChildren relayoutChildren, Maybe<LayoutRect> partialRepaintRect,
    Pair<float, float> oldContentTopAndBottomIncludingInkOverflow, LayoutUnit &repaintLogicalTop,
    LayoutUnit &repaintLogicalBottom) noexcept
  {
    auto isFullLayout = SelfNeedsLayout() || relayoutChildren == RelayoutChildren::Yes;
    if (isFullLayout)
    {
      if (!SelfNeedsLayout())
      {
        // In order to really trigger full repaint, the block container has to have the self layout flag set.
        // Without having it set, repaint after layout logic only issues repaint on the diff of before/after
        // repaint bounds. It results in incorrect repaint when the inline content changes (new text) and
        // expands the same time. (it only affects shrink-to-fit type of containers).
        // FIXME: We have the exact damaged rect here, should be able to issue repaint on both inline and
        // block directions.
        SetNeedsLayout(MarkingBehavior::MarkOnlyThis);
      }
      // Let's trigger full repaint instead for now.
      // FIXME: We should revisit this behavior and run repaints strictly on visual overflow.
      repaintLogicalTop = LayoutUnit {};
      repaintLogicalBottom = LayoutUnit {};
      return;
    }

    if (partialRepaintRect)
    {
      repaintLogicalTop = partialRepaintRect->Y();
      repaintLogicalBottom = partialRepaintRect->MaxY();
      return;
    }

    auto contentTopAndBottomIncludingInkOverflow = InlineContentTopAndBottomIncludingInkOverflow();
    auto damageTopIncludingInkOverflow = std::min(oldContentTopAndBottomIncludingInkOverflow.first,
                                                  contentTopAndBottomIncludingInkOverflow.first);
    auto damageBottomIncludingInkOverflow = std::max(oldContentTopAndBottomIncludingInkOverflow.second,
                                                     contentTopAndBottomIncludingInkOverflow.second);

    repaintLogicalTop =
      std::min(LayoutUnit::FromFloatFloor(damageTopIncludingInkOverflow), BorderAndPaddingBefore());
    repaintLogicalBottom =
      std::max(LayoutUnit::FromFloatCeil(damageBottomIncludingInkOverflow), LogicalHeight());
  }

  void RenderBlockFlow::LayoutInlineContent(RelayoutChildren relayoutChildren, LayoutUnit previousHeight,
                                            LayoutUnit &repaintLogicalTop,
                                            LayoutUnit &repaintLogicalBottom) noexcept
  {
    auto inlineContentStatus = MarkInlineContentDirtyForLayout(relayoutChildren);
    if (inlineContentStatus.HasSimpleOutOfFlowContentOnly)
    {
      // Shortcut the layout.
      _lineLayout = nullptr;

      SetStaticPositionsForSimpleOutOfFlowContent();
      SetLogicalHeight(BorderAndPaddingLogicalHeight() + LayoutUnit(ScrollbarLogicalHeight()));
      return;
    }

    auto oldContentTopAndBottomIncludingInkOverflow = InlineContentTopAndBottomIncludingInkOverflow();
    _previousInlineLayoutContentTopAndBottomIncludingInkOverflow = null;

    auto mayRunSimpleBlockContentInInlineLayout =
      inlineContentStatus.OnlyBlockContentNeedsLayout && *inlineContentStatus.OnlyBlockContentNeedsLayout;
    if (mayRunSimpleBlockContentInInlineLayout)
    {
      auto marginInfo = MarginInfo {*this, MarginInfo::IgnoreScrollbarForAfterMargin::No};
      if (LayoutSimpleBlockContentInInline(marginInfo))
      {
        SetLogicalHeight(previousHeight);
        (void)HandleAfterSideOfBlock(
          marginInfo,
          previousHeight - (BorderAndPaddingLogicalHeight() + LayoutUnit(ScrollbarLogicalHeight())));
        UpdateRepaintTopAndBottomAfterLayout(relayoutChildren, null,
                                             oldContentTopAndBottomIncludingInkOverflow, repaintLogicalTop,
                                             repaintLogicalBottom);
        return;
      }
    }

    if (!InlineLayout())
    {
      _lineLayout = CreateUnique<layout_integration::LineLayout>(*this);
    }
    auto &inlineLayout = *InlineLayout();

    krys_debug_assert(ContainingBlock() || Is<RenderView>(*this));
    inlineLayout.UpdateFormattingContexGeometries(ContainingBlock() ? ContainingBlockLogicalWidthForContent()
                                                                    : LayoutUnit());

    auto marginInfo = MarginInfo {*this, MarginInfo::IgnoreScrollbarForAfterMargin::No};
    auto partialRepaintRect =
      inlineLayout.Layout(marginInfo, relayoutChildren == RelayoutChildren::Yes
                                        ? layout_integration::LineLayout::ForceFullLayout::Yes
                                        : layout_integration::LineLayout::ForceFullLayout::No);

    // The legacy line clamp (-webkit-line-clamp) was dropped, so the content is never clamped here
    // (updateLineClampStateAndLogicalHeightAfterLayout).
    auto contentBoxHeight = [&]() -> LayoutUnit
    {
      if (HasContentfulInlineOrBlockLine())
      {
        return inlineLayout.ContentLogicalHeight();
      }
      if (HasLineIfEmpty())
      {
        return LineHeight();
      }
      return {};
    };
    auto borderBoxLogicalHeight = HandleAfterSideOfBlock(marginInfo, contentBoxHeight());
    SetLogicalHeight(borderBoxLogicalHeight);
    UpdateRepaintTopAndBottomAfterLayout(relayoutChildren, partialRepaintRect,
                                         oldContentTopAndBottomIncludingInkOverflow, repaintLogicalTop,
                                         repaintLogicalBottom);

    // FIXME: Tell the accessibility cache the inline content was laid out, once there is one.
  }

  void RenderBlockFlow::SetStaticPositionsForSimpleOutOfFlowContent() noexcept
  {
    krys_debug_assert(ChildrenInline());
#ifndef NDEBUG
    krys_debug_assert(!HasLineIfEmpty());
    for (auto walker = InlineWalker(*this); !walker.AtEnd(); walker.Advance())
    {
      if (walker.Current()->Style().IsDisplayInlineType())
      {
        krys_debug_assert(HasSimpleStaticPositionForInlineLevelOutOfFlowChildrenByStyle(Style()));
        break;
      }
    }
#endif
    // We have nothing but out-of-flow boxes so we don't need to run the actual line layout.
    // Instead, we can just set the static positions to the point where all these boxes would end up.
    // This is a common case when using transforms to animate positioned boxes.
    auto staticPosition = LayoutPoint {BorderAndPaddingStart(), BorderAndPaddingBefore()};

    for (auto walker = InlineWalker(*this); !walker.AtEnd(); walker.Advance())
    {
      auto &renderer = Downcast<RenderBox>(*walker.Current());
      auto &layer = *renderer.Layer();

      krys_debug_assert(renderer.IsOutOfFlowPositioned());

      auto previousStaticPosition = LayoutPoint {layer.StaticInlinePosition(), layer.StaticBlockPosition()};
      auto delta = staticPosition - previousStaticPosition;
      auto hasStaticInlinePositioning = renderer.Style().HasStaticInlinePosition(IsHorizontalWritingMode());

      layer.SetStaticInlinePosition(staticPosition.X());
      layer.SetStaticBlockPosition(staticPosition.Y());

      if (!delta.IsZero() && hasStaticInlinePositioning)
      {
        renderer.SetChildNeedsLayout(MarkingBehavior::MarkOnlyThis);
      }
    }
  }

  namespace
  {
    /// @brief Iterates the renderers that contribute to the inline min/max widths.
    /// @note (1) Positioned content is skipped (since it does not contribute to min/max width of a block)
    /// (2) We do not drill into the children of floats or replaced elements, since you can't break
    /// in the middle of such an element.
    /// (3) Inline flows (e.g., <a>, <span>, <i>) are walked twice, since each side can have
    /// distinct borders/margin/padding that contribute to the min/max width.
    struct InlineMinMaxIterator
    {
    private:
      const RenderBlockFlow &_blockContainer;
      RenderObject *_current {nullptr};
      bool _isEndOfInline {false};
      bool _initial {true};

    public:
      explicit InlineMinMaxIterator(const RenderBlockFlow &blockContainer) noexcept
          : _blockContainer(blockContainer)
      {
      }

      KRYS_NODISCARD bool IsEndOfInline() const noexcept
      {
        return _isEndOfInline;
      }

      RenderObject *Next() noexcept
      {
        RenderObject *candidate = _initial ? _blockContainer.FirstChild() : nullptr;
        _initial = false;

        bool oldEndOfInline = _isEndOfInline;
        _isEndOfInline = false;
        do
        {
          if (!oldEndOfInline && Is<RenderInline>(_current))
          {
            candidate = _current->FirstChildSlow();
          }

          if (!candidate)
          {
            // We hit the end of our inline. (It was empty, e.g., <span></span>.)
            if (!oldEndOfInline && _current && _current->IsRenderInline())
            {
              candidate = _current;
              _isEndOfInline = true;
              break;
            }

            while (_current && _current != &_blockContainer)
            {
              candidate = _current->NextSibling();
              if (candidate)
              {
                break;
              }
              _current = _current->Parent();
              if (_current && _current != &_blockContainer && _current->IsRenderInline())
              {
                candidate = _current;
                _isEndOfInline = true;
                break;
              }
            }
          }

          if (!candidate)
          {
            break;
          }

          if (candidate->IsOutOfFlowPositioned())
          {
            _current = candidate;
            candidate = nullptr;
            continue;
          }

          if (Is<RenderInline>(*candidate) || candidate->IsRenderTextOrLineBreak() || candidate->IsFloating()
              || candidate->IsBlockLevelReplacedOrAtomicInline())
          {
            break;
          }

          if (candidate->Style().IsDisplayBlockLevel())
          {
            break;
          }

          krys_debug_assert(false);
          _current = candidate;
          candidate = nullptr;
        } while (_current || _current == &_blockContainer);
        // Update our position.
        _current = candidate;
        return candidate;
      }
    };

    template <typename TSize>
    KRYS_NODISCARD LayoutUnit BorderMarginOrPaddingWidth(LayoutUnit childValue, const TSize &marginOrPadding,
                                                         const style::ZoomFactor &zoomFactor) noexcept
    {
      if (auto fixed = marginOrPadding.TryFixed())
      {
        return LayoutUnit(fixed->ResolveZoom(zoomFactor));
      }
      if constexpr (std::same_as<TSize, style::MarginEdge>)
      {
        if (marginOrPadding.IsAuto())
        {
          return {};
        }
      }
      return childValue;
    }

    KRYS_NODISCARD LayoutUnit GetBorderPaddingMargin(const RenderBoxModelObject &child,
                                                     bool endOfInline) noexcept
    {
      auto &childStyle = child.Style();
      const auto &childZoomFactor = childStyle.UsedZoomForLength();

      if (endOfInline)
      {
        return BorderMarginOrPaddingWidth(child.MarginEnd(), childStyle.MarginEnd(), childZoomFactor)
               + BorderMarginOrPaddingWidth(child.PaddingEnd(), childStyle.PaddingEnd(), childZoomFactor)
               + child.BorderEnd();
      }
      return BorderMarginOrPaddingWidth(child.MarginStart(), childStyle.MarginStart(), childZoomFactor)
             + BorderMarginOrPaddingWidth(child.PaddingStart(), childStyle.PaddingStart(), childZoomFactor)
             + child.BorderStart();
    }

    inline void StripTrailingSpace(float &inlineMax, float &inlineMin,
                                   RenderObject *trailingSpaceChild) noexcept
    {
      if (auto *renderText = DynamicDowncast<RenderText>(trailingSpaceChild))
      {
        // Collapse away the trailing space at the end of a block.
        const auto &font = renderText->Style().GetFontCascade(); // FIXME: This ignores first-line.
        float spaceWidth = font.Width(RenderBlock::ConstructTextRun(u8" ", renderText->Style()));
        inlineMax -= spaceWidth + font.WordSpacing();
        if (inlineMin > inlineMax)
        {
          inlineMin = inlineMax;
        }
      }
    }

    KRYS_NODISCARD inline Maybe<Pair<const RenderText *, const RenderText *>>
      TrailingRubyBaseAndAdjacentTextContent(const RenderInline &rubyBase,
                                             const RenderBlockFlow &blockContainer) noexcept
    {
      // This functions returns adjacent _content_ renderers by skipping non-inline content (floats,
      // out-of-flow content) inline boxes and related annotation boxes. e.g. <ruby>
      //       <span>base</span><rt>annotation</rt>
      //       <span>adjacent base</span><rt>annotation</rt>
      //      </ruby>
      // returns "base" and "adjacent base" RenderText renderers.
      if (!rubyBase.FirstInFlowChild())
      {
        return null;
      }

      auto shouldSkip = [&](const RenderObject &renderer)
      {
        if (Is<RenderText>(renderer))
        {
          return false;
        }
        if (Is<RenderInline>(renderer))
        {
          return true;
        }
        auto &renderBox = Downcast<RenderBoxModelObject>(renderer);
        return !renderBox.IsInFlow() || renderBox.Style().Display() == DisplayType::RubyAnnotation;
      };

      auto walker = InlineWalker(blockContainer, const_cast<RenderObject *>(rubyBase.FirstInFlowChild()));
      auto lastInlineChildOfRubyBase = [&]() -> RenderObject *
      {
        RenderObject *lastChild = nullptr;
        for (; !walker.AtEnd(); walker.Advance())
        {
          auto *renderer = walker.Current();
          if (renderer->Parent() == rubyBase.Parent())
          {
            return lastChild;
          }
          if (!shouldSkip(*renderer))
          {
            lastChild = renderer;
          }
        }
        return nullptr;
      };
      auto *lastChild = lastInlineChildOfRubyBase();
      if (!lastChild || !Is<RenderText>(*lastChild))
      {
        return null;
      }

      auto firstInlineAfterRubyBase = [&]() -> RenderObject *
      {
        for (; !walker.AtEnd(); walker.Advance())
        {
          if (!shouldSkip(*walker.Current()))
          {
            return walker.Current();
          }
        }
        return nullptr;
      };
      auto *firstSibling = firstInlineAfterRubyBase();
      if (!firstSibling || !Is<RenderText>(*firstSibling))
      {
        return null;
      }

      return Pair<const RenderText *, const RenderText *> {&Downcast<RenderText>(*lastChild),
                                                           &Downcast<RenderText>(*firstSibling)};
    }

    KRYS_NODISCARD inline bool HasTrailingSoftWrapOpportunity(const RenderInline &rubyBase,
                                                              const RenderBlockFlow &blockContainer) noexcept
    {
      if (!rubyBase.Parent()->Style().AutoWrap())
      {
        return false;
      }

      if (auto lastAndNextTextContent = TrailingRubyBaseAndAdjacentTextContent(rubyBase, blockContainer))
      {
        return layout::TextUtil::MayBreakInBetween(
          lastAndNextTextContent->first->Text(), lastAndNextTextContent->first->Style(),
          lastAndNextTextContent->second->Text(), lastAndNextTextContent->second->Style());
      }
      return false;
    }

    KRYS_NODISCARD inline LayoutUnit PreferredWidth(LayoutUnit preferredWidth, float result) noexcept
    {
      return std::max(preferredWidth, LayoutUnit::FromFloatCeil(result));
    }

    KRYS_NODISCARD inline Maybe<LayoutUnit>
      TextIndentForBlockContainer(const RenderBlockFlow &renderer) noexcept
    {
      auto &style = renderer.Style();
      if (auto fixedTextIndent = style.GetTextIndent().Length.TryFixed())
      {
        return !fixedTextIndent->IsZero()
                 ? Maybe<LayoutUnit> {LayoutUnit {fixedTextIndent->ResolveZoom(style.UsedZoomForLength())}}
                 : null;
      }

      auto indentValue = LayoutUnit {};
      if (auto *containingBlock = renderer.ContainingBlock())
      {
        if (auto containingBlockFixedLogicalWidth = containingBlock->Style().LogicalWidth().TryFixed())
        {
          auto containingBlockFixedLogicalWidthValue = style::Evaluate<LayoutUnit>(
            *containingBlockFixedLogicalWidth, containingBlock->Style().UsedZoomForLength());
          // At this point of the shrink-to-fit computation, we don't have a used value for the containing
          // block width (that's exactly to what we try to contribute here) unless the computed value is
          // fixed.
          indentValue =
            style::Evaluate<LayoutUnit>(style.GetTextIndent().Length, containingBlockFixedLogicalWidthValue,
                                        containingBlock->Style().UsedZoomForLength());
        }
      }
      return indentValue ? Maybe<LayoutUnit> {indentValue} : null;
    }
  }

  void RenderBlockFlow::ComputeInlinePreferredLogicalWidths(LayoutUnit &minLogicalWidth,
                                                            LayoutUnit &maxLogicalWidth) const noexcept
  {
    krys_debug_assert(!ShouldApplyInlineSizeContainment());

    if (const_cast<RenderBlockFlow &>(*this).TryComputePreferredWidthsUsingInlinePath(minLogicalWidth,
                                                                                      maxLogicalWidth))
    {
      return;
    }

    float inlineMax = 0.f;
    float inlineMin = 0.f;

    const RenderStyle &styleToUse = Style();
    // If we are at the start of a line, we want to ignore all white-space.
    // Also strip spaces if we previously had text that ended in a trailing space.
    bool stripFrontSpaces = true;
    RenderObject *trailingSpaceChild = nullptr;

    // Firefox and Opera will allow a table cell to grow to fit an image inside it under
    // very specific cirucumstances (in order to match common WinIE renderings).
    // Not supporting the quirk has caused us to mis-render some real sites.
    bool allowImagesToBreak = !GetDocument().InQuirksMode() || !IsRenderTableCell()
                              || !styleToUse.LogicalWidth().IsIntrinsicOrLegacyIntrinsicOrAuto();

    bool oldAutoWrap = styleToUse.AutoWrap();

    InlineMinMaxIterator childIterator(*this);

    // Signals the text indent was more negative than the min preferred width
    auto remainingNegativeTextIndent = Maybe<float> {};
    auto textIndentForMinimum = TextIndentForBlockContainer(*this);
    auto textIndentForMaximum = textIndentForMinimum;
    const RenderBox *previousFloat = nullptr;
    bool isPrevChildInlineFlow = false;
    bool shouldBreakLineAfterText = false;
    bool canHangPunctuationAtStart =
      styleToUse.GetHangingPunctuation().Contains(style::HangingPunctuationValue::First);
    bool canHangPunctuationAtEnd =
      styleToUse.GetHangingPunctuation().Contains(style::HangingPunctuationValue::Last);
    RenderText *lastText = nullptr;
    struct RubyBaseContent
    {
      float MinimumWidth {0.f};
      float MaxiumumWidth {0.f};
      bool HasBreakingPositionAfter {false};
    };
    List<RubyBaseContent> rubyBaseContentStack;

    bool addedStartPunctuationHang = false;

    while (RenderObject *child = childIterator.Next())
    {
      // Interlinear annotations don't participate in inline layout, but they put a minimum width requirement
      // on the associated ruby base.
      auto isInterlinearTypeAnnotation = [&]
      {
        if (auto *renderBlock = DynamicDowncast<RenderBlock>(*child))
        {
          auto &style = renderBlock->Style();
          return style.Display() == DisplayType::RubyAnnotation
                 && (!style.IsInterCharacterRubyPosition()
                     || styleToUse.GetWritingMode().IsVerticalTypographic());
        }
        return false;
      };
      if (isInterlinearTypeAnnotation())
      {
        auto annotationMinimumIntrinsicWidth = LayoutUnit {};
        auto annotationMaximumIntrinsicWidth = LayoutUnit {};
        ComputeChildPreferredLogicalWidths(Downcast<RenderBlock>(*child), annotationMinimumIntrinsicWidth,
                                           annotationMaximumIntrinsicWidth);

        if (!rubyBaseContentStack.empty())
        {
          // Annotation box is always preceded by the associated ruby base.
          // inlineMin/max only gets expanded if the annotation is wider than the base content is.
          auto baseContent = rubyBaseContentStack.back();
          rubyBaseContentStack.pop_back();
          inlineMax +=
            std::max(0.f, annotationMaximumIntrinsicWidth.CeilToFloat() - baseContent.MaxiumumWidth);
          if (baseContent.HasBreakingPositionAfter)
          {
            // When base end has breaking position, the inlineMin value is already reset as we are not
            // tracking the inline content for this "line" anymore. However the annotation still belows to the
            // current "line" so we have to update the minLogicalWidth in case annotation is wider than the
            // base content.
            minLogicalWidth +=
              std::max(0.f, annotationMinimumIntrinsicWidth.CeilToFloat() - baseContent.MinimumWidth);
          }
          else
          {
            inlineMin +=
              std::max(0.f, annotationMinimumIntrinsicWidth.CeilToFloat() - baseContent.MinimumWidth);
          }
        }
        else
        {
          krys_debug_assert(false);
        }
        continue;
      }

      auto resetLineForForcedLineBreak = [&]
      {
        if (styleToUse.CollapseWhiteSpace())
        {
          StripTrailingSpace(inlineMax, inlineMin, trailingSpaceChild);
        }
        minLogicalWidth = PreferredWidth(minLogicalWidth, inlineMin);
        maxLogicalWidth = PreferredWidth(maxLogicalWidth, inlineMax);
        inlineMin = 0;
        inlineMax = 0;
        stripFrontSpaces = true;
        trailingSpaceChild = nullptr;
        textIndentForMinimum = null;
        textIndentForMaximum = null;
        remainingNegativeTextIndent = null;
        addedStartPunctuationHang = true;
        isPrevChildInlineFlow = false;
        oldAutoWrap = child->Parent()->Style().AutoWrap();
      };

      if (child->IsBR())
      {
        resetLineForForcedLineBreak();
        continue;
      }

      if (child->Style().IsDisplayBlockLevel() && !child->IsFloating() && Is<RenderBox>(*child))
      {
        resetLineForForcedLineBreak();

        auto blockMinWidth = LayoutUnit {};
        auto blockMaxWidth = LayoutUnit {};
        ComputeChildPreferredLogicalWidths(Downcast<RenderBox>(*child), blockMinWidth, blockMaxWidth);

        auto marginsInInlineDirection = LayoutUnit {};
        if (auto fixedMarginStart = child->Style().MarginStart(GetWritingMode()).TryFixed())
        {
          marginsInInlineDirection +=
            LayoutUnit::FromFloatCeil(fixedMarginStart->ResolveZoom(child->Style().UsedZoomForLength()));
        }
        if (auto fixedMarginEnd = child->Style().MarginEnd(GetWritingMode()).TryFixed())
        {
          marginsInInlineDirection +=
            LayoutUnit::FromFloatCeil(fixedMarginEnd->ResolveZoom(child->Style().UsedZoomForLength()));
        }

        minLogicalWidth = std::max(minLogicalWidth, blockMinWidth + marginsInInlineDirection);
        maxLogicalWidth = std::max(maxLogicalWidth, blockMaxWidth + marginsInInlineDirection);
        continue;
      }

      // Step One: determine whether or not we need to terminate our current line.
      // Each discrete chunk can become the new min-width, if it is the widest chunk
      // seen so far, and it can also become the max-width.

      // Children fall into three categories:
      // (1) An inline flow object. These objects always have a min/max of 0,
      // and are included in the iteration solely so that their margins can
      // be added in.
      //
      // (2) An inline non-text non-flow object, e.g., an inline replaced element.
      // These objects can always be on a line by themselves, so in this situation
      // we need to break the current line, and then add in our own margins and min/max
      // width on its own line, and then terminate the line.
      //
      // (3) A text object. Text runs can have breakable characters at the start,
      // the middle or the end. They may also lose whitespace off the front if
      // we're already ignoring whitespace. In order to compute accurate min-width
      // information, we need three pieces of information.
      // (a) the min-width of the first non-breakable run. Should be 0 if the text string
      // starts with whitespace.
      // (b) the min-width of the last non-breakable run. Should be 0 if the text string
      // ends with whitespace.
      // (c) the min/max width of the string (trimmed for whitespace).
      //
      // If the text string starts with whitespace, then we need to terminate our current line
      // (unless we're already in a whitespace stripping mode.
      //
      // If the text string has a breakable character in the middle, but didn't start
      // with whitespace, then we add the width of the first non-breakable run and
      // then end the current line. We then need to use the intermediate min/max width
      // values (if any of them are larger than our current min/max). We then look at
      // the width of the last non-breakable run and use that to start a new line
      // (unless we end in whitespace).
      auto autoWrap = child->IsBlockLevelReplacedOrAtomicInline() || Is<RenderText>(*child)
                        ? child->Parent()->Style().AutoWrap()
                        : child->Style().AutoWrap();
      auto childMin = 0.f;
      auto childMax = 0.f;

      if (!child->IsRenderText())
      {
        if (child->IsLineBreakOpportunity())
        {
          minLogicalWidth = PreferredWidth(minLogicalWidth, inlineMin);
          inlineMin = 0;
          continue;
        }
        auto &childStyle = Downcast<RenderElement>(*child).Style();
        // Case (1) and (2). Inline replaced and inline flow elements.
        if (auto *renderInline = DynamicDowncast<RenderInline>(*child))
        {
          // Add in padding/border/margin from the appropriate side of
          // the element.
          float bpm = GetBorderPaddingMargin(*renderInline, childIterator.IsEndOfInline()).ToFloat();
          childMin += bpm;
          childMax += bpm;

          if (childStyle.Display() == DisplayType::RubyBase && !childIterator.IsEndOfInline())
          {
            rubyBaseContentStack.push_back({inlineMin, inlineMax, false});
          }

          inlineMin += childMin;
          inlineMax += childMax;

          if (childStyle.Display() == DisplayType::RubyBase && childIterator.IsEndOfInline())
          {
            if (!rubyBaseContentStack.empty())
            {
              auto rubyBaseStart = rubyBaseContentStack.back();
              auto baseHasBreakingPositionAfter = HasTrailingSoftWrapOpportunity(*renderInline, *this);
              rubyBaseContentStack.back() =
                RubyBaseContent {inlineMin - rubyBaseStart.MinimumWidth,
                                 inlineMax - rubyBaseStart.MaxiumumWidth, baseHasBreakingPositionAfter};
              if (baseHasBreakingPositionAfter)
              {
                // Let's mark based end as a breaking opportunity. Note that annotation may chage the final
                // value of minLogicalWidth.
                minLogicalWidth = PreferredWidth(minLogicalWidth, inlineMin);
                inlineMin = 0;
              }
            }
            else
            {
              krys_debug_assert(false);
            }
          }

          child->ClearNeedsPreferredWidthsUpdate();
        }
        else
        {
          const auto &childZoomFactor = childStyle.UsedZoomForLength();

          // Inline replaced boxes add in their margins to their min/max values.
          if (!child->IsFloating())
          {
            lastText = nullptr;
          }
          LayoutUnit margins;
          if (auto fixedMarginStart = childStyle.MarginStart(GetWritingMode()).TryFixed())
          {
            margins += LayoutUnit::FromFloatCeil(fixedMarginStart->ResolveZoom(childZoomFactor));
          }
          if (auto fixedMarginEnd = childStyle.MarginEnd(GetWritingMode()).TryFixed())
          {
            margins += LayoutUnit::FromFloatCeil(fixedMarginEnd->ResolveZoom(childZoomFactor));
          }
          childMin += margins.CeilToFloat();
          childMax += margins.CeilToFloat();
        }
      }

      if (!Is<RenderInline>(*child) && !Is<RenderText>(*child))
      {
        // Case (2). Inline replaced boxes and floats.
        // Terminate the current line as far as minwidth is concerned.
        LayoutUnit childMinPreferredLogicalWidth;
        LayoutUnit childMaxPreferredLogicalWidth;
        auto *box = DynamicDowncast<RenderBox>(*child);
        if (box->IsHorizontalWritingMode() != IsHorizontalWritingMode())
        {
          auto extent = box->ComputeLogicalHeight(box->BorderAndPaddingLogicalHeight(), 0_lu).Extent;
          childMinPreferredLogicalWidth = extent;
          childMaxPreferredLogicalWidth = extent;
        }
        else
        {
          ComputeChildPreferredLogicalWidths(*box, childMinPreferredLogicalWidth,
                                             childMaxPreferredLogicalWidth);
        }
        childMin += childMinPreferredLogicalWidth.CeilToFloat();
        childMax += childMaxPreferredLogicalWidth.CeilToFloat();

        bool clearPreviousFloat = false;
        if (box->IsFloating())
        {
          auto childClearValue = RenderStyle::GetUsedClear(*box);
          if (previousFloat)
          {
            auto previousFloatValue = RenderStyle::GetUsedFloat(*previousFloat);
            clearPreviousFloat =
              (previousFloatValue == UsedFloat::Left
               && (childClearValue == UsedClear::Left || childClearValue == UsedClear::Both))
              || (previousFloatValue == UsedFloat::Right
                  && (childClearValue == UsedClear::Right || childClearValue == UsedClear::Both));
          }
          previousFloat = box;
        }

        bool canBreakReplacedElement = !box->IsRenderImage() || allowImagesToBreak;
        if (((canBreakReplacedElement && (autoWrap || oldAutoWrap)
              && (!isPrevChildInlineFlow || shouldBreakLineAfterText))
             || clearPreviousFloat))
        {
          minLogicalWidth = PreferredWidth(minLogicalWidth, inlineMin);
          inlineMin = 0;
        }

        // If we're supposed to clear the previous float, then terminate maxwidth as well.
        if (clearPreviousFloat)
        {
          maxLogicalWidth = PreferredWidth(maxLogicalWidth, inlineMax);
          inlineMax = 0;
        }

        // Add in text-indent. This is added in only once.
        if (!box->IsFloating())
        {
          if (textIndentForMinimum)
          {
            childMin += LayoutUnit {textIndentForMinimum->CeilToFloat()}.ToFloat();
            textIndentForMinimum =
              childMin < 0 ? Maybe<LayoutUnit> {LayoutUnit::FromFloatCeil(childMin)} : null;
          }

          if (textIndentForMaximum)
          {
            childMax += LayoutUnit {textIndentForMaximum->CeilToFloat()}.ToFloat();
            textIndentForMaximum =
              childMax < 0 ? Maybe<LayoutUnit> {LayoutUnit::FromFloatCeil(childMax)} : null;
          }
        }

        if (canHangPunctuationAtStart && !addedStartPunctuationHang && !box->IsFloating())
        {
          addedStartPunctuationHang = true;
        }

        // Add our width to the max.
        inlineMax += std::max<float>(0, childMax);

        if ((!autoWrap || !canBreakReplacedElement || (isPrevChildInlineFlow && !shouldBreakLineAfterText)))
        {
          if (box->IsFloating())
          {
            minLogicalWidth = PreferredWidth(minLogicalWidth, childMin);
          }
          else
          {
            inlineMin += childMin;
          }
        }
        else
        {
          // Now check our line.
          minLogicalWidth = PreferredWidth(minLogicalWidth, childMin);

          // Now start a new line.
          inlineMin = 0;
        }

        if (autoWrap && canBreakReplacedElement && isPrevChildInlineFlow)
        {
          minLogicalWidth = PreferredWidth(minLogicalWidth, inlineMin);
          inlineMin = 0;
        }

        // We are no longer stripping whitespace at the start of a line.
        if (!box->IsFloating())
        {
          stripFrontSpaces = false;
          trailingSpaceChild = nullptr;
          lastText = nullptr;
        }
      }
      else if (auto *renderText = DynamicDowncast<RenderText>(*child))
      {
        // FIXME: Combine the text of text-combine-upright text (RenderCombineText::combineTextIfNeeded), once
        // there's text-combine-upright.

        // Determine if we have a breakable character. Pass in
        // whether or not we should ignore any spaces at the front
        // of the string. If those are going to be stripped out,
        // then they shouldn't be considered in the breakable char
        // check.
        bool strippingBeginWS = stripFrontSpaces;
        auto widths = renderText->TrimmedPreferredWidths(inlineMax, stripFrontSpaces);

        childMin = widths.Min;
        childMax = widths.Max;

        // This text object will not be rendered, but it may still provide a breaking opportunity.
        if (!widths.HasBreak && !childMax)
        {
          if (autoWrap && (widths.BeginWS || widths.EndWS || widths.EndZeroSpace))
          {
            minLogicalWidth = PreferredWidth(minLogicalWidth, inlineMin);
            inlineMin = 0;
          }
          continue;
        }

        lastText = renderText;

        if (stripFrontSpaces)
        {
          trailingSpaceChild = child;
        }
        else
        {
          trailingSpaceChild = nullptr;
        }

        // Add in text-indent. This is added in only once.
        float ti = 0.f;
        if (textIndentForMinimum || remainingNegativeTextIndent)
        {
          ti = textIndentForMinimum ? textIndentForMinimum->CeilToFloat()
                                    : std::ceil(*remainingNegativeTextIndent);
          childMin += ti;
          widths.BeginMin += ti;
          // It the text indent negative and larger than the child minimum, we re-use the remainder
          // in future minimum calculations, but using the negative value again on the maximum
          // will lead to under-counting the max pref width.
          textIndentForMinimum = null;
          remainingNegativeTextIndent = childMin < 0 ? Maybe<float> {childMin} : null;
        }

        if (textIndentForMaximum)
        {
          auto textIndent = textIndentForMaximum->CeilToFloat();
          childMax += textIndent;
          widths.BeginMax += textIndent;
          textIndentForMaximum = null;
        }

        // See if we have a hanging punctuation situation at the start.
        if (canHangPunctuationAtStart && !addedStartPunctuationHang)
        {
          auto startIndex = strippingBeginWS ? renderText->FirstCharacterIndexStrippingSpaces() : 0;
          float hangStartWidth = renderText->HangablePunctuationStartWidth(startIndex);
          childMin -= hangStartWidth;
          widths.BeginMin -= hangStartWidth;
          childMax -= hangStartWidth;
          widths.BeginMax -= hangStartWidth;
          addedStartPunctuationHang = true;
        }

        // If we have no breakable characters at all,
        // then this is the easy case. We add ourselves to the current
        // min and max and continue.
        if (!widths.HasBreakableChar)
        {
          inlineMin += childMin;
        }
        else
        {
          // We have a breakable character. Now we need to know if
          // we start and end with whitespace.
          if (widths.BeginWS)
          {
            // End the current line.
            minLogicalWidth = PreferredWidth(minLogicalWidth, inlineMin);
          }
          else
          {
            inlineMin += widths.BeginMin;
            minLogicalWidth = PreferredWidth(minLogicalWidth, inlineMin);
            childMin -= ti;
          }

          inlineMin = childMin;

          if (widths.EndWS || widths.EndZeroSpace)
          {
            // We end in breakable space, which means we can end our current line.
            minLogicalWidth = PreferredWidth(minLogicalWidth, inlineMin);
            inlineMin = 0;
            shouldBreakLineAfterText = false;
          }
          else
          {
            minLogicalWidth = PreferredWidth(minLogicalWidth, inlineMin);
            inlineMin = widths.EndMin;
            shouldBreakLineAfterText = true;
          }
        }

        if (widths.HasBreak)
        {
          inlineMax += widths.BeginMax;
          maxLogicalWidth = PreferredWidth(maxLogicalWidth, inlineMax);
          maxLogicalWidth = PreferredWidth(maxLogicalWidth, childMax);
          inlineMax = widths.EndMax;
          textIndentForMinimum = null;
          textIndentForMaximum = null;
          remainingNegativeTextIndent = null;
          addedStartPunctuationHang = true;
          if (widths.EndsWithBreak)
          {
            stripFrontSpaces = true;
          }
        }
        else
        {
          inlineMax += std::max<float>(0, childMax);
        }
      }

      // Ignore spaces after a list marker.
      if (child->IsRenderListMarker())
      {
        stripFrontSpaces = true;
      }

      isPrevChildInlineFlow = !child->IsRenderText() && child->IsRenderInline();
      oldAutoWrap = autoWrap;
    }

    if (styleToUse.CollapseWhiteSpace())
    {
      StripTrailingSpace(inlineMax, inlineMin, trailingSpaceChild);
    }

    if (canHangPunctuationAtEnd && lastText && !lastText->Text().empty())
    {
      auto lastCodePointStart = [&]
      {
        auto &text = lastText->Text();
        auto position = text.size() - 1;
        while (position && text::IsTrailingUTF8(text[position]))
        {
          --position;
        }
        return position;
      };
      auto endIndex =
        trailingSpaceChild == lastText ? lastText->LastCharacterIndexStrippingSpaces() : lastCodePointStart();
      float endHangWidth = lastText->HangablePunctuationEndWidth(endIndex);
      inlineMin -= endHangWidth;
      inlineMax -= endHangWidth;
    }

    minLogicalWidth = PreferredWidth(minLogicalWidth, inlineMin);
    maxLogicalWidth = PreferredWidth(maxLogicalWidth, inlineMax);
  }

  bool RenderBlockFlow::TryComputePreferredWidthsUsingInlinePath(LayoutUnit &minLogicalWidth,
                                                                 LayoutUnit &maxLogicalWidth) noexcept
  {
    if (!FirstInFlowChild())
    {
      return false;
    }

    ComputeAndSetLineLayoutPath();

    if (GetLineLayoutPath() != LineLayoutPath::InlinePath)
    {
      return false;
    }

    if (!layout_integration::LineLayout::CanUseForPreferredWidthComputation(*this))
    {
      return false;
    }

    if (!InlineLayout())
    {
      _lineLayout = CreateUnique<layout_integration::LineLayout>(*this);
    }

    std::tie(minLogicalWidth, maxLogicalWidth) = InlineLayout()->ComputeIntrinsicWidthConstraints();
    for (auto walker = InlineWalker(*this); !walker.AtEnd(); walker.Advance())
    {
      auto *renderer = walker.Current();
      renderer->ClearNeedsPreferredWidthsUpdate();
      if (auto *renderText = DynamicDowncast<RenderText>(renderer))
      {
        renderText->ResetMinMaxWidth();
      }
    }
    return true;
  }
#pragma region Painting

  void RenderBlockFlow::PaintFloats(PaintInfo &paintInfo, const LayoutPoint &paintOffset,
                                    bool preservePhase) noexcept
  {
    if (!_floatingObjects)
    {
      return;
    }

    for (const auto &floatingObject : _floatingObjects->Set())
    {
      if (!floatingObject->Renderer())
      {
        continue;
      }
      if (!floatingObject->ShouldPaint())
      {
        continue;
      }

      auto floatBoxLocation = FlipFloatForWritingModeForChild(
        *floatingObject, paintOffset + floatingObject->TranslationOffsetToAncestor());
      if (preservePhase)
      {
        floatingObject->Renderer()->Paint(paintInfo, floatBoxLocation);
        continue;
      }
      auto &renderer = *floatingObject->Renderer();
      auto paintInfoForFloat = PaintInfo {paintInfo};

      paintInfoForFloat.Phase = PaintPhase::BlockBackground;
      renderer.Paint(paintInfoForFloat, floatBoxLocation);

      paintInfoForFloat.Phase = PaintPhase::ChildBlockBackgrounds;
      renderer.Paint(paintInfoForFloat, floatBoxLocation);

      paintInfoForFloat.Phase = PaintPhase::Float;
      renderer.Paint(paintInfoForFloat, floatBoxLocation);

      paintInfoForFloat.Phase = PaintPhase::Foreground;
      renderer.Paint(paintInfoForFloat, floatBoxLocation);

      paintInfoForFloat.Phase = PaintPhase::Outline;
      renderer.Paint(paintInfoForFloat, floatBoxLocation);
    }
  }

  void RenderBlockFlow::PaintInlineChildren(PaintInfo &paintInfo, const LayoutPoint &paintOffset) noexcept
  {
    krys_debug_assert(ChildrenInline());

    if (!InlineLayout())
    {
      return;
    }

    InlineLayout()->Paint(paintInfo, paintOffset);
  }

  void RenderBlockFlow::PaintBlockLevelContentInInline(PaintInfo &paintInfo,
                                                       const LayoutPoint &paintOffset) noexcept
  {
    krys_debug_assert(ChildrenInline());

    PaintInfo paintInfoForChildren = PaintInfoForBlockChildren(paintInfo);

    for (auto walker = InlineWalker(*this); !walker.AtEnd(); walker.Advance())
    {
      auto *child = DynamicDowncast<RenderBox>(walker.Current());
      if (!child)
      {
        continue;
      }
      if (!PaintChild(*child, paintInfo, paintOffset, paintInfoForChildren, PaintAsBlock))
      {
        return;
      }
    }
  }

#pragma endregion

  bool RenderBlockFlow::HitTestFloats(const HitTestRequest &request, HitTestResult &result,
                                      const HitTestLocation &locationInContainer,
                                      const LayoutPoint &accumulatedOffset) noexcept
  {
    if (!_floatingObjects)
    {
      return false;
    }

    const LayoutPoint &adjustedLocation = accumulatedOffset;
    // FIXME: The frame view's scroll position (added for the view) comes with frame view scrolling.

    const auto &set = _floatingObjects->Set();
    for (auto it = set.rbegin(); it != set.rend(); ++it)
    {
      const auto &floatingObject = *it;
      auto *renderer = floatingObject->Renderer();
      if (!renderer)
      {
        continue;
      }
      if (floatingObject->ShouldPaint())
      {
        LayoutPoint childPoint = FlipFloatForWritingModeForChild(
          *floatingObject, adjustedLocation + floatingObject->TranslationOffsetToAncestor());
        if (renderer->HitTest(request, result, locationInContainer, childPoint))
        {
          UpdateHitTestResult(result, locationInContainer.Point() - ToLayoutSize(childPoint));
          return true;
        }
      }
    }

    return false;
  }

  namespace
  {
    KRYS_NODISCARD dom::Position PositionForRun(const RenderBlockFlow &flow,
                                                inline_iterator::LeafBoxIterator box, bool start) noexcept
    {
      if (!box)
      {
        return {};
      }

      if (!box->Renderer().NonPseudoNode())
      {
        return dom::MakeDeprecatedLegacyPosition(
          ShareRefPtr<dom::Node>(flow.NonPseudoElement()),
          static_cast<uint32>(start ? flow.CaretMinOffset() : flow.CaretMaxOffset()));
      }

      if (!box->IsText())
      {
        return dom::MakeDeprecatedLegacyPosition(
          ShareRefPtr(box->Renderer().NonPseudoNode()),
          static_cast<uint32>(start ? box->Renderer().CaretMinOffset() : box->Renderer().CaretMaxOffset()));
      }

      inline_iterator::TextBoxIterator textBox(*box);
      return dom::MakeDeprecatedLegacyPosition(ShareRefPtr(textBox->Renderer().NonPseudoNode()),
                                               start ? textBox->Start() : textBox->End());
    }
  }

  dom::PositionWithAffinity
    RenderBlockFlow::PositionForPointWithInlineChildren(const LayoutPoint &pointInLogicalContents,
                                                        HitTestSource source) noexcept
  {
    krys_debug_assert(ChildrenInline());

    auto firstLineBox = inline_iterator::FirstLineBoxFor(*this);

    if (!firstLineBox)
    {
      return CreatePositionWithAffinity(0, Affinity::Downstream);
    }

    bool linesAreFlipped = GetWritingMode().IsLineInverted();
    bool blocksAreFlipped = GetWritingMode().IsBlockFlipped();

    // look for the closest line box in the root box which is at the passed-in y coordinate
    inline_iterator::LeafBoxIterator closestBox;
    inline_iterator::LineBoxIterator firstLineBoxWithChildren;
    inline_iterator::LineBoxIterator lastLineBoxWithChildren;
    for (auto lineBox = firstLineBox; lineBox; lineBox.TraverseNext())
    {
      if (!lineBox->LineLeftmostLeafBox())
      {
        continue;
      }
      if (!firstLineBoxWithChildren)
      {
        firstLineBoxWithChildren = lineBox;
      }

      if (!linesAreFlipped && lineBox->IsFirstAfterPageBreak()
          && (pointInLogicalContents.Y() < lineBox->LogicalTop()
              || (blocksAreFlipped && pointInLogicalContents.Y() == lineBox->LogicalTop())))
      {
        break;
      }

      lastLineBoxWithChildren = lineBox;

      // check if this root line box is located at this y coordinate
      auto selectionBottom = LineSelection::LogicalBottom(*lineBox);
      if (pointInLogicalContents.Y() < selectionBottom
          || (blocksAreFlipped && pointInLogicalContents.Y() == selectionBottom))
      {
        if (linesAreFlipped)
        {
          auto nextLineBoxWithChildren = lineBox->Next();
          while (nextLineBoxWithChildren && !nextLineBoxWithChildren->LineLeftmostLeafBox())
          {
            nextLineBoxWithChildren.TraverseNext();
          }

          if (nextLineBoxWithChildren && nextLineBoxWithChildren->IsFirstAfterPageBreak()
              && (pointInLogicalContents.Y() > nextLineBoxWithChildren->LogicalTop()
                  || (!blocksAreFlipped
                      && pointInLogicalContents.Y() == nextLineBoxWithChildren->LogicalTop())))
          {
            continue;
          }
        }
        closestBox = inline_iterator::ClosestBoxForHorizontalPosition(*lineBox, pointInLogicalContents.X());
        if (closestBox)
        {
          break;
        }
      }
    }

    bool moveCaretToBoundary = EditingBehavior(GetDocument().GetSettings().EditingBehaviorType)
                                 .ShouldMoveCaretToHorizontalBoundaryWhenPastTopOrBottom();

    if (!moveCaretToBoundary && !closestBox && lastLineBoxWithChildren)
    {
      // y coordinate is below last root line box, pretend we hit it
      closestBox = inline_iterator::ClosestBoxForHorizontalPosition(*lastLineBoxWithChildren,
                                                                    pointInLogicalContents.X());
    }

    if (closestBox)
    {
      if (moveCaretToBoundary)
      {
        auto firstLineWithChildrenTop = LayoutUnit {
          std::min(inline_iterator::PreviousLineBoxContentBottomOrBorderAndPadding(*firstLineBoxWithChildren),
                   firstLineBoxWithChildren->ContentLogicalTop())};
        if (pointInLogicalContents.Y() < firstLineWithChildrenTop
            || (blocksAreFlipped && pointInLogicalContents.Y() == firstLineWithChildrenTop))
        {
          auto box = firstLineBoxWithChildren->LineLeftmostLeafBox();
          if (box->IsLineBreak())
          {
            if (auto next = box->NextLineRightwardOnLineIgnoringLineBreak())
            {
              box = next;
            }
          }
          // y coordinate is above first root line box, so return the start of the first
          return PositionForRun(*this, box, true);
        }
      }

      // pass the box a top position that is inside it
      auto point =
        LayoutPoint {pointInLogicalContents.X(),
                     LayoutUnit {inline_iterator::ContentStartInBlockDirection(*closestBox->GetLineBox())}};
      if (!IsHorizontalWritingMode())
      {
        point = point.TransposedPoint();
      }
      if (closestBox->Renderer().IsBlockLevelReplacedOrAtomicInline())
      {
        return PositionForPointRespectingEditingBoundaries(
          *this, const_cast<RenderBox &>(Downcast<RenderBox>(closestBox->Renderer())), point, source);
      }
      return const_cast<RenderObject &>(closestBox->Renderer()).PositionForPoint(point, source);
    }

    if (lastLineBoxWithChildren)
    {
      // We hit this case for Mac behavior when the Y coordinate is below the last box.
      if (auto blockBox = lastLineBoxWithChildren->BlockLevelBox())
      {
        auto &childBlockRenderer = const_cast<RenderBox &>(Downcast<RenderBox>(blockBox->Renderer()));
        return PositionForPointRespectingEditingBoundaries(*this, childBlockRenderer, pointInLogicalContents,
                                                           source);
      }
      krys_debug_assert(moveCaretToBoundary);
      inline_iterator::LineLogicalOrderCache orderCache;
      if (auto logicallyLastBox =
            inline_iterator::LastLeafOnLineInLogicalOrderWithNode(lastLineBoxWithChildren, orderCache))
      {
        return PositionForRun(*this, logicallyLastBox, false);
      }
    }

    // Can't reach this. We have a root line box, but it has no kids.
    // FIXME: This should ASSERT_NOT_REACHED(), but clicking on placeholder text
    // seems to hit this code path.
    return CreatePositionWithAffinity(0, Affinity::Downstream);
  }

  bool RenderBlockFlow::HitTestInlineChildren(const HitTestRequest &request, HitTestResult &result,
                                              const HitTestLocation &locationInContainer,
                                              const LayoutPoint &accumulatedOffset,
                                              HitTestAction hitTestAction) noexcept
  {
    krys_debug_assert(ChildrenInline());

    return InlineLayout()
           && InlineLayout()->HitTest(request, result, locationInContainer, accumulatedOffset, hitTestAction);
  }
}
