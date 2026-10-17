#include "Krystal.Booey/Rendering/RenderBlockFlow.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Page/LocalFrameViewLayoutContext.hpp"
#include "Krystal.Booey/Rendering/BlockStepSizing.hpp"
#include "Krystal.Booey/Rendering/RenderChildIterator.hpp"
#include "Krystal.Booey/Rendering/RenderIterator.hpp"
#include "Krystal.Booey/Rendering/RenderLayer.hpp"
#include "Krystal.Booey/Rendering/RenderLayoutState.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"

namespace krys::boo
{
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

  void RenderBlockFlow::RebuildFloatingObjectSetFromIntrudingFloats() noexcept
  {
    // FIXME: Rebuild the floating objects from the floats that intrude from the previous siblings and the
    // parent, once there's float layout.
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
      // FIXME: The preferred widths of inline content (computeInlinePreferredLogicalWidths), once there's
      // inline layout.
    }
    else
    {
      ComputeBlockPreferredLogicalWidths(minLogicalWidth, maxLogicalWidth);
    }

    maxLogicalWidth = std::max(minLogicalWidth, maxLogicalWidth);

    // FIXME: Multi-column widths (adjustIntrinsicLogicalWidthsForColumns), horizontal marquees, and a table
    // cell's fixed width, once there are columns, marquees and table layout.

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

    // FIXME: Empty block containers produce empty formatting lines which may affect trim-start/end
    // (TextBoxTrimmer), once there's inline layout. Repaint (LayoutRepainter), once there's painting.

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

    // FIXME: Repaint (LayoutRepainter), once there's painting.

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

    // FIXME: Repaint with our new bounds if they are different from our old bounds, once there's painting.
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
    // FIXME: Shift the lines of inline content, once there's inline layout.
    for (auto *child = FirstChildBox(); child; child = child->NextSiblingBox())
    {
      SetLogicalTopForChild(*child, LogicalTopForChild(*child) + space);
      if (child->IsOutOfFlowPositioned() && child->Style().HasStaticBlockPosition(IsHorizontalWritingMode()))
      {
        krys_debug_assert(child->Layer());
        child->Layer()->SetStaticBlockPosition(child->Layer()->StaticBlockPosition() + space);
        child->SetChildNeedsLayout(MarkingBehavior::MarkOnlyThis);
      }
    }
    // FIXME: Shift the floats, once there's float layout.

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
      // FIXME: Empty block containers produce empty formatting lines which may affect trim-start/end
      // (TextBoxTrimmer), once there's inline layout.

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

    // FIXME: Trim the text boxes (TextBoxTrimmer) and update the line clamp (LineClampUpdater), and apply
    // text-box-trim's end to the last formatted line, once there's inline layout.
    ChildrenInline()
      ? LayoutInlineChildren(relayoutChildren, previousHeight, repaintLogicalTop, repaintLogicalBottom)
      : LayoutBlockChildren(relayoutChildren, maxFloatLogicalBottom);
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
        // FIXME: Add the box to its containing block's out-of-flow boxes and set its static position
        // (adjustOutOfFlowBlock), once there's out-of-flow layout.
        continue;
      }
      if (child.IsFloating())
      {
        // FIXME: Mark the siblings the float intrudes into, insert the float and position it
        // (insertFloatingBoxAndMarkForLayout, adjustFloatingBlock), once there's float layout. Until then
        // it's laid out in place.
        child.LayoutIfNeeded();
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

    // FIXME: Walk the inline content (InlineWalker), lay out its boxes and update the lines' overflow, once
    // there's inline layout.
    LayoutChildrenWithoutLineLayout();
  }

  void RenderBlockFlow::LayoutInlineChildren(RelayoutChildren, LayoutUnit, LayoutUnit &,
                                             LayoutUnit &) noexcept
  {
    // FIXME: Lay out the inline content (the line layout path, layoutInlineContent), once there's inline
    // layout. Until then the content has no lines: its boxes are laid out where they are, and the block is as
    // tall as an empty one.
    LayoutChildrenWithoutLineLayout();
    SetLogicalHeight(BorderAndPaddingLogicalHeight() + LayoutUnit(ScrollbarLogicalHeight()));
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

      // FIXME: If the child moved, we have to repaint it as well as any floating/positioned
      // descendants, once there's painting.
    }

    // FIXME: Repaint a child laid out for the first time, and apply an after page or column break, once
    // there's painting and pagination.

#if KRYS_DEBUG
    krys_debug_assert(View().FrameView().LayoutContext().LayoutDelta() == oldLayoutDelta);
#endif
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

    // FIXME: Whether the inline content has lines that aren't self-collapsing (isSelfCollapsingContent), once
    // there's inline layout.

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

    // FIXME: After our style changed, if we lose our ability to propagate floats into next sibling blocks,
    // mark the descendants of the top most parent containing that overhanging float for layout, once there's
    // float layout.

    // FIXME: Update the preferred widths of the inline content (InlineWalker), once there's inline layout.

    // FIXME: The styles of the multi-column flow's children, once there's multi-column layout.
  }

  void RenderBlockFlow::MarkAllDescendantsWithFloatsForLayout(RenderBox *, bool) noexcept
  {
    // FIXME: Mark the descendants that contain the floats for layout, once there's float layout.
  }

  LayoutUnit RenderBlockFlow::AddOverhangingFloats(RenderBlockFlow &, bool) noexcept
  {
    // FIXME: Add the child's floats that overhang it, once there's float layout.
    return 0_lu;
  }

  LayoutUnit RenderBlockFlow::ComputedClearDeltaForChild(RenderBox &, LayoutUnit) noexcept
  {
    // FIXME: The clearance the child needs to clear the floats, once there's float layout.
    return 0_lu;
  }

  void RenderBlockFlow::AddOverflowFromInFlowChildren(EnumSet<ComputeOverflowOptions> options) noexcept
  {
    if (ChildrenInline())
    {
      // FIXME: The overflow of the lines (addOverflowFromInlineChildren), and the fragments' visual overflow,
      // once there's inline layout and fragmented flows.
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

    // FIXME: The first line's baseline, once there's inline layout.

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

    // FIXME: The last line's baseline, once there's inline layout.

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
}
