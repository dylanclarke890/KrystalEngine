#include "Krystal.Booey/Rendering/RenderTable.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/HTML/HTMLTableElement.hpp"
#include "Krystal.Booey/Rendering/AutoTableLayout.hpp"
#include "Krystal.Booey/Rendering/BackgroundPainter.hpp"
#include "Krystal.Booey/Rendering/BorderPainter.hpp"
#include "Krystal.Booey/Rendering/BorderShape.hpp"
#include "Krystal.Booey/Rendering/FixedTableLayout.hpp"
#include "Krystal.Booey/Rendering/HitTestResult.hpp"
#include "Krystal.Booey/Rendering/LayoutRepainter.hpp"
#include "Krystal.Booey/Rendering/PaintInfo.hpp"
#include "Krystal.Booey/Rendering/RenderChildIterator.hpp"
#include "Krystal.Booey/Rendering/RenderLayer.hpp"
#include "Krystal.Booey/Rendering/RenderLayoutState.hpp"
#include "Krystal.Booey/Rendering/RenderTableCaption.hpp"
#include "Krystal.Booey/Rendering/RenderTableCell.hpp"
#include "Krystal.Booey/Rendering/RenderTableCol.hpp"
#include "Krystal.Booey/Rendering/RenderTableRow.hpp"
#include "Krystal.Booey/Rendering/RenderTableSection.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Style/Difference.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesEvaluation.hpp"
#include <algorithm>
#include <limits>

namespace krys::boo
{
  namespace
  {
    inline void ResetSectionPointerIfNotBefore(WeakPtr<RenderTableSection> &section,
                                               RenderObject *before) noexcept
    {
      if (!before || !section)
      {
        return;
      }

      auto *previousSibling = before->PreviousSibling();
      while (previousSibling && previousSibling != section.get())
      {
        previousSibling = previousSibling->PreviousSibling();
      }
      if (!previousSibling)
      {
        section = nullptr;
      }
    }
  }

  RenderTable::RenderTable(Type type, dom::Element &element, RenderStyle &&style) noexcept
      : RenderBlock(type, element, krys::move(style), {}), _columnPos(1, 0_lu)
  {
    SetChildrenInline(false);
    krys_debug_assert(IsRenderTable());
  }

  RenderTable::RenderTable(Type type, dom::Document &document, RenderStyle &&style) noexcept
      : RenderBlock(type, document, krys::move(style), {}), _columnPos(1, 0_lu)
  {
    SetChildrenInline(false);
    krys_debug_assert(IsRenderTable());
  }

  // Do not add any code in below destructor. Add it to willBeDestroyed() instead.
  RenderTable::~RenderTable() noexcept = default;

  bool RenderTable::IsHTMLTableElement() const noexcept
  {
    auto *htmlElement = DynamicDowncast<html::HTMLElement>(GetElement());
    return htmlElement && Is<html::HTMLTableElement>(*htmlElement);
  }

  bool RenderTable::CollapseBorders() const noexcept
  {
    return Style().GetBorderCollapse() == BorderCollapse::Collapse;
  }

  bool RenderTable::HasSections() const noexcept
  {
    return _head || _foot || _firstBody;
  }

  RenderTableSection *RenderTable::Header() const noexcept
  {
    return _head.get();
  }

  RenderTableSection *RenderTable::Footer() const noexcept
  {
    return _foot.get();
  }

  RenderTableSection *RenderTable::FirstBody() const noexcept
  {
    return _firstBody.get();
  }

  RenderTableSection *RenderTable::TopSection() const noexcept
  {
    krys_debug_assert(!NeedsSectionRecalc());
    if (_head)
    {
      return _head.get();
    }
    if (_firstBody)
    {
      return _firstBody.get();
    }
    return _foot.get();
  }

  RenderTableSection *RenderTable::BottomSection() const noexcept
  {
    RecalcSectionsIfNeeded();
    if (_foot)
    {
      return _foot.get();
    }

    for (auto *child = LastChild(); child; child = child->PreviousSibling())
    {
      if (child == _head.get())
      {
        continue;
      }
      if (auto *tableSection = DynamicDowncast<RenderTableSection>(*child))
      {
        return tableSection;
      }
    }
    return _head.get();
  }

  void RenderTable::StyleDidChange(style::Difference diff, const RenderStyle *oldStyle) noexcept
  {
    RenderBlock::StyleDidChange(diff, oldStyle);
    PropagateStyleToAnonymousChildren(StylePropagationType::AllChildren);

    auto oldFixedTableLayout = oldStyle ? oldStyle->IsFixedTableLayout() : false;

    // In the collapsed border model, there is no cell spacing.
    auto usedZoom = Style().UsedZoomForLength();
    _hSpacing =
      CollapseBorders() ? 0_lu : style::Evaluate<LayoutUnit>(Style().BorderHorizontalSpacing(), usedZoom);
    _vSpacing =
      CollapseBorders() ? 0_lu : style::Evaluate<LayoutUnit>(Style().BorderVerticalSpacing(), usedZoom);
    krys_debug_assert(_hSpacing >= 0);
    krys_debug_assert(_vSpacing >= 0);

    if (!_tableLayout || Style().IsFixedTableLayout() != oldFixedTableLayout)
    {
      // According to the CSS2 spec, you only use fixed table layout if an explicit width is specified on the
      // table. Auto width implies auto table layout.
      if (Style().IsFixedTableLayout())
      {
        _tableLayout = CreateUnique<FixedTableLayout>(this);
      }
      else
      {
        auto resetTableCellPreferredLogicalWidths = [&]
        {
          if (!_tableLayout)
          {
            return;
          }
          // Fixed table layout sets min/max preferred widths to clean without actually computing them (see
          // FixedTableLayout::CalcWidthArray).
          for (auto &section : ChildrenOfType<RenderTableSection>(*this))
          {
            for (auto *row = section.FirstRow(); row; row = row->NextRow())
            {
              for (auto *cell = row->FirstCell(); cell; cell = cell->NextCell())
              {
                cell->SetNeedsPreferredWidthsUpdate();
              }
            }
          }
        };
        resetTableCellPreferredLogicalWidths();
        _tableLayout = CreateUnique<AutoTableLayout>(this);
      }
    }

    if (oldStyle)
    {
      InvalidateCollapsedBordersAfterStyleChangeIfNeeded(*oldStyle, Style());
    }
  }

  void RenderTable::WillInsertTableColumn(RenderTableCol &, RenderObject *) noexcept
  {
    _hasColElements = true;
  }

  void RenderTable::WillInsertTableSection(RenderTableSection &child, RenderObject *beforeChild) noexcept
  {
    switch (child.Style().Display())
    {
      case DisplayType::TableHeaderGroup:
        ResetSectionPointerIfNotBefore(_head, beforeChild);
        if (!_head)
        {
          _head = WeakPtr<RenderTableSection>::WithRef(child);
        }
        else
        {
          ResetSectionPointerIfNotBefore(_firstBody, beforeChild);
          if (!_firstBody)
          {
            _firstBody = WeakPtr<RenderTableSection>::WithRef(child);
          }
        }
        break;
      case DisplayType::TableFooterGroup:
        ResetSectionPointerIfNotBefore(_foot, beforeChild);
        if (!_foot)
        {
          _foot = WeakPtr<RenderTableSection>::WithRef(child);
          break;
        }
        KRYS_FALLTHROUGH;
      case DisplayType::TableRowGroup:
        ResetSectionPointerIfNotBefore(_firstBody, beforeChild);
        if (!_firstBody)
        {
          _firstBody = WeakPtr<RenderTableSection>::WithRef(child);
        }
        break;
      default: krys_debug_assert(false);
    }

    SetNeedsSectionRecalc();
  }

  void RenderTable::AddCaption(RenderTableCaption &caption) noexcept
  {
    krys_debug_assert(
      std::ranges::none_of(_captions, [&](const auto &existing) { return existing == &caption; }));
    _captions.push_back(WeakPtr<RenderTableCaption>::WithRef(caption));
  }

  void RenderTable::RemoveCaption(RenderTableCaption &oldCaption) noexcept
  {
    auto it = std::ranges::find_if(_captions, [&](const auto &caption) { return caption == &oldCaption; });
    krys_debug_assert(it != _captions.end());
    if (it != _captions.end())
    {
      _captions.erase(it);
    }
  }

  void RenderTable::InvalidateCachedColumns() noexcept
  {
    _columnRenderersValid = false;
    _columnRenderers.clear();
    _effectiveColumnIndexMap.clear();
  }

  void RenderTable::InvalidateCachedColumnOffsets() noexcept
  {
    _columnOffsetTop = -1_lu;
    _columnOffsetHeight = -1_lu;
  }

  void RenderTable::AddColumn(const RenderTableCol *) noexcept
  {
    InvalidateCachedColumns();
  }

  void RenderTable::InvalidateColumns() noexcept
  {
    InvalidateCachedColumns();
    // We don't really need to recompute our sections, but we need to update our
    // column count and whether we have a column. Currently, we only have one
    // size-fit-all flag but we may have to consider splitting it.
    SetNeedsSectionRecalc();
  }

  void RenderTable::UpdateLogicalWidth() noexcept
  {
    RecalcSectionsIfNeeded();

    if (IsGridItem())
    {
      // FIXME: Investigate whether the grid layout algorithm provides all the logic
      // needed and that we're not skipping anything essential due to the early return here.
      RenderBlock::UpdateLogicalWidth();
      return;
    }

    if (IsOutOfFlowPositioned())
    {
      LogicalExtentComputedValues computedValues;
      ComputeOutOfFlowPositionedLogicalWidth(computedValues);
      SetLogicalWidth(computedValues.Extent);
      SetLogicalLeft(computedValues.Position);
      SetMarginStart(computedValues.Margins.Start);
      SetMarginEnd(computedValues.Margins.End);
    }

    auto &cb = *ContainingBlock();

    auto availableLogicalWidth = ContainingBlockLogicalWidthForContent();
    bool hasPerpendicularContainingBlock = GetWritingMode().IsOrthogonal(cb.GetWritingMode());
    auto containerWidthInInlineDirection =
      hasPerpendicularContainingBlock ? PerpendicularContainingBlockLogicalHeight() : availableLogicalWidth;

    const auto &styleLogicalWidth = Style().LogicalWidth();
    if (auto overridingLogicalWidth = OverridingBorderBoxLogicalWidth())
    {
      SetLogicalWidth(*overridingLogicalWidth);
    }
    else if ((styleLogicalWidth.IsSpecified() && styleLogicalWidth.IsPossiblyPositive())
             || styleLogicalWidth.IsIntrinsic())
    {
      SetLogicalWidth(
        ConvertStyleLogicalWidthToComputedWidth(styleLogicalWidth, containerWidthInInlineDirection));
    }
    else
    {
      // Subtract out any fixed margins from our available width for auto width tables.
      auto usedZoom = Style().UsedZoomForLength();
      auto marginStart =
        style::EvaluateMinimum<LayoutUnit>(Style().MarginStart(), availableLogicalWidth, usedZoom);
      auto marginEnd =
        style::EvaluateMinimum<LayoutUnit>(Style().MarginEnd(), availableLogicalWidth, usedZoom);
      auto marginTotal = marginStart + marginEnd;

      // Subtract out our margins to get the available content width.
      auto availableContentLogicalWidth = std::max(0_lu, containerWidthInInlineDirection - marginTotal);
      if (ShrinkToAvoidFloats() && cb.ContainsFloats() && !hasPerpendicularContainingBlock)
      {
        // FIXME: Work with regions someday.
        availableContentLogicalWidth = ShrinkLogicalWidthToAvoidFloats(marginStart, marginEnd, cb);
      }

      // Ensure we aren't bigger than our available width.
      auto maxWidth = MaxPreferredLogicalWidth();
      // ScaledWidthFromPercentColumns depends on the column layouts in AutoTableLayout, which
      // MaxPreferredLogicalWidth fills in. So ScaledWidthFromPercentColumns has to be called after
      // MaxPreferredLogicalWidth.
      auto scaledWidth =
        _tableLayout->ScaledWidthFromPercentColumns() + BordersPaddingAndSpacingInRowDirection();
      maxWidth = std::max(scaledWidth, maxWidth);
      SetLogicalWidth(std::min(availableContentLogicalWidth, maxWidth));
    }

    // Ensure we aren't bigger than our max-width style.
    const auto &styleMaxLogicalWidth = Style().LogicalMaxWidth();
    if (styleMaxLogicalWidth.IsSpecified() || styleMaxLogicalWidth.IsIntrinsic())
    {
      auto computedMaxLogicalWidth =
        ConvertStyleLogicalWidthToComputedWidth(styleMaxLogicalWidth, availableLogicalWidth);
      SetLogicalWidth(std::min(LogicalWidth(), computedMaxLogicalWidth));
    }

    // Ensure we aren't smaller than our min preferred width.
    SetLogicalWidth(std::max(LogicalWidth(), MinPreferredLogicalWidth()));

    // Ensure we aren't smaller than our min-width style.
    const auto &styleMinLogicalWidth = Style().LogicalMinWidth();
    if (styleMinLogicalWidth.IsSpecified() || styleMinLogicalWidth.IsIntrinsic())
    {
      auto computedMinLogicalWidth =
        ConvertStyleLogicalWidthToComputedWidth(styleMinLogicalWidth, availableLogicalWidth);
      SetLogicalWidth(std::max(LogicalWidth(), computedMinLogicalWidth));
    }

    // Finally, with our true width determined, compute our margins for real.
    SetMarginStart(0_lu);
    SetMarginEnd(0_lu);
    if (!hasPerpendicularContainingBlock)
    {
      auto containerLogicalWidthForAutoMargins = availableLogicalWidth;
      if (AvoidsFloats() && cb.ContainsFloats())
      {
        containerLogicalWidthForAutoMargins = ContainingBlockAvailableLineWidth();
      }
      ComputedMarginValues marginValues;
      bool hasSameDirection = !cb.GetWritingMode().IsInlineOpposing(GetWritingMode());
      ComputeInlineDirectionMargins(cb, availableLogicalWidth, containerLogicalWidthForAutoMargins,
                                    LogicalWidth(), hasSameDirection ? marginValues.Start : marginValues.End,
                                    hasSameDirection ? marginValues.End : marginValues.Start);
      SetMarginStart(marginValues.Start);
      SetMarginEnd(marginValues.End);
    }
    else
    {
      auto usedZoom = Style().UsedZoomForLength();
      SetMarginStart(
        style::EvaluateMinimum<LayoutUnit>(Style().MarginStart(), availableLogicalWidth, usedZoom));
      SetMarginEnd(style::EvaluateMinimum<LayoutUnit>(Style().MarginEnd(), availableLogicalWidth, usedZoom));
    }
  }

  // This method takes a RenderStyle's logical width, min-width, or max-width length and computes its actual
  // value.
  template <typename TSize>
  LayoutUnit RenderTable::ConvertStyleLogicalWidthToComputedWidth(const TSize &styleLogicalWidth,
                                                                  LayoutUnit availableWidth) noexcept
  {
    if (styleLogicalWidth.IsIntrinsic())
    {
      return ComputeIntrinsicLogicalWidthUsing(styleLogicalWidth, availableWidth,
                                               BordersPaddingAndSpacingInRowDirection());
    }

    // HTML tables' width styles already include borders and padding, but CSS tables' width styles do not.
    LayoutUnit borders;
    bool isCSSTable = !IsHTMLTableElement();
    if (isCSSTable && styleLogicalWidth.IsSpecified() && styleLogicalWidth.IsPossiblyPositive()
        && Style().GetBoxSizing() == BoxSizing::ContentBox)
    {
      borders = BorderStart() + BorderEnd() + (CollapseBorders() ? 0_lu : PaddingStart() + PaddingEnd());
    }

    return style::EvaluateMinimum<LayoutUnit>(styleLogicalWidth, availableWidth, Style().UsedZoomForLength())
           + borders;
  }

  template <typename TSize>
  LayoutUnit RenderTable::ConvertStyleLogicalHeightToComputedHeight(const TSize &styleLogicalHeight) noexcept
  {
    auto borderAndPaddingBefore = BorderBefore() + (CollapseBorders() ? 0_lu : PaddingBefore());
    auto borderAndPaddingAfter = BorderAfter() + (CollapseBorders() ? 0_lu : PaddingAfter());
    auto borderAndPadding = borderAndPaddingBefore + borderAndPaddingAfter;
    if (auto fixedStyleLogicalHeight = styleLogicalHeight.TryFixed())
    {
      // HTML tables size as though CSS height includes border/padding, CSS tables do not.
      LayoutUnit borders;
      // FIXME: We cannot apply box-sizing: content-box on <table> which other browsers allow.
      if (IsHTMLTableElement() || Style().GetBoxSizing() == BoxSizing::BorderBox)
      {
        borders = borderAndPadding;
      }
      return style::Evaluate<LayoutUnit>(*fixedStyleLogicalHeight, Style().UsedZoomForLength()) - borders;
    }
    if (styleLogicalHeight.IsPercentOrCalculated())
    {
      return ComputePercentageLogicalHeight(styleLogicalHeight).value_or(0_lu);
    }
    if (styleLogicalHeight.IsIntrinsic())
    {
      return ComputeIntrinsicLogicalContentHeightUsing(styleLogicalHeight, LogicalHeight() - borderAndPadding,
                                                       borderAndPadding)
        .value_or(0_lu);
    }
    krys_debug_assert(false);
    return 0_lu;
  }

  void RenderTable::LayoutCaption(RenderTableCaption &caption) noexcept
  {
    LayoutRect captionRect(caption.FrameRect());

    if (caption.NeedsLayout())
    {
      // The margins may not be available but ensure the caption is at least located beneath any previous
      // sibling caption so that it does not mistakenly think any floats in the previous caption intrude into
      // it.
      caption.SetLogicalLocation(
        LayoutPoint(caption.MarginStart(), caption.MarginBefore() + LogicalHeight()));
      // If RenderTableCaption ever gets a Layout() function, use it here.
      caption.LayoutIfNeeded();
    }
    // Apply the margins to the location now that they are definitely available from layout
    caption.SetLogicalLocation(LayoutPoint(caption.MarginStart(), caption.MarginBefore() + LogicalHeight()));

    if (!SelfNeedsLayout() && caption.CheckForRepaintDuringLayout())
    {
      caption.RepaintDuringLayoutIfMoved(captionRect);
    }

    SetLogicalHeight(LogicalHeight() + caption.LogicalHeight() + caption.MarginBefore()
                     + caption.MarginAfter());
  }

  void RenderTable::LayoutCaptions(BottomCaptionLayoutPhase bottomCaptionLayoutPhase) noexcept
  {
    if (_captions.empty())
    {
      return;
    }
    // FIXME: Collapse caption margin.
    for (auto &caption : _captions)
    {
      auto isBottomCaption = caption->Style().GetCaptionSide() == CaptionSide::Bottom;
      if ((bottomCaptionLayoutPhase == BottomCaptionLayoutPhase::Yes && !isBottomCaption)
          || (bottomCaptionLayoutPhase == BottomCaptionLayoutPhase::No && isBottomCaption))
      {
        continue;
      }
      LayoutCaption(*caption);
    }
  }

  void RenderTable::DistributeExtraLogicalHeight(LayoutUnit extraLogicalHeight) noexcept
  {
    if (extraLogicalHeight <= 0)
    {
      return;
    }

    // FIXME: Distribute the extra logical height between all table sections instead of giving it all to the
    // first one.
    if (auto *section = FirstBody())
    {
      extraLogicalHeight -= section->DistributeExtraLogicalHeightToRows(extraLogicalHeight);
    }

    // FIXME: We really would like to enable this assert to ensure that all the extra space has been
    // distributed. However our current distribution algorithm does not round properly and thus we can have
    // some remaining height.
    // krys_debug_assert(!TopSection() || !extraLogicalHeight);
  }

  void RenderTable::SimplifiedNormalFlowLayout() noexcept
  {
    for (auto &caption : _captions)
    {
      caption->LayoutIfNeeded();
    }
    for (auto *section = TopSection(); section; section = SectionBelow(section))
    {
      section->LayoutIfNeeded();
      section->LayoutRows();
      section->ComputeOverflowFromCells();
      section->AddVisualEffectOverflow();
    }
  }

  LayoutUnit RenderTable::SumCaptionsLogicalHeight() const noexcept
  {
    LayoutUnit height;
    for (const auto &caption : _captions)
    {
      height += caption->LogicalHeight() + caption->MarginBefore() + caption->MarginAfter();
    }
    return height;
  }

  void RenderTable::SetNeedsSectionRecalc() noexcept
  {
    if (RenderTreeBeingDestroyed())
    {
      return;
    }
    _needsSectionRecalc = true;
    SetNeedsLayout();
  }

  void RenderTable::Layout() noexcept
  {
    krys_debug_assert(NeedsLayout());

    if (SimplifiedLayout())
    {
      return;
    }

    RecalcSectionsIfNeeded();
    // FIXME: We should do this recalc lazily in BorderStart/BorderEnd so that we don't have to make sure
    // to call this before we call BorderStart/BorderEnd to avoid getting a stale value.
    RecalcBordersInRowDirection();

    bool sectionMoved = false;
    LayoutUnit movedSectionLogicalTop;
    bool shouldCacheIntrinsicContentLogicalHeightForFlexItem = true;

    LayoutRepainter repainter(*this);
    {
      // There are no reflections (-webkit-box-reflect was dropped).
      LayoutStateMaintainer statePusher(*this, LocationOffset(),
                                        IsTransformed() || GetWritingMode().IsBlockFlipped());

      auto oldLogicalWidth = LogicalWidth();
      auto oldLogicalHeight = LogicalHeight();
      ResetLogicalHeightBeforeLayoutIfNeeded();
      UpdateLogicalWidth();

      if (LogicalWidth() != oldLogicalWidth)
      {
        for (auto &caption : _captions)
        {
          caption->SetNeedsLayout(RenderObjectMarkingBehavior::MarkOnlyThis);
        }
      }
      // FIXME: The optimisation below doesn't work since the internal table
      // layout could have changed. We need to add a flag to the table
      // layout that tells us if something has changed in the min max
      // calculations to do it correctly.
      //     if ( oldWidth != width() || columns.size() + 1 != columnPos.size() )
      _tableLayout->Layout();

      LayoutUnit totalSectionLogicalHeight;

      bool collapsing = CollapseBorders();

      for (auto &child : ChildrenOfType<RenderElement>(*this))
      {
        if (auto *section = DynamicDowncast<RenderTableSection>(child))
        {
          if (_columnLogicalWidthChanged)
          {
            section->SetChildNeedsLayout(RenderObjectMarkingBehavior::MarkOnlyThis);
          }
          section->LayoutIfNeeded();
          totalSectionLogicalHeight += section->CalcRowLogicalHeight();
          if (collapsing)
          {
            section->RecalcOuterBorder();
          }
          krys_debug_assert(!section->NeedsLayout());
        }
        else if (auto *column = DynamicDowncast<RenderTableCol>(child))
        {
          column->LayoutIfNeeded();
          krys_debug_assert(!column->NeedsLayout());
        }
      }

      LayoutCaptions();

      auto borderAndPaddingBefore = BorderBefore() + (collapsing ? 0_lu : PaddingBefore());
      auto borderAndPaddingAfter = BorderAfter() + (collapsing ? 0_lu : PaddingAfter());

      SetLogicalHeight(LogicalHeight() + borderAndPaddingBefore);

      if (!IsOutOfFlowPositioned())
      {
        UpdateLogicalHeight();
      }

      LayoutUnit computedLogicalHeight;

      const auto &logicalHeightLength = Style().LogicalHeight();
      if (logicalHeightLength.IsIntrinsic()
          || (logicalHeightLength.IsSpecified() && logicalHeightLength.IsPossiblyPositive()))
      {
        computedLogicalHeight = ConvertStyleLogicalHeightToComputedHeight(logicalHeightLength);
      }

      if (auto overridingLogicalHeight = OverridingBorderBoxLogicalHeight())
      {
        computedLogicalHeight =
          std::max(computedLogicalHeight,
                   *overridingLogicalHeight - borderAndPaddingAfter - SumCaptionsLogicalHeight());
      }

      if (!ShouldIgnoreLogicalMinMaxHeightSizes())
      {
        const auto &logicalMaxHeightLength = Style().LogicalMaxHeight();
        if (logicalMaxHeightLength.IsFillAvailable() || logicalMaxHeightLength.IsSpecified())
        {
          auto computedMaxLogicalHeight = ConvertStyleLogicalHeightToComputedHeight(logicalMaxHeightLength);
          computedLogicalHeight = std::min(computedLogicalHeight, computedMaxLogicalHeight);
        }

        auto logicalMinHeightLength = Style().LogicalMinHeight();
        if (logicalMinHeightLength.IsMinContent() || logicalMinHeightLength.IsMaxContent()
            || logicalMinHeightLength.IsFitContent())
        {
          logicalMinHeightLength = css::keywords::Auto {};
        }

        if (logicalMinHeightLength.IsIntrinsic() || logicalMinHeightLength.IsSpecified())
        {
          auto computedMinLogicalHeight = ConvertStyleLogicalHeightToComputedHeight(logicalMinHeightLength);
          computedLogicalHeight = std::max(computedLogicalHeight, computedMinLogicalHeight);
        }
      }

      DistributeExtraLogicalHeight(computedLogicalHeight - totalSectionLogicalHeight);

      for (auto *section = TopSection(); section; section = SectionBelow(section))
      {
        section->LayoutRows();
      }

      if (!TopSection() && computedLogicalHeight > totalSectionLogicalHeight && !GetDocument().InQuirksMode())
      {
        // Completely empty tables (with no sections or anything) should at least honor their
        // overriding or specified height in strict mode, but this value will not be cached.
        shouldCacheIntrinsicContentLogicalHeightForFlexItem = false;
        auto tableLogicalHeight = [&]
        {
          if (auto overridingLogicalHeight = OverridingBorderBoxLogicalHeight())
          {
            return *overridingLogicalHeight - borderAndPaddingAfter;
          }
          return LogicalHeight() + computedLogicalHeight;
        };
        SetLogicalHeight(tableLogicalHeight());
      }

      auto sectionLogicalLeft = GetWritingMode().IsLogicalLeftInlineStart() ? BorderStart() : BorderEnd();
      if (!collapsing)
      {
        sectionLogicalLeft += GetWritingMode().IsLogicalLeftInlineStart() ? PaddingStart() : PaddingEnd();
      }

      // position the table sections
      for (auto *section = TopSection(); section; section = SectionBelow(section))
      {
        if (!sectionMoved && section->LogicalTop() != LogicalHeight())
        {
          sectionMoved = true;
          movedSectionLogicalTop = std::min(LogicalHeight(), section->LogicalTop())
                                   + (GetWritingMode().IsHorizontal() ? section->VisualOverflowRect().Y()
                                                                      : section->VisualOverflowRect().X());
        }
        section->SetLogicalLocation(LayoutPoint(sectionLogicalLeft, LogicalHeight()));

        SetLogicalHeight(LogicalHeight() + section->LogicalHeight());

        section->AddVisualEffectOverflow();
      }

      SetLogicalHeight(LogicalHeight() + borderAndPaddingAfter);

      LayoutCaptions(BottomCaptionLayoutPhase::Yes);

      if (IsOutOfFlowPositioned())
      {
        UpdateLogicalHeight();
      }

      // table can be containing block of positioned elements.
      bool dimensionChanged = oldLogicalWidth != LogicalWidth() || oldLogicalHeight != LogicalHeight();
      LayoutOutOfFlowBoxes(dimensionChanged ? RelayoutChildren::Yes : RelayoutChildren::No);

      UpdateLayerTransform();

      // Layout was changed, so probably borders too.
      InvalidateCollapsedBorders();

      // The location or height of one or more sections may have changed.
      InvalidateCachedColumnOffsets();

      ComputeOverflow(ClientLogicalBottom());
    }

    // FIXME: The page offset, once there's pagination.

    bool didFullRepaint = repainter.RepaintAfterLayout();
    // Repaint with our new bounds if they are different from our old bounds.
    if (!didFullRepaint && sectionMoved)
    {
      if (GetWritingMode().IsHorizontal())
      {
        RepaintRectangle(LayoutRect(VisualOverflowRect().X(), movedSectionLogicalTop,
                                    VisualOverflowRect().Width(),
                                    VisualOverflowRect().MaxY() - movedSectionLogicalTop));
      }
      else
      {
        RepaintRectangle(LayoutRect(movedSectionLogicalTop, VisualOverflowRect().Y(),
                                    VisualOverflowRect().MaxX() - movedSectionLogicalTop,
                                    VisualOverflowRect().Height()));
      }
    }

    // FIXME: Laying the table out again when a section moved in paginated layout
    // (markForPaginationRelayoutIfNeeded), once there's pagination.

    // FIXME: This value isn't the intrinsic content logical height, but we need
    // to update the value as its used by flexbox layout. crbug.com/367324
    if (shouldCacheIntrinsicContentLogicalHeightForFlexItem)
    {
      CacheIntrinsicContentLogicalHeightForFlexItem(ContentBoxLogicalHeight());
    }

    _columnLogicalWidthChanged = false;
    ClearNeedsLayout();
  }

  void RenderTable::InvalidateCollapsedBordersAfterStyleChangeIfNeeded(
    const RenderStyle &oldStyle, const RenderStyle &newStyle, RenderTableCell *cellWithStyleChange) noexcept
  {
    auto shouldInvalidate = [&]
    {
      if (oldStyle.GetWritingMode() != newStyle.GetWritingMode())
      {
        return true;
      }
      return !style::BorderIsEquivalentForPainting(oldStyle, newStyle);
    };
    if (shouldInvalidate())
    {
      InvalidateCollapsedBorders(cellWithStyleChange);
    }
  }

  void RenderTable::InvalidateCollapsedBorders(RenderTableCell *cellWithStyleChange) noexcept
  {
    _collapsedBordersValid = false;
    _collapsedBorders.clear();

    for (auto &section : ChildrenOfType<RenderTableSection>(*this))
    {
      section.ClearCachedCollapsedBorders();
    }

    if (!_collapsedEmptyBorderIsPresent)
    {
      return;
    }

    if (cellWithStyleChange)
    {
      // It is enough to invalidate just the surrounding cells when cell border style changes.
      cellWithStyleChange->InvalidateHasEmptyCollapsedBorders();
      if (auto *below = CellBelow(cellWithStyleChange))
      {
        below->InvalidateHasEmptyCollapsedBorders();
      }
      if (auto *above = CellAbove(cellWithStyleChange))
      {
        above->InvalidateHasEmptyCollapsedBorders();
      }
      if (auto *before = CellBefore(cellWithStyleChange))
      {
        before->InvalidateHasEmptyCollapsedBorders();
      }
      if (auto *after = CellAfter(cellWithStyleChange))
      {
        after->InvalidateHasEmptyCollapsedBorders();
      }
      return;
    }

    for (auto &section : ChildrenOfType<RenderTableSection>(*this))
    {
      for (auto *row = section.FirstRow(); row; row = row->NextRow())
      {
        for (auto *cell = row->FirstCell(); cell; cell = cell->NextCell())
        {
          krys_debug_assert(cell->Table() == this);
          cell->InvalidateHasEmptyCollapsedBorders();
        }
      }
    }
    _collapsedEmptyBorderIsPresent = false;
  }

  void RenderTable::AddOverflowFromInFlowChildren(EnumSet<ComputeOverflowOptions>) noexcept
  {
    // Add overflow from borders.
    // Technically it's odd that we are incorporating the borders into layout overflow, which is only supposed
    // to be about overflow from our descendant objects, but since tables don't support overflow:auto, this
    // works out fine.
    if (CollapseBorders())
    {
      auto rightBorderOverflow = Width() + OuterBorderRight() - BorderRight();
      auto leftBorderOverflow = BorderLeft() - OuterBorderLeft();
      auto bottomBorderOverflow = Height() + OuterBorderBottom() - BorderBottom();
      auto topBorderOverflow = BorderTop() - OuterBorderTop();
      auto borderOverflowRect =
        LayoutRect(leftBorderOverflow, topBorderOverflow, rightBorderOverflow - leftBorderOverflow,
                   bottomBorderOverflow - topBorderOverflow);
      if (borderOverflowRect != BorderBoxRect())
      {
        AddLayoutOverflow(borderOverflowRect);
        AddVisualOverflow(borderOverflowRect);
      }
    }

    // Add overflow from our caption.
    for (auto &caption : _captions)
    {
      if (caption)
      {
        AddOverflowFromContainedBox(*caption);
      }
    }

    // Add overflow from our sections.
    for (auto *section = TopSection(); section; section = SectionBelow(section))
    {
      AddOverflowFromContainedBox(*section);
    }
  }

  void RenderTable::ComputeIntrinsicLogicalWidths(LayoutUnit &minWidth, LayoutUnit &maxWidth,
                                                  TableIntrinsics intrinsics) const noexcept
  {
    RecalcSectionsIfNeeded();
    // FIXME: Do the recalc in BorderStart/BorderEnd and make those const_cast this call.
    // Then _borderStart/_borderEnd will be transparent a cache and it removes the possibility
    // of reading out stale values.
    const_cast<RenderTable *>(this)->RecalcBordersInRowDirection();
    // FIXME: Restructure the table layout code so that we can make this method const.
    const_cast<RenderTable *>(this)->_tableLayout->ComputeIntrinsicLogicalWidths(minWidth, maxWidth,
                                                                                 intrinsics);

    // FIXME: We should include captions widths here like we do in ComputePreferredLogicalWidths.
  }

  void RenderTable::ComputeIntrinsicLogicalWidths(LayoutUnit &minWidth, LayoutUnit &maxWidth) const noexcept
  {
    ComputeIntrinsicLogicalWidths(minWidth, maxWidth, TableIntrinsics::ForLayout);
  }

  void RenderTable::ComputeIntrinsicKeywordLogicalWidths(LayoutUnit &minWidth,
                                                         LayoutUnit &maxWidth) const noexcept
  {
    ComputeIntrinsicLogicalWidths(minWidth, maxWidth, TableIntrinsics::ForKeyword);
  }

  void RenderTable::ComputePreferredLogicalWidths() noexcept
  {
    krys_debug_assert(NeedsPreferredLogicalWidthsUpdate());

    ComputeIntrinsicLogicalWidths(_minPreferredLogicalWidth, _maxPreferredLogicalWidth);

    auto bordersPaddingAndSpacing = BordersPaddingAndSpacingInRowDirection();
    _minPreferredLogicalWidth += bordersPaddingAndSpacing;
    _maxPreferredLogicalWidth += bordersPaddingAndSpacing;

    _tableLayout->ApplyPreferredLogicalWidthQuirks(_minPreferredLogicalWidth, _maxPreferredLogicalWidth);

    for (auto &caption : _captions)
    {
      auto captionMinWidth = caption->MinPreferredLogicalWidth();
      // Only add fixed margins during preferred width calculation
      const auto &captionStyle = caption->Style();
      auto usedZoom = captionStyle.UsedZoomForLength();
      if (auto fixedMarginStart = captionStyle.MarginStart().TryFixed())
      {
        captionMinWidth += style::Evaluate<LayoutUnit>(*fixedMarginStart, usedZoom);
      }
      if (auto fixedMarginEnd = captionStyle.MarginEnd().TryFixed())
      {
        captionMinWidth += style::Evaluate<LayoutUnit>(*fixedMarginEnd, usedZoom);
      }
      _minPreferredLogicalWidth = std::max(_minPreferredLogicalWidth, captionMinWidth);
    }

    _maxPreferredLogicalWidth = std::max(_maxPreferredLogicalWidth, _minPreferredLogicalWidth);

    const auto &styleToUse = Style();
    // FIXME: This should probably be checking for IsSpecified since you should be able to use percentage or
    // calc values for min-width.
    if (auto fixedLogicalMinWidth = styleToUse.LogicalMinWidth().TryFixed();
        fixedLogicalMinWidth && fixedLogicalMinWidth->IsPositive())
    {
      _maxPreferredLogicalWidth =
        std::max(_maxPreferredLogicalWidth, AdjustContentBoxLogicalWidthForBoxSizing(*fixedLogicalMinWidth));
      _minPreferredLogicalWidth =
        std::max(_minPreferredLogicalWidth, AdjustContentBoxLogicalWidthForBoxSizing(*fixedLogicalMinWidth));
    }

    // FIXME: This should probably be checking for IsSpecified since you should be able to use percentage or
    // calc values for maxWidth.
    if (auto fixedLogicalMaxWidth = styleToUse.LogicalMaxWidth().TryFixed())
    {
      _maxPreferredLogicalWidth =
        std::min(_maxPreferredLogicalWidth, AdjustContentBoxLogicalWidthForBoxSizing(*fixedLogicalMaxWidth));
      _maxPreferredLogicalWidth = std::max(_maxPreferredLogicalWidth, _minPreferredLogicalWidth);
    }

    // FIXME: We should be adding BorderAndPaddingLogicalWidth here, but
    // _tableLayout->ComputePreferredLogicalWidths already does, so a bunch of tests break doing this naively.
    ClearNeedsPreferredWidthsUpdate();
  }

  RenderTableSection *RenderTable::TopNonEmptySection() const noexcept
  {
    auto *section = TopSection();
    if (section && !section->NumRows())
    {
      section = SectionBelow(section, SkipEmptySections::Yes);
    }
    return section;
  }

  RenderTableSection *RenderTable::BottomNonEmptySection() const noexcept
  {
    auto *section = BottomSection();
    if (section && !section->NumRows())
    {
      section = SectionAbove(section, SkipEmptySections::Yes);
    }
    return section;
  }

  void RenderTable::SplitColumn(uint32 position, uint32 firstSpan) noexcept
  {
    // We split the column at "position", taking "firstSpan" cells from the span.
    krys_debug_assert(_columns[position].Span > firstSpan);
    _columns.insert(_columns.begin() + position, {firstSpan});
    _columns[position + 1].Span -= firstSpan;

    // Propagate the change in our columns representation to the sections that don't need
    // cell recalc. If they do, they will be synced up directly with _columns later.
    for (auto &section : ChildrenOfType<RenderTableSection>(*this))
    {
      if (section.NeedsCellRecalc())
      {
        continue;
      }
      section.SplitColumn(position, firstSpan);
    }

    _columnPos.resize(NumEffCols() + 1);
  }

  void RenderTable::AppendColumn(uint32 span) noexcept
  {
    auto newColumnIndex = static_cast<uint32>(_columns.size());
    _columns.push_back({span});

    // Unless the table has cell(s) with colspan that exceed the number of columns afforded
    // by the other rows in the table we can use the fast path when mapping columns to effective columns.
    _hasCellColspanThatDeterminesTableWidth = _hasCellColspanThatDeterminesTableWidth || span > 1;

    // Propagate the change in our columns representation to the sections that don't need
    // cell recalc. If they do, they will be synced up directly with _columns later.
    for (auto &section : ChildrenOfType<RenderTableSection>(*this))
    {
      if (section.NeedsCellRecalc())
      {
        continue;
      }
      section.AppendColumn(newColumnIndex);
    }

    _columnPos.resize(NumEffCols() + 1);
  }

  RenderTableCol *RenderTable::FirstColumn() const noexcept
  {
    for (auto &child : ChildrenOfType<RenderObject>(*this))
    {
      if (auto *column = DynamicDowncast<RenderTableCol>(child))
      {
        return const_cast<RenderTableCol *>(column);
      }
    }
    return nullptr;
  }

  void RenderTable::UpdateColumnCache() const noexcept
  {
    krys_debug_assert(_hasColElements);
    krys_debug_assert(_columnRenderers.empty());
    krys_debug_assert(_effectiveColumnIndexMap.empty());
    krys_debug_assert(!_columnRenderersValid);

    uint32 columnIndex = 0;
    for (auto *columnRenderer = FirstColumn(); columnRenderer; columnRenderer = columnRenderer->NextColumn())
    {
      if (columnRenderer->IsTableColumnGroupWithColumnChildren())
      {
        continue;
      }
      _columnRenderers.push_back(WeakPtr<RenderTableCol>::WithRef(*columnRenderer));
      // FIXME: We should look to compute the effective column index successively from previous values instead
      // of calling ColToEffCol(), which is in O(NumEffCols()). Although it's unlikely that this is a hot
      // function.
      _effectiveColumnIndexMap.emplace(columnRenderer, ColToEffCol(columnIndex));
      columnIndex += columnRenderer->Span();
    }
    _columnRenderersValid = true;
  }

  uint32 RenderTable::EffectiveIndexOfColumn(const RenderTableCol &column) const noexcept
  {
    if (!_columnRenderersValid)
    {
      UpdateColumnCache();
    }
    const auto *columnToUse = &column;
    if (columnToUse->IsTableColumnGroupWithColumnChildren())
    {
      columnToUse = columnToUse->NextColumn(); // First column in column-group
    }
    auto it = _effectiveColumnIndexMap.find(columnToUse);
    krys_debug_assert(it != _effectiveColumnIndexMap.end());
    if (it == _effectiveColumnIndexMap.end())
    {
      return std::numeric_limits<uint32>::max();
    }
    return it->second;
  }

  LayoutUnit RenderTable::OffsetTopForColumn(const RenderTableCol &column) const noexcept
  {
    if (EffectiveIndexOfColumn(column) >= NumEffCols())
    {
      return 0_lu;
    }
    if (_columnOffsetTop >= 0)
    {
      krys_debug_assert(!NeedsLayout());
      return _columnOffsetTop;
    }
    auto *section = TopNonEmptySection();
    return _columnOffsetTop = section ? section->OffsetTop() : 0_lu;
  }

  LayoutUnit RenderTable::OffsetLeftForColumn(const RenderTableCol &column) const noexcept
  {
    auto columnIndex = EffectiveIndexOfColumn(column);
    if (columnIndex >= NumEffCols())
    {
      return 0_lu;
    }
    return _columnPos[columnIndex] + _hSpacing + BorderLeft();
  }

  LayoutUnit RenderTable::OffsetWidthForColumn(const RenderTableCol &column) const noexcept
  {
    const auto *currentColumn = &column;
    bool hasColumnChildren = currentColumn->IsTableColumnGroupWithColumnChildren();
    if (hasColumnChildren)
    {
      currentColumn = currentColumn->NextColumn(); // First column in column-group
    }
    auto numberOfEffectiveColumns = NumEffCols();
    krys_assert(_columnPos.size() >= numberOfEffectiveColumns + 1);
    LayoutUnit width;
    auto spacing = _hSpacing;
    while (currentColumn)
    {
      auto columnIndex = EffectiveIndexOfColumn(*currentColumn);
      auto span = currentColumn->Span();
      while (span && columnIndex < numberOfEffectiveColumns)
      {
        width += _columnPos[columnIndex + 1] - _columnPos[columnIndex] - spacing;
        span -= std::min(span, _columns[columnIndex].Span);
        ++columnIndex;
        if (span)
        {
          width += spacing;
        }
      }
      if (!hasColumnChildren)
      {
        break;
      }
      currentColumn = currentColumn->NextColumn();
      if (!currentColumn || currentColumn->IsTableColumnGroup())
      {
        break;
      }
      width += spacing;
    }
    return width;
  }

  LayoutUnit RenderTable::OffsetHeightForColumn(const RenderTableCol &column) const noexcept
  {
    if (EffectiveIndexOfColumn(column) >= NumEffCols())
    {
      return 0_lu;
    }
    if (_columnOffsetHeight >= 0)
    {
      krys_debug_assert(!NeedsLayout());
      return _columnOffsetHeight;
    }
    LayoutUnit height;
    for (auto *section = TopSection(); section; section = SectionBelow(section))
    {
      height += section->OffsetHeight();
    }
    _columnOffsetHeight = height;
    return _columnOffsetHeight;
  }

  RenderTableCol *RenderTable::SlowColElement(uint32 col, bool *startEdge, bool *endEdge) const noexcept
  {
    krys_debug_assert(_hasColElements);

    if (!_columnRenderersValid)
    {
      UpdateColumnCache();
    }

    uint32 columnCount = 0;
    for (auto &columnRenderer : _columnRenderers)
    {
      if (!columnRenderer)
      {
        continue;
      }
      auto span = columnRenderer->Span();
      auto startCol = columnCount;
      krys_debug_assert(span >= 1);
      auto endCol = columnCount + span - 1;
      columnCount += span;
      if (columnCount > col)
      {
        if (startEdge)
        {
          *startEdge = startCol == col;
        }
        if (endEdge)
        {
          *endEdge = endCol == col;
        }
        return columnRenderer.get();
      }
    }
    return nullptr;
  }

  void RenderTable::RecalcSections() const noexcept
  {
    krys_debug_assert(_needsSectionRecalc);

    _head = nullptr;
    _foot = nullptr;
    _firstBody = nullptr;
    _hasColElements = false;
    _hasCellColspanThatDeterminesTableWidth = HasCellColspanThatDeterminesTableWidth();

    // We need to get valid pointers to caption, head, foot and first body again
    for (auto *child = FirstChildBox(); child; child = child->NextSiblingBox())
    {
      switch (child->Style().Display())
      {
        case DisplayType::TableColumn:
        case DisplayType::TableColumnGroup: _hasColElements = true; break;
        case DisplayType::TableHeaderGroup:
          if (auto *section = DynamicDowncast<RenderTableSection>(*child))
          {
            if (!_head)
            {
              _head = WeakPtr<RenderTableSection>::WithRef(*section);
            }
            else if (!_firstBody)
            {
              _firstBody = WeakPtr<RenderTableSection>::WithRef(*section);
            }
            section->RecalcCellsIfNeeded();
          }
          break;
        case DisplayType::TableFooterGroup:
          if (auto *section = DynamicDowncast<RenderTableSection>(*child))
          {
            if (!_foot)
            {
              _foot = WeakPtr<RenderTableSection>::WithRef(*section);
            }
            else if (!_firstBody)
            {
              _firstBody = WeakPtr<RenderTableSection>::WithRef(*section);
            }
            section->RecalcCellsIfNeeded();
          }
          break;
        case DisplayType::TableRowGroup:
          if (auto *section = DynamicDowncast<RenderTableSection>(*child))
          {
            if (!_firstBody)
            {
              _firstBody = WeakPtr<RenderTableSection>::WithRef(*section);
            }
            section->RecalcCellsIfNeeded();
          }
          break;
        default: break;
      }
    }

    // repair column count (AddChild can grow it too much, because it always adds elements to the last row of
    // a section)
    uint32 maxCols = 0;
    for (auto &section : ChildrenOfType<RenderTableSection>(*this))
    {
      auto sectionCols = section.NumColumns();
      if (sectionCols > maxCols)
      {
        maxCols = sectionCols;
      }
    }

    _columns.resize(maxCols);
    _columnPos.resize(maxCols + 1);

    // Now that we know the number of maximum number of columns, let's shrink the sections grids if needed.
    for (auto &section : ChildrenOfType<RenderTableSection>(const_cast<RenderTable &>(*this)))
    {
      section.RemoveRedundantColumns();
    }

    _needsSectionRecalc = false;
  }

  LayoutUnit RenderTable::CalcBorderStart() const noexcept
  {
    if (!CollapseBorders())
    {
      return RenderBlock::BorderStart();
    }

    // Determined by the first cell of the first row. See the CSS 2.1 spec, section 17.6.2.
    if (!NumEffCols())
    {
      return 0_lu;
    }

    float borderWidth = 0;

    const auto &tableStartBorder = Style().BorderStart();
    if (tableStartBorder.HasHiddenStyle())
    {
      return 0_lu;
    }
    if (tableStartBorder.HasVisibleStyle())
    {
      borderWidth = style::Evaluate<float>(tableStartBorder.Width, style::ZoomNeeded {});
    }

    if (auto *column = ColElement(0))
    {
      // FIXME: We don't account for direction on columns and column groups.
      const auto &columnAdjoiningBorder = column->Style().BorderStart();
      if (columnAdjoiningBorder.HasHiddenStyle())
      {
        return 0_lu;
      }
      if (columnAdjoiningBorder.HasVisibleStyle())
      {
        borderWidth =
          std::max(borderWidth, style::Evaluate<float>(columnAdjoiningBorder.Width, style::ZoomNeeded {}));
      }
      // FIXME: This logic doesn't properly account for the first column in the first column-group case.
    }

    if (const auto *topNonEmptySection = TopNonEmptySection())
    {
      const auto &sectionAdjoiningBorder = topNonEmptySection->BorderAdjoiningTableStart();
      if (sectionAdjoiningBorder.HasHiddenStyle())
      {
        return 0_lu;
      }

      if (sectionAdjoiningBorder.HasVisibleStyle())
      {
        borderWidth =
          std::max(borderWidth, style::Evaluate<float>(sectionAdjoiningBorder.Width, style::ZoomNeeded {}));
      }

      if (const auto *adjoiningStartCell = topNonEmptySection->CellAt(0, 0).PrimaryCell())
      {
        // FIXME: Make this work with perpendicular and flipped cells.
        const auto &startCellAdjoiningBorder = adjoiningStartCell->BorderAdjoiningTableStart();
        if (startCellAdjoiningBorder.HasHiddenStyle())
        {
          return 0_lu;
        }

        const auto &firstRowAdjoiningBorder = adjoiningStartCell->Row()->BorderAdjoiningTableStart();
        if (firstRowAdjoiningBorder.HasHiddenStyle())
        {
          return 0_lu;
        }

        if (startCellAdjoiningBorder.HasVisibleStyle())
        {
          borderWidth = std::max(
            borderWidth, style::Evaluate<float>(startCellAdjoiningBorder.Width, style::ZoomNeeded {}));
        }
        if (firstRowAdjoiningBorder.HasVisibleStyle())
        {
          borderWidth = std::max(borderWidth,
                                 style::Evaluate<float>(firstRowAdjoiningBorder.Width, style::ZoomNeeded {}));
        }
      }
    }
    return CollapsedBorderValue::AdjustedCollapsedBorderWidth(borderWidth, GetDocument().DeviceScaleFactor(),
                                                              GetWritingMode().IsInlineFlipped());
  }

  LayoutUnit RenderTable::CalcBorderEnd() const noexcept
  {
    if (!CollapseBorders())
    {
      return RenderBlock::BorderEnd();
    }

    // Determined by the last cell of the first row. See the CSS 2.1 spec, section 17.6.2.
    if (!NumEffCols())
    {
      return 0_lu;
    }

    float borderWidth = 0;

    const auto &tableEndBorder = Style().BorderEnd();
    if (tableEndBorder.HasHiddenStyle())
    {
      return 0_lu;
    }
    if (tableEndBorder.HasVisibleStyle())
    {
      borderWidth = style::Evaluate<float>(tableEndBorder.Width, style::ZoomNeeded {});
    }

    auto endColumn = NumEffCols() - 1;
    if (auto *column = ColElement(endColumn))
    {
      // FIXME: We don't account for direction on columns and column groups.
      const auto &columnAdjoiningBorder = column->Style().BorderEnd();
      if (columnAdjoiningBorder.HasHiddenStyle())
      {
        return 0_lu;
      }
      if (columnAdjoiningBorder.HasVisibleStyle())
      {
        borderWidth =
          std::max(borderWidth, style::Evaluate<float>(columnAdjoiningBorder.Width, style::ZoomNeeded {}));
      }
      // FIXME: This logic doesn't properly account for the last column in the last column-group case.
    }

    if (const auto *topNonEmptySection = TopNonEmptySection())
    {
      const auto &sectionAdjoiningBorder = topNonEmptySection->BorderAdjoiningTableEnd();
      if (sectionAdjoiningBorder.HasHiddenStyle())
      {
        return 0_lu;
      }

      if (sectionAdjoiningBorder.HasVisibleStyle())
      {
        borderWidth =
          std::max(borderWidth, style::Evaluate<float>(sectionAdjoiningBorder.Width, style::ZoomNeeded {}));
      }

      if (const auto *adjoiningEndCell = topNonEmptySection->CellAt(0, LastColumnIndex()).PrimaryCell())
      {
        // FIXME: Make this work with perpendicular and flipped cells.
        const auto &endCellAdjoiningBorder = adjoiningEndCell->BorderAdjoiningTableEnd();
        if (endCellAdjoiningBorder.HasHiddenStyle())
        {
          return 0_lu;
        }

        const auto &firstRowAdjoiningBorder = adjoiningEndCell->Row()->BorderAdjoiningTableEnd();
        if (firstRowAdjoiningBorder.HasHiddenStyle())
        {
          return 0_lu;
        }

        if (endCellAdjoiningBorder.HasVisibleStyle())
        {
          borderWidth =
            std::max(borderWidth, style::Evaluate<float>(endCellAdjoiningBorder.Width, style::ZoomNeeded {}));
        }
        if (firstRowAdjoiningBorder.HasVisibleStyle())
        {
          borderWidth = std::max(borderWidth,
                                 style::Evaluate<float>(firstRowAdjoiningBorder.Width, style::ZoomNeeded {}));
        }
      }
    }
    return CollapsedBorderValue::AdjustedCollapsedBorderWidth(borderWidth, GetDocument().DeviceScaleFactor(),
                                                              !GetWritingMode().IsInlineFlipped());
  }

  void RenderTable::RecalcBordersInRowDirection() noexcept
  {
    // FIXME: We need to compute the collapsed before / after borders in the same fashion.
    _borderStart = CalcBorderStart();
    _borderEnd = CalcBorderEnd();
  }

  LayoutUnit RenderTable::BorderBefore() const noexcept
  {
    if (CollapseBorders())
    {
      RecalcSectionsIfNeeded();
      return OuterBorderBefore();
    }
    return RenderBlock::BorderBefore();
  }

  LayoutUnit RenderTable::BorderAfter() const noexcept
  {
    if (CollapseBorders())
    {
      RecalcSectionsIfNeeded();
      return OuterBorderAfter();
    }
    return RenderBlock::BorderAfter();
  }

  LayoutUnit RenderTable::BorderBottom() const noexcept
  {
    auto writingMode = GetWritingMode();
    if (writingMode.IsHorizontal())
    {
      return writingMode.IsBlockTopToBottom() ? BorderAfter() : BorderBefore();
    }
    return writingMode.IsInlineTopToBottom() ? BorderEnd() : BorderStart();
  }

  LayoutUnit RenderTable::BorderLeft() const noexcept
  {
    auto writingMode = GetWritingMode();
    if (writingMode.IsHorizontal())
    {
      return writingMode.IsInlineLeftToRight() ? BorderStart() : BorderEnd();
    }
    return writingMode.IsBlockLeftToRight() ? BorderBefore() : BorderAfter();
  }

  LayoutUnit RenderTable::BorderRight() const noexcept
  {
    auto writingMode = GetWritingMode();
    if (writingMode.IsHorizontal())
    {
      return writingMode.IsInlineLeftToRight() ? BorderEnd() : BorderStart();
    }
    return writingMode.IsBlockLeftToRight() ? BorderAfter() : BorderBefore();
  }

  LayoutUnit RenderTable::BorderTop() const noexcept
  {
    auto writingMode = GetWritingMode();
    if (writingMode.IsHorizontal())
    {
      return writingMode.IsBlockTopToBottom() ? BorderBefore() : BorderAfter();
    }
    return writingMode.IsInlineTopToBottom() ? BorderStart() : BorderEnd();
  }

  RectEdges<LayoutUnit> RenderTable::BorderWidths() const noexcept
  {
    return {BorderTop(), BorderRight(), BorderBottom(), BorderLeft()};
  }

  LayoutUnit RenderTable::PaddingTop() const noexcept
  {
    if (CollapseBorders())
    {
      return {};
    }
    return RenderBlock::PaddingTop();
  }

  LayoutUnit RenderTable::PaddingBottom() const noexcept
  {
    if (CollapseBorders())
    {
      return {};
    }
    return RenderBlock::PaddingBottom();
  }

  LayoutUnit RenderTable::PaddingLeft() const noexcept
  {
    if (CollapseBorders())
    {
      return {};
    }
    return RenderBlock::PaddingLeft();
  }

  LayoutUnit RenderTable::PaddingRight() const noexcept
  {
    if (CollapseBorders())
    {
      return {};
    }
    return RenderBlock::PaddingRight();
  }

  LayoutUnit RenderTable::PaddingAfter() const noexcept
  {
    if (CollapseBorders())
    {
      return {};
    }
    return RenderBlock::PaddingAfter();
  }

  LayoutUnit RenderTable::PaddingBefore() const noexcept
  {
    if (CollapseBorders())
    {
      return {};
    }
    return RenderBlock::PaddingBefore();
  }

  LayoutUnit RenderTable::PaddingStart() const noexcept
  {
    if (CollapseBorders())
    {
      return {};
    }
    return RenderBlock::PaddingStart();
  }

  LayoutUnit RenderTable::PaddingEnd() const noexcept
  {
    if (CollapseBorders())
    {
      return {};
    }
    return RenderBlock::PaddingEnd();
  }

  LayoutUnit RenderTable::BordersPaddingAndSpacingInRowDirection() const noexcept
  {
    // 'border-spacing' only applies to separate borders (see 17.6.1 The separated borders model).
    return BorderStart() + BorderEnd()
           + (CollapseBorders() ? 0_lu : (PaddingStart() + PaddingEnd() + BorderSpacingInRowDirection()));
  }

  LayoutUnit RenderTable::OuterBorderBefore() const noexcept
  {
    if (!CollapseBorders())
    {
      return 0_lu;
    }
    LayoutUnit borderWidth;
    if (auto *topSection = TopSection())
    {
      borderWidth = topSection->OuterBorderBefore();
      if (borderWidth < 0)
      {
        return 0_lu; // Overridden by hidden
      }
    }
    const auto &tb = Style().BorderBefore();
    if (tb.HasHiddenStyle())
    {
      return 0_lu;
    }
    if (tb.HasVisibleStyle())
    {
      auto collapsedBorderWidth =
        std::max(borderWidth, LayoutUnit(style::Evaluate<float>(tb.Width, style::ZoomNeeded {}) / 2));
      borderWidth = FloorToDevicePixel(collapsedBorderWidth, GetDocument().DeviceScaleFactor());
    }
    return borderWidth;
  }

  LayoutUnit RenderTable::OuterBorderAfter() const noexcept
  {
    if (!CollapseBorders())
    {
      return 0_lu;
    }
    LayoutUnit borderWidth;

    if (auto *section = BottomSection())
    {
      borderWidth = section->OuterBorderAfter();
      if (borderWidth < 0)
      {
        return 0_lu; // Overridden by hidden
      }
    }
    const auto &tb = Style().BorderAfter();
    if (tb.HasHiddenStyle())
    {
      return 0_lu;
    }
    if (tb.HasVisibleStyle())
    {
      auto deviceScaleFactor = GetDocument().DeviceScaleFactor();
      auto collapsedBorderWidth = std::max(
        borderWidth,
        LayoutUnit((style::Evaluate<float>(tb.Width, style::ZoomNeeded {}) + (1 / deviceScaleFactor)) / 2));
      borderWidth = FloorToDevicePixel(collapsedBorderWidth, deviceScaleFactor);
    }
    return borderWidth;
  }

  LayoutUnit RenderTable::OuterBorderStart() const noexcept
  {
    if (!CollapseBorders())
    {
      return 0_lu;
    }

    LayoutUnit borderWidth;

    const auto &tb = Style().BorderStart();
    if (tb.HasHiddenStyle())
    {
      return 0_lu;
    }
    if (tb.HasVisibleStyle())
    {
      return CollapsedBorderValue::AdjustedCollapsedBorderWidth(
        style::Evaluate<float>(tb.Width, style::ZoomNeeded {}), GetDocument().DeviceScaleFactor(),
        GetWritingMode().IsInlineFlipped());
    }

    bool allHidden = true;
    for (auto *section = TopSection(); section; section = SectionBelow(section))
    {
      auto sw = section->OuterBorderStart();
      if (sw < 0)
      {
        continue;
      }
      allHidden = false;
      borderWidth = std::max(borderWidth, sw);
    }
    if (allHidden)
    {
      return 0_lu;
    }

    return borderWidth;
  }

  LayoutUnit RenderTable::OuterBorderEnd() const noexcept
  {
    if (!CollapseBorders())
    {
      return 0_lu;
    }

    LayoutUnit borderWidth;

    const auto &tb = Style().BorderEnd();
    if (tb.HasHiddenStyle())
    {
      return 0_lu;
    }
    if (tb.HasVisibleStyle())
    {
      return CollapsedBorderValue::AdjustedCollapsedBorderWidth(
        style::Evaluate<float>(tb.Width, style::ZoomNeeded {}), GetDocument().DeviceScaleFactor(),
        !GetWritingMode().IsInlineFlipped());
    }

    bool allHidden = true;
    for (auto *section = TopSection(); section; section = SectionBelow(section))
    {
      auto sw = section->OuterBorderEnd();
      if (sw < 0)
      {
        continue;
      }
      allHidden = false;
      borderWidth = std::max(borderWidth, sw);
    }
    if (allHidden)
    {
      return 0_lu;
    }

    return borderWidth;
  }

  LayoutUnit RenderTable::OuterBorderBottom() const noexcept
  {
    auto writingMode = GetWritingMode();
    if (writingMode.IsHorizontal())
    {
      return writingMode.IsBlockTopToBottom() ? OuterBorderAfter() : OuterBorderBefore();
    }
    return writingMode.IsInlineTopToBottom() ? OuterBorderEnd() : OuterBorderStart();
  }

  LayoutUnit RenderTable::OuterBorderLeft() const noexcept
  {
    auto writingMode = GetWritingMode();
    if (writingMode.IsHorizontal())
    {
      return writingMode.IsInlineLeftToRight() ? OuterBorderStart() : OuterBorderEnd();
    }
    return writingMode.IsBlockLeftToRight() ? OuterBorderBefore() : OuterBorderAfter();
  }

  LayoutUnit RenderTable::OuterBorderRight() const noexcept
  {
    auto writingMode = GetWritingMode();
    if (writingMode.IsHorizontal())
    {
      return writingMode.IsInlineLeftToRight() ? OuterBorderEnd() : OuterBorderStart();
    }
    return writingMode.IsBlockLeftToRight() ? OuterBorderAfter() : OuterBorderBefore();
  }

  LayoutUnit RenderTable::OuterBorderTop() const noexcept
  {
    auto writingMode = GetWritingMode();
    if (writingMode.IsHorizontal())
    {
      return writingMode.IsBlockTopToBottom() ? OuterBorderBefore() : OuterBorderAfter();
    }
    return writingMode.IsInlineTopToBottom() ? OuterBorderStart() : OuterBorderEnd();
  }

  RenderTableSection *RenderTable::SectionAbove(const RenderTableSection *section,
                                                SkipEmptySections skipEmptySections) const noexcept
  {
    RecalcSectionsIfNeeded();

    if (section == _head.get())
    {
      return nullptr;
    }

    auto *prevSection = section == _foot.get() ? LastChild() : section->PreviousSibling();
    while (prevSection)
    {
      auto *tableSection = DynamicDowncast<RenderTableSection>(*prevSection);
      if (tableSection && prevSection != _head.get() && prevSection != _foot.get()
          && (skipEmptySections == SkipEmptySections::No || tableSection->NumRows()))
      {
        return tableSection;
      }
      prevSection = prevSection->PreviousSibling();
    }
    if (!prevSection && _head && (skipEmptySections == SkipEmptySections::No || _head->NumRows()))
    {
      return _head.get();
    }
    return nullptr;
  }

  RenderTableSection *RenderTable::SectionBelow(const RenderTableSection *section,
                                                SkipEmptySections skipEmptySections) const noexcept
  {
    RecalcSectionsIfNeeded();

    if (section == _foot.get())
    {
      return nullptr;
    }

    auto *nextSection = section == _head.get() ? FirstChild() : section->NextSibling();
    while (nextSection)
    {
      auto *tableSection = DynamicDowncast<RenderTableSection>(*nextSection);
      if (tableSection && nextSection != _head.get() && nextSection != _foot.get()
          && (skipEmptySections == SkipEmptySections::No || tableSection->NumRows()))
      {
        return tableSection;
      }
      nextSection = nextSection->NextSibling();
    }
    if (!nextSection && _foot && (skipEmptySections == SkipEmptySections::No || _foot->NumRows()))
    {
      return _foot.get();
    }
    return nullptr;
  }

  RenderTableCell *RenderTable::CellAbove(const RenderTableCell *cell) const noexcept
  {
    RecalcSectionsIfNeeded();

    // Find the section and row to look in
    auto r = cell->RowIndex();
    RenderTableSection *section = nullptr;
    uint32 rAbove = 0;
    if (r > 0)
    {
      // cell is not in the first row, so use the above row in its own section
      section = cell->Section();
      rAbove = r - 1;
    }
    else
    {
      section = SectionAbove(cell->Section(), SkipEmptySections::Yes);
      if (section)
      {
        krys_debug_assert(section->NumRows());
        rAbove = section->NumRows() - 1;
      }
    }

    // Look up the cell in the section's grid, which requires effective col index
    if (section)
    {
      auto effCol = ColToEffCol(cell->Col());
      return section->CellAt(rAbove, effCol).PrimaryCell();
    }
    return nullptr;
  }

  RenderTableCell *RenderTable::CellBelow(const RenderTableCell *cell) const noexcept
  {
    RecalcSectionsIfNeeded();

    // Find the section and row to look in
    auto r = cell->RowIndex() + cell->RowSpan() - 1;
    RenderTableSection *section = nullptr;
    uint32 rBelow = 0;
    if (r < cell->Section()->NumRows() - 1)
    {
      // The cell is not in the last row, so use the next row in the section.
      section = cell->Section();
      rBelow = r + 1;
    }
    else
    {
      section = SectionBelow(cell->Section(), SkipEmptySections::Yes);
      if (section)
      {
        rBelow = 0;
      }
    }

    // Look up the cell in the section's grid, which requires effective col index
    if (section)
    {
      auto effCol = ColToEffCol(cell->Col());
      return section->CellAt(rBelow, effCol).PrimaryCell();
    }
    return nullptr;
  }

  RenderTableCell *RenderTable::CellBefore(const RenderTableCell *cell) const noexcept
  {
    RecalcSectionsIfNeeded();

    auto *section = cell->Section();
    auto effCol = ColToEffCol(cell->Col());
    if (!effCol)
    {
      return nullptr;
    }

    // If we hit a colspan back up to a real cell.
    return section->CellAt(cell->RowIndex(), effCol - 1).PrimaryCell();
  }

  RenderTableCell *RenderTable::CellAfter(const RenderTableCell *cell) const noexcept
  {
    RecalcSectionsIfNeeded();

    auto effCol = ColToEffCol(cell->Col() + cell->ColSpan());
    if (effCol >= NumEffCols())
    {
      return nullptr;
    }
    return cell->Section()->PrimaryCellAt(cell->RowIndex(), effCol);
  }

  Maybe<LayoutUnit> RenderTable::FirstLineBaseline() const noexcept
  {
    // The baseline of a 'table' is the same as the 'inline-table' baseline per CSS 3 Flexbox (CSS 2.1
    // doesn't define the baseline of a 'table' only an 'inline-table').
    // This is also needed to properly determine the baseline of a cell if it has a table child.

    if ((IsWritingModeRoot() && !IsFlexItem()) || ShouldApplyLayoutContainment())
    {
      return null;
    }

    RecalcSectionsIfNeeded();

    auto *topNonEmptySection = TopNonEmptySection();
    if (!topNonEmptySection)
    {
      return null;
    }

    Maybe<LayoutUnit> baseline;
    if (auto firstLineBaseline = topNonEmptySection->FirstLineBaseline())
    {
      baseline = firstLineBaseline;
    }
    else if (topNonEmptySection->FirstRow() && !topNonEmptySection->FirstRow()->FirstCell())
    {
      // Other browsers use the top of the section as the baseline if its first row is empty of cells or
      // content. The baseline of an empty row isn't specified by CSS 2.1.
      baseline = 0_lu;
    }

    if (!baseline)
    {
      return null;
    }
    // Subpixel inline layout is off, so the section's top is snapped to an integer.
    return LayoutUnit(topNonEmptySection->LogicalTop().ToInt()) + *baseline;
  }

  Maybe<LayoutUnit> RenderTable::LastLineBaseline() const noexcept
  {
    if (IsWritingModeRoot() || ShouldApplyLayoutContainment())
    {
      return null;
    }

    RecalcSectionsIfNeeded();

    auto *tableSection = BottomNonEmptySection();
    if (!tableSection)
    {
      return null;
    }

    if (auto lastLineBaseline = tableSection->LastLineBaseline())
    {
      // Subpixel inline layout is off, so the section's top is snapped to an integer.
      return LayoutUnit(tableSection->LogicalTop().ToInt()) + *lastLineBaseline;
    }

    return null;
  }

  LayoutRect RenderTable::OverflowClipRect(const LayoutPoint &location,
                                           OverlayScrollbarSizeRelevancy relevancy) const noexcept
  {
    LayoutRect rect;
    // Don't clip out the table's side of the collapsed borders if we're in the paint phase that will ask the
    // sections to paint them. Likewise, if we're self-painting we avoid clipping them out as the clip rect
    // that will be passed down to child layers from RenderLayer will do that instead.
    // FIXME: The paint phase that paints the collapsed borders (PaintPhase::ChildBlockBackgrounds), once
    // there's painting.
    if (Layer() && Layer()->IsSelfPaintingLayer())
    {
      rect = BorderBoxRect();
      rect.SetLocation(location + rect.Location());
    }
    else
    {
      rect = RenderBox::OverflowClipRect(location, relevancy);
    }

    // If we have a caption, expand the clip to include the caption.
    // FIXME: Technically this is wrong, but it's virtually impossible to fix this
    // for real until captions have been re-written.
    // FIXME: This code assumes (like all our other caption code) that only top/bottom are
    // supported.  When we actually support left/right and stop mapping them to top/bottom,
    // we might have to hack this code first (depending on what order we do these bug fixes in).
    if (!_captions.empty())
    {
      if (GetWritingMode().IsHorizontal())
      {
        rect.SetHeight(Height());
        rect.SetY(location.Y());
      }
      else
      {
        rect.SetWidth(Width());
        rect.SetX(location.X());
      }
    }

    return rect;
  }

  // Collect all the unique border values that we want to paint in a sorted list.
  void RenderTable::RecalcCollapsedBorders() noexcept
  {
    if (_collapsedBordersValid)
    {
      return;
    }
    _collapsedBorders.clear();
    for (auto &section : ChildrenOfType<RenderTableSection>(*this))
    {
      for (RenderTableRow *row = section.FirstRow(); row; row = row->NextRow())
      {
        for (RenderTableCell *cell = row->FirstCell(); cell; cell = cell->NextCell())
        {
          krys_debug_assert(cell->Table() == this);
          cell->CollectBorderValues(_collapsedBorders);
        }
      }
    }
    RenderTableCell::SortBorderValues(_collapsedBorders);
    _collapsedBordersValid = true;
  }

  void RenderTable::Paint(PaintInfo &paintInfo, const LayoutPoint &paintOffset) noexcept
  {
    // FIXME: Tables inside skipped content behind continuations (the isSkippedContent check) come with
    // content-visibility's skipped content.

    LayoutPoint adjustedPaintOffset = paintOffset + Location();

    PaintPhase paintPhase = paintInfo.Phase;

    if (!IsDocumentElementRenderer())
    {
      LayoutRect overflowBox = VisualOverflowRect();
      FlipForWritingMode(overflowBox);
      overflowBox.MoveBy(adjustedPaintOffset);
      if (!overflowBox.Intersects(paintInfo.Rect))
      {
        return;
      }
    }

    bool pushedClip = PushContentsClip(paintInfo, adjustedPaintOffset);
    PaintObject(paintInfo, adjustedPaintOffset);
    if (pushedClip)
    {
      PopContentsClip(paintInfo, paintPhase, adjustedPaintOffset);
    }
  }

  void RenderTable::PaintObject(PaintInfo &paintInfo, const LayoutPoint &paintOffset) noexcept
  {
    PaintPhase paintPhase = paintInfo.Phase;
    if ((paintPhase == PaintPhase::BlockBackground || paintPhase == PaintPhase::ChildBlockBackground)
        && HasVisibleBoxDecorations() && Style().UsedVisibility() == Visibility::Visible)
    {
      PaintBoxDecorations(paintInfo, paintOffset);
    }

    if (paintPhase == PaintPhase::Mask)
    {
      // FIXME: Masks (paintMask) come with masks.
      return;
    }

    // FIXME: Accessibility bounds come with accessibility.

    // We're done.  We don't bother painting any children.
    if (paintPhase == PaintPhase::BlockBackground)
    {
      return;
    }

    // We don't paint our own background, but we do let the kids paint their backgrounds.
    if (paintPhase == PaintPhase::ChildBlockBackgrounds)
    {
      paintPhase = PaintPhase::ChildBlockBackground;
    }

    PaintInfo info(paintInfo);
    info.Phase = paintPhase;
    info.UpdateSubtreePaintRootForChildren(this);

    for (auto &box : ChildrenOfType<RenderBox>(*this))
    {
      if (!box.HasSelfPaintingLayer() && (box.IsRenderTableSection() || box.IsRenderTableCaption()))
      {
        LayoutPoint childPoint = FlipForWritingModeForChild(box, paintOffset);
        box.Paint(info, childPoint);
      }
    }

    if (CollapseBorders() && paintPhase == PaintPhase::ChildBlockBackground
        && Style().UsedVisibility() == Visibility::Visible)
    {
      RecalcCollapsedBorders();
      // Using our cached sorted styles, we then do individual passes,
      // painting each style of border from lowest precedence to highest precedence.
      info.Phase = PaintPhase::CollapsedTableBorders;
      size_t count = _collapsedBorders.size();
      for (size_t i = 0; i < count; ++i)
      {
        _currentBorder = &_collapsedBorders[i];
        for (RenderTableSection *section = BottomSection(); section; section = SectionAbove(section))
        {
          LayoutPoint childPoint = FlipForWritingModeForChild(*section, paintOffset);
          section->Paint(info, childPoint);
        }
      }
      _currentBorder = nullptr;
    }

    // Paint outline.
    if ((paintPhase == PaintPhase::Outline || paintPhase == PaintPhase::SelfOutline) && HasOutline()
        && Style().UsedVisibility() == Visibility::Visible)
    {
      PaintOutline(paintInfo, LayoutRect(paintOffset, Size()));
    }
  }

  void RenderTable::AdjustBorderBoxRectForPainting(LayoutRect &rect) noexcept
  {
    for (auto &weakCaption : _captions)
    {
      auto *caption = weakCaption.get();
      if (!caption)
      {
        continue;
      }
      LayoutUnit captionLogicalHeight =
        caption->LogicalHeight() + caption->MarginBefore() + caption->MarginAfter();
      bool captionIsBefore =
        (caption->Style().GetCaptionSide() != CaptionSide::Bottom) ^ GetWritingMode().IsBlockFlipped();
      if (GetWritingMode().IsHorizontal())
      {
        rect.SetHeight(rect.Height() - captionLogicalHeight);
        if (captionIsBefore)
        {
          rect.Move(0_lu, captionLogicalHeight);
        }
      }
      else
      {
        rect.SetWidth(rect.Width() - captionLogicalHeight);
        if (captionIsBefore)
        {
          rect.Move(captionLogicalHeight, 0_lu);
        }
      }
    }

    RenderBlock::AdjustBorderBoxRectForPainting(rect);
  }

  void RenderTable::PaintBoxDecorations(PaintInfo &paintInfo, const LayoutPoint &paintOffset) noexcept
  {
    if (!paintInfo.ShouldPaintWithinRoot(*this))
    {
      return;
    }

    LayoutRect rect(paintOffset, Size());
    AdjustBorderBoxRectForPainting(rect);

    BackgroundPainter backgroundPainter {*this, paintInfo};

    auto bleedAvoidance = DetermineBleedAvoidance(paintInfo.Context());
    if (!BackgroundPainter::BoxShadowShouldBeAppliedToBackground(*this, rect.Location(), bleedAvoidance, {}))
    {
      backgroundPainter.PaintBoxShadow(rect, Style(), style::ShadowStyle::Normal);
    }

    GraphicsContextStateSaver stateSaver(paintInfo.Context(), false);
    if (bleedAvoidance == BleedAvoidance::UseTransparencyLayer)
    {
      // To avoid the background color bleeding out behind the border, we'll render background and border
      // into a transparency layer, and then clip that in one go (which requires setting up the clip before
      // beginning the layer).
      stateSaver.Save();
      auto borderShape = BorderShape::ShapeForBorderRect(Style(), rect);
      borderShape.ClipToOuterShape(paintInfo.Context(), GetDocument().DeviceScaleFactor());
      paintInfo.Context().BeginTransparencyLayer(1);
    }

    backgroundPainter.PaintBackground(rect, bleedAvoidance);
    backgroundPainter.PaintBoxShadow(rect, Style(), style::ShadowStyle::Inset);

    if (Style().HasVisibleBorderDecoration() && !CollapseBorders())
    {
      BorderPainter {*this, paintInfo}.PaintBorder(rect, Style());
    }

    if (bleedAvoidance == BleedAvoidance::UseTransparencyLayer)
    {
      paintInfo.Context().EndTransparencyLayer();
    }
  }

  bool RenderTable::NodeAtPoint(const HitTestRequest &request, HitTestResult &result,
                                const HitTestLocation &locationInContainer,
                                const LayoutPoint &accumulatedOffset, HitTestAction action) noexcept
  {
    LayoutPoint adjustedLocation = accumulatedOffset + Location();

    // Check kids first.
    if (!HasNonVisibleOverflow() || locationInContainer.Intersects(OverflowClipRect(adjustedLocation)))
    {
      for (RenderObject *child = LastChild(); child; child = child->PreviousSibling())
      {
        auto *box = DynamicDowncast<RenderBox>(*child);
        if (box && !box->HasSelfPaintingLayer()
            && (box->IsRenderTableSection() || box->IsRenderTableCaption()))
        {
          LayoutPoint childPoint = FlipForWritingModeForChild(*box, adjustedLocation);
          if (box->NodeAtPoint(request, result, locationInContainer, childPoint, action))
          {
            UpdateHitTestResult(result, ToLayoutPoint(locationInContainer.Point() - childPoint));
            return true;
          }
        }
      }
    }

    // Check our bounds next.
    LayoutRect boundsRect(adjustedLocation, Size());
    if (VisibleToHitTesting(&request)
        && (action == HitTestBlockBackground || action == HitTestChildBlockBackground)
        && locationInContainer.Intersects(boundsRect))
    {
      UpdateHitTestResult(result,
                          FlipForWritingMode(locationInContainer.Point() - ToLayoutSize(adjustedLocation)));
      if (result.AddNodeToListBasedTestResult(NodeForHitTest(), request, locationInContainer, boundsRect)
          == HitTestProgress::Stop)
      {
        return true;
      }
    }

    return false;
  }
}
