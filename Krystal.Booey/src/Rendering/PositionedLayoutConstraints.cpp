#include "Krystal.Booey/Rendering/PositionedLayoutConstraints.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Rendering/RenderGrid.hpp"
#include "Krystal.Booey/Rendering/RenderInline.hpp"
#include "Krystal.Booey/Rendering/RenderLayer.hpp"
#include "Krystal.Booey/Rendering/RenderTableRow.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Style/AnchorPositionEvaluator.hpp"
#include <utility>

namespace krys::boo
{
  PositionedLayoutConstraints::PositionedLayoutConstraints(const RenderBox &renderer,
                                                           LogicalBoxAxis selfAxis) noexcept
      : PositionedLayoutConstraints(renderer, renderer.Style(), selfAxis)
  {
  }

  PositionedLayoutConstraints::PositionedLayoutConstraints(const RenderBox &renderer,
                                                           const RenderStyle &style,
                                                           LogicalBoxAxis selfAxis) noexcept
      : _renderer(renderer),
        // Using containingBlock() would be wrong for relpositioned inlines.
        _container(&Downcast<RenderBoxModelObject>(*renderer.Container())),
        _containingWritingMode(_container->Style().GetWritingMode()), _writingMode(style.GetWritingMode()),
        _selfAxis(selfAxis),
        _containingAxis(!_containingWritingMode.IsOrthogonal(_writingMode) ? selfAxis
                                                                           : OppositeAxis(selfAxis)),
        _physicalAxis(selfAxis == LogicalBoxAxis::Inline ? _writingMode.InlineAxis()
                                                         : _writingMode.BlockAxis()),
        _style(style), _alignment(_containingAxis == LogicalBoxAxis::Inline ? style.GetJustifySelf().Resolve()
                                                                            : style.GetAlignSelf().Resolve()),
        _defaultAnchorBox(NeedsAnchor() ? style::AnchorPositionEvaluator::DefaultAnchorForBox(renderer)
                                        : nullptr),
        _marginBefore(style::MarginEdge::Fixed {0}), _marginAfter(style::MarginEdge::Fixed {0}),
        _insetBefore(style::InsetEdge::Fixed {0}), _insetAfter(style::InsetEdge::Fixed {0})
  {
    krys_debug_assert(_container);

    // Compute basic containing block info.
    _originalContainingRange = renderer.ContainingBlockRangeForPositioned(*_container, _physicalAxis);
    _containingRange = _originalContainingRange;
    _containingInlineSize =
      (LogicalBoxAxis::Inline == _containingAxis)
        ? _containingRange.Size()
        : renderer.ContainingBlockRangeForPositioned(*_container, OppositeAxis(_physicalAxis)).Size();

    // Adjust for scrollable area.
    if (!_style.GetPositionArea().IsNone() && PositionType::Fixed != _style.GetPosition())
    {
      ExpandToScrollableArea(_containingRange);
    }

    // Adjust for grid-area.
    CaptureGridArea();

    // Capture the anchor geometry and adjust for position-area.
    CaptureAnchorGeometry();
  }

  void PositionedLayoutConstraints::ComputeInsets() noexcept
  {
    // Cache insets and margins, etc.
    CaptureInsets();

    if (_useStaticPosition)
    {
      ComputeStaticPosition();
    }

    if (ContainingCoordsAreFlipped())
    {
      // Ideally this check is incorporated into captureInsets() but currently it needs to happen after
      // computeStaticPosition() because containingCoordsAreFlipped() depends on m_useStaticPosition.
      std::swap(_marginBefore, _marginAfter);
      std::swap(_insetBefore, _insetAfter);
    }

    // Compute the inset-modified containing block.
    _insetModifiedContainingRange = _containingRange;
    _insetModifiedContainingRange.ShiftMinEdgeBy(InsetBeforeValue());
    _insetModifiedContainingRange.ShiftMaxEdgeBy(-InsetAfterValue());
  }

  LayoutUnit PositionedLayoutConstraints::MarginBeforeValue() const noexcept
  {
    return style::EvaluateMinimum<LayoutUnit>(_marginBefore, _containingInlineSize,
                                              _style.UsedZoomForLength());
  }

  LayoutUnit PositionedLayoutConstraints::MarginAfterValue() const noexcept
  {
    return style::EvaluateMinimum<LayoutUnit>(_marginAfter, _containingInlineSize,
                                              _style.UsedZoomForLength());
  }

  LayoutUnit PositionedLayoutConstraints::InsetBeforeValue() const noexcept
  {
    return style::EvaluateMinimum<LayoutUnit>(_insetBefore, ContainingSize(), _style.UsedZoomForLength());
  }

  LayoutUnit PositionedLayoutConstraints::InsetAfterValue() const noexcept
  {
    return style::EvaluateMinimum<LayoutUnit>(_insetAfter, ContainingSize(), _style.UsedZoomForLength());
  }

  bool PositionedLayoutConstraints::NeedsAnchor() const noexcept
  {
    return !_style.GetPositionArea().IsNone() || _alignment.Position() == ItemPosition::AnchorCenter;
  }

  bool PositionedLayoutConstraints::ContainingCoordsAreFlipped() const noexcept
  {
    bool orthogonalOpposing =
      (_containingAxis == LogicalBoxAxis::Inline && _writingMode.IsBlockFlipped())
      || (_containingAxis == LogicalBoxAxis::Block && _containingWritingMode.IsBlockFlipped());
    // FIXME: Static position has a confusing implementation. Leaving it alone for now.
    return !_useStaticPosition
           && ((IsBlockOpposing() && _containingAxis == LogicalBoxAxis::Block)
               || (IsOrthogonal() && orthogonalOpposing));
  }

  void PositionedLayoutConstraints::CaptureInsets() noexcept
  {
    bool isHorizontal = BoxAxis::Horizontal == _physicalAxis;

    if (isHorizontal)
    {
      _bordersPlusPadding =
        _renderer.BorderLeft() + _renderer.PaddingLeft() + _renderer.PaddingRight() + _renderer.BorderRight();
      _useStaticPosition = _style.Left().IsAuto() && _style.Right().IsAuto() && !_defaultAnchorBox;
    }
    else
    {
      _bordersPlusPadding =
        _renderer.BorderTop() + _renderer.PaddingTop() + _renderer.PaddingBottom() + _renderer.BorderBottom();
      _useStaticPosition = _style.Top().IsAuto() && _style.Bottom().IsAuto() && !_defaultAnchorBox;
    }

    if (LogicalBoxAxis::Inline == _selfAxis)
    {
      _marginBefore = isHorizontal ? _style.MarginLeft() : _style.MarginTop();
      _marginAfter = isHorizontal ? _style.MarginRight() : _style.MarginBottom();
      _insetBefore = _style.LogicalLeft();
      _insetAfter = _style.LogicalRight();
    }
    else
    {
      _marginBefore = _style.MarginBefore();
      _marginAfter = _style.MarginAfter();
      _insetBefore = _style.LogicalTop();
      _insetAfter = _style.LogicalBottom();
    }

    if (_defaultAnchorBox)
    {
      // If the box uses anchor-center or position-area and does have a default anchor box,
      // then it doesn't use static positioning.
      _useStaticPosition = false;
    }
  }

  // MARK: - Adjustments to the containing block.

  void PositionedLayoutConstraints::ExpandToScrollableArea(
    LayoutRange &containingRange, Maybe<ScrollPosition> fromScrollPosition) const noexcept
  {
    // FIXME: Extend this logic to other scrollable containing blocks.
    if (!Is<RenderView>(_container))
    {
      return;
    }

    const auto *initialContainingBlock = Downcast<RenderBox>(_container);
    for (auto *child = initialContainingBlock->FirstChildBox(); child; child = child->NextSiblingBox())
    {
      if (child->IsOutOfFlowPositioned())
      {
        continue;
      }
      LayoutUnit outerSize = BoxAxis::Vertical == _physicalAxis
                               ? child->Height() + std::max(0_lu, child->MarginTop() + child->MarginBottom())
                               : child->Width() + std::max(0_lu, child->MarginLeft() + child->MarginRight());
      if (StartIsBefore())
      {
        containingRange.FloorSizeFromMinEdge(outerSize);
      }
      else
      {
        containingRange.FloorSizeFromMaxEdge(outerSize);
      }
    }

    if (fromScrollPosition)
    {
      auto scrollOffset =
        BoxAxis::Horizontal == _physicalAxis ? fromScrollPosition->X() : fromScrollPosition->Y();
      containingRange.MoveBy(LayoutUnit(-scrollOffset));
    }
  }

  void PositionedLayoutConstraints::CaptureGridArea() noexcept
  {
    const auto *gridContainer = DynamicDowncast<RenderGrid>(_container);
    if (!gridContainer)
    {
      return;
    }

    if (LogicalBoxAxis::Inline == _containingAxis)
    {
      _containingRange =
        gridContainer->GridAreaRangeForOutOfFlow(_renderer, style::GridTrackSizingDirection::Columns);
      _containingInlineSize = _containingRange.Size();
    }
    else
    {
      _containingRange =
        gridContainer->GridAreaRangeForOutOfFlow(_renderer, style::GridTrackSizingDirection::Rows);
      _containingInlineSize =
        gridContainer->GridAreaRangeForOutOfFlow(_renderer, style::GridTrackSizingDirection::Columns).Size();
    }
  }

  LayoutRange PositionedLayoutConstraints::ExtractRange(LayoutRect anchorRect) noexcept
  {
    LayoutRange anchorRange;
    if (BoxAxis::Horizontal == _physicalAxis)
    {
      anchorRange.Set(anchorRect.X(), anchorRect.Width());
    }
    else
    {
      anchorRange.Set(anchorRect.Y(), anchorRect.Height());
    }

    if (_containingWritingMode.IsBlockFlipped() && LogicalBoxAxis::Block == _containingAxis)
    {
      // Coordinate fixup for flipped blocks.
      anchorRange.MoveTo(_containingRange.Max() - anchorRange.Max() + _container->BorderAfter());
    }
    return anchorRange;
  }

  void PositionedLayoutConstraints::CaptureAnchorGeometry() noexcept
  {
    if (!_defaultAnchorBox)
    {
      return;
    }

    // Store the anchor geometry.
    LayoutRect anchorRect = style::AnchorPositionEvaluator::ComputeAnchorRectRelativeToContainingBlock(
      *_defaultAnchorBox, *_container, _renderer);
    _anchorArea = ExtractRange(anchorRect);

    // Adjust containing block for position-area.
    if (_style.GetPositionArea().IsNone())
    {
      return;
    }
    _containingRange = AdjustForPositionArea(_containingRange, _anchorArea, _physicalAxis);

    // Margin basis is always against the inline axis.
    if (LogicalBoxAxis::Inline == _containingAxis)
    {
      _containingInlineSize = _containingRange.Size();
      return;
    }
    // Else we're representing the block axis, but need the inline dimensions.
    auto inlineAxis = OppositeAxis(_physicalAxis);
    LayoutRange inlineContainingBlock(_container->BorderLogicalLeft(), _containingInlineSize);
    auto inlineAnchorArea = BoxAxis::Horizontal == inlineAxis
                              ? LayoutRange {anchorRect.X(), anchorRect.Width()}
                              : LayoutRange {anchorRect.Y(), anchorRect.Height()};
    _containingInlineSize = AdjustForPositionArea(inlineContainingBlock, inlineAnchorArea, inlineAxis).Size();
  }

  LayoutRange PositionedLayoutConstraints::AdjustForPositionArea(LayoutRange rangeToAdjust,
                                                                 LayoutRange anchorArea,
                                                                 BoxAxis containerAxis) noexcept
  {
    krys_debug_assert(!_style.GetPositionArea().IsNone());
    krys_debug_assert(_defaultAnchorBox);
    krys_debug_assert(NeedsAnchor());
    krys_debug_assert(anchorArea.Size() >= 0);

    auto adjustedRange = rangeToAdjust;
    switch (_style.GetPositionArea().TryValue()->CoordMatchedTrackForAxis(
      containerAxis, _containingWritingMode, _writingMode))
    {
      case style::PositionAreaTrack::Start:
        adjustedRange.ShiftMaxEdgeTo(anchorArea.Min());
        adjustedRange.FloorSizeFromMaxEdge();
        return adjustedRange;
      case style::PositionAreaTrack::SpanStart:
        adjustedRange.ShiftMaxEdgeTo(anchorArea.Max());
        adjustedRange.CapMinEdgeTo(anchorArea.Min());
        return adjustedRange;
      case style::PositionAreaTrack::End:
        adjustedRange.ShiftMinEdgeTo(anchorArea.Max());
        adjustedRange.FloorSizeFromMinEdge();
        return adjustedRange;
      case style::PositionAreaTrack::SpanEnd:
        adjustedRange.ShiftMinEdgeTo(anchorArea.Min());
        adjustedRange.FloorMaxEdgeTo(anchorArea.Max());
        return adjustedRange;
      case style::PositionAreaTrack::Center: adjustedRange = anchorArea; return adjustedRange;
      case style::PositionAreaTrack::SpanAll:
        adjustedRange.CapMinEdgeTo(anchorArea.Min());
        adjustedRange.FloorMaxEdgeTo(anchorArea.Max());
        return adjustedRange;
    }
    krys_debug_assert(false);
    return adjustedRange;
  }

  // MARK: - Resolving margins and alignment (after sizing).

  Maybe<LayoutUnit>
    PositionedLayoutConstraints::RemainingSpaceForStaticAlignment(LayoutUnit itemSize) const noexcept
  {
    if (!_useStaticPosition || _containingAxis == LogicalBoxAxis::Inline)
    {
      return null;
    }

    if (auto *parent = DynamicDowncast<RenderGrid>(_renderer.Parent()))
    {
      const auto &itemStyle = _renderer.Style();

      auto itemResolvedAlignSelf = [&]
      {
        if (!itemStyle.GetAlignSelf().IsAuto())
        {
          return itemStyle.GetAlignSelf().Resolve(StyleSelfAlignmentData {ItemPosition::Start});
        }
        return parent->Style().GetAlignItems().Resolve(StyleSelfAlignmentData {ItemPosition::Start});
      }();

      switch (itemResolvedAlignSelf.Position())
      {
        case ItemPosition::Center:
        case ItemPosition::FlexEnd:
        case ItemPosition::SelfEnd:
        case ItemPosition::End:
        {
          if (_container == parent)
          {
            return null;
          }

          // FIXME: This is probably not the correct set of writing mode checks.

          const auto &containingBlockStyle = _container->Style();
          if (!containingBlockStyle.GetWritingMode().IsHorizontal())
          {
            return null;
          }

          if (!containingBlockStyle.GetWritingMode().IsBidiLTR())
          {
            return null;
          }

          const auto &parentStyle = parent->Style();
          if (!parentStyle.GetWritingMode().IsHorizontal())
          {
            return null;
          }

          if (!parentStyle.GetWritingMode().IsBidiLTR())
          {
            return null;
          }

          if (!itemStyle.GetWritingMode().IsHorizontal())
          {
            return null;
          }

          if (!itemStyle.GetWritingMode().IsBidiLTR())
          {
            return null;
          }

          if (itemResolvedAlignSelf.PositionType() != ItemPositionType::NonLegacy)
          {
            return null;
          }

          if (itemResolvedAlignSelf.Overflow() != OverflowAlignment::Default)
          {
            return null;
          }

          auto remainingSpace = parent->ContentBoxLogicalHeight() - itemSize;
#if KRYS_DEBUG
          _isEligibleForStaticRangeAlignment = remainingSpace >= 0_lu;
#endif
          if (remainingSpace >= 0_lu)
          {
            return remainingSpace;
          }
          return null;
        }
        default: return null;
      }
    }
    return null;
  }

  // See CSS2 § 10.3.7-8 and 10.6.4-5.
  void PositionedLayoutConstraints::ResolvePosition(
    RenderBox::LogicalExtentComputedValues &computedValues) const noexcept
  {
    // Static position should have resolved one of our insets by now.
    krys_debug_assert(!_useStaticPosition || !(_insetBefore.IsAuto() && _insetAfter.IsAuto()));

    auto usedMarginBefore = MarginBeforeValue();
    auto usedMarginAfter = MarginAfterValue();
    auto alignmentShift = 0_lu;

    auto outerSize = usedMarginBefore + computedValues.Extent + usedMarginAfter;
    auto remainingSpace = InsetModifiedContainingSize() - outerSize;

    bool honorAutoInsets = !_defaultAnchorBox || _alignment.IsNormal();
    bool hasAutoBeforeInset = _insetBefore.IsAuto() && honorAutoInsets;
    bool hasAutoAfterInset = _insetAfter.IsAuto() && honorAutoInsets;
    bool hasAutoBeforeMargin = _marginBefore.IsAuto() && !_defaultAnchorBox;
    bool hasAutoAfterMargin = _marginAfter.IsAuto() && !_defaultAnchorBox;

    if ((hasAutoBeforeMargin || hasAutoAfterMargin) && !hasAutoBeforeInset && !hasAutoAfterInset)
    {
      // Distribute remainingSpace to auto margins.
      if (hasAutoBeforeMargin && hasAutoAfterMargin)
      {
        // Distribute usable space to both margins equally.
        auto usableRemainingSpace =
          (LogicalBoxAxis::Inline == _containingAxis) ? std::max(0_lu, remainingSpace) : remainingSpace;
        usedMarginBefore = usedMarginAfter = usableRemainingSpace / 2;

        // Distribute unused space to the end side.
        auto unusedSpace = remainingSpace - (usedMarginBefore + usedMarginAfter);
        if (StartIsBefore())
        {
          usedMarginAfter += unusedSpace;
        }
        else
        {
          usedMarginBefore += unusedSpace;
        }
      }
      else if (hasAutoBeforeMargin)
      {
        usedMarginBefore = remainingSpace;
      }
      else if (hasAutoAfterMargin)
      {
        usedMarginAfter = remainingSpace;
      }
    }
    else
    {
      if (auto staticRemainingSpace = RemainingSpaceForStaticAlignment(outerSize))
      {
        alignmentShift = ResolveAlignmentShift(*staticRemainingSpace, outerSize);
      }
      else if (hasAutoBeforeInset != hasAutoAfterInset)
      {
        alignmentShift = hasAutoAfterInset ? 0_lu : remainingSpace;
      }
      else // Align into remaining space.
      {
        alignmentShift = ResolveAlignmentShift(remainingSpace, outerSize);
      }
    }

    auto position = _insetModifiedContainingRange.Min() + usedMarginBefore + alignmentShift;

    computedValues.Position = position;
    if (LogicalBoxAxis::Inline == _selfAxis)
    {
      if (_writingMode.IsLogicalLeftInlineStart() == !ContainingCoordsAreFlipped())
      {
        computedValues.Margins.Start = usedMarginBefore;
        computedValues.Margins.End = usedMarginAfter;
      }
      else
      {
        computedValues.Margins.Start = usedMarginAfter;
        computedValues.Margins.End = usedMarginBefore;
      }
    }
    else if (ContainingCoordsAreFlipped())
    {
      computedValues.Margins.Before = usedMarginAfter;
      computedValues.Margins.After = usedMarginBefore;
    }
    else
    {
      computedValues.Margins.Before = usedMarginBefore;
      computedValues.Margins.After = usedMarginAfter;
    }
  }

  LayoutUnit PositionedLayoutConstraints::ResolveAlignmentShift(LayoutUnit unusedSpace,
                                                                LayoutUnit itemSize) const noexcept
  {
    bool startIsBefore = this->StartIsBefore();
    bool isOverflowing = unusedSpace < 0_lu;
    if (isOverflowing && OverflowAlignment::Safe == _alignment.Overflow())
    {
      return startIsBefore ? 0_lu : unusedSpace;
    }

    ItemPosition resolvedAlignment = ResolveAlignmentValue();
    krys_debug_assert(ItemPosition::Auto != resolvedAlignment);

    LayoutUnit shift;
    if (ItemPosition::AnchorCenter == resolvedAlignment)
    {
      auto anchorCenterPosition = _anchorArea.Min() + (_anchorArea.Size() - itemSize) / 2;
      shift = anchorCenterPosition - _insetModifiedContainingRange.Min();
      if (_alignment.Overflow() == OverflowAlignment::Safe)
      {
        if (startIsBefore)
        {
          if (shift < 0)
          {
            shift = 0_lu;
          }
        }
        else
        {
          if (shift > unusedSpace)
          {
            shift = unusedSpace;
          }
        }
      }
      if (!isOverflowing && OverflowAlignment::Default == _alignment.Overflow())
      {
        // Avoid introducing overflow of the IMCB.
        if (shift < 0)
        {
          shift = 0_lu;
        }
        else if (shift > unusedSpace)
        {
          shift = unusedSpace;
        }
      }
    }
    else
    {
      auto alignmentSpace = StyleSelfAlignmentData::AdjustmentFromStartEdge(
        unusedSpace, resolvedAlignment, _containingAxis, _containingWritingMode, _writingMode);
      shift = startIsBefore ? alignmentSpace : unusedSpace - alignmentSpace;
    }

    if (isOverflowing && ItemPosition::Normal != resolvedAlignment
        && OverflowAlignment::Default == _alignment.Overflow())
    {
      // Allow overflow, but try to stay within the containing block.
      // See https://www.w3.org/TR/css-align-3/#auto-safety-position

      auto containingRange = _originalContainingRange;
      if (_defaultAnchorBox && PositionType::Fixed == _style.GetPosition())
      {
        // We didn't modify the _containingRange to include scrollable area for positioning, but we should
        // allow it for overflow management if we can scroll to reach that overflow.
        if (const auto *renderView = DynamicDowncast<RenderView>(_container))
        {
          auto &view = renderView->FrameView();
          auto scrollPosition = view.ScrollPositionRespectingCustomFixedPosition();
          ExpandToScrollableArea(containingRange, view.ConstrainedScrollPosition(ScrollPosition {
                                                    scrollPosition.X().ToInt(), scrollPosition.Y().ToInt()}));
        }
      }

      auto spaceAfter = std::max(0_lu, containingRange.Max() - _insetModifiedContainingRange.Max());
      auto spaceBefore = std::max(0_lu, _insetModifiedContainingRange.Min() - containingRange.Min());

      auto [allowsInfiniteOverflowBefore, allowsInfiniteOverflowAfter] = ContainerAllowsInfiniteOverflow();

      if (startIsBefore)
      {
        // Avoid overflow on the end side
        spaceAfter += (unusedSpace - shift);
        if (spaceAfter < 0 && !allowsInfiniteOverflowAfter)
        {
          shift += spaceAfter;
        }
        // Disallow overflow on the start side.
        spaceBefore += shift;
        if (spaceBefore < 0 && !allowsInfiniteOverflowBefore)
        {
          shift -= spaceBefore;
        }
      }
      else
      {
        // Avoid overflow on the end side
        spaceBefore += shift;
        if (spaceBefore < 0 && !allowsInfiniteOverflowBefore)
        {
          shift -= spaceBefore;
        }
        // Disallow overflow on the start side.
        spaceAfter += (unusedSpace - shift);
        if (spaceAfter < 0 && !allowsInfiniteOverflowAfter)
        {
          shift += spaceAfter;
        }
      }
    }
    return shift;
  }

  ItemPosition PositionedLayoutConstraints::ResolveAlignmentValue() const noexcept
  {
    if (_useStaticPosition)
    {
#if KRYS_DEBUG
      krys_debug_assert(_isEligibleForStaticRangeAlignment);
#endif
      const auto *parentStyle = _renderer.Parent() ? &_renderer.Parent()->Style() : nullptr;

      if (!parentStyle || !_style.GetAlignSelf().IsAuto())
      {
        return _style.GetAlignSelf().Resolve(StyleSelfAlignmentData {ItemPosition::Start}).Position();
      }
      return parentStyle->GetAlignItems().Resolve(StyleSelfAlignmentData {ItemPosition::Start}).Position();
    }

    auto alignmentPosition = [&]
    {
      auto itemPosition = _alignment.Position();
      return (ItemPosition::Auto == itemPosition) ? ItemPosition::Normal : itemPosition;
    }();

    if (auto positionAreaValue = _style.GetPositionArea().TryValue();
        positionAreaValue && ItemPosition::Normal == alignmentPosition)
    {
      alignmentPosition =
        positionAreaValue->DefaultAlignmentForAxis(_physicalAxis, _containingWritingMode, _writingMode);
    }

    if (!_defaultAnchorBox && alignmentPosition == ItemPosition::AnchorCenter)
    {
      return ItemPosition::Center;
    }
    return alignmentPosition;
  }

  Pair<bool, bool> PositionedLayoutConstraints::ContainerAllowsInfiniteOverflow() const noexcept
  {
    if (!(_container->HasPotentiallyScrollableOverflow()
          || (Is<RenderView>(_container) && PositionType::Fixed != _style.GetPosition())))
    {
      return {false, false};
    }
    const auto *containerBox = DynamicDowncast<RenderBox>(_container);
    krys_debug_assert(containerBox);
    if (!containerBox)
    {
      return {false, false};
    }

    auto overflowLimits = containerBox->AllowedLayoutOverflow(); // Already in flipped coordinates.
    return _physicalAxis == BoxAxis::Vertical
             ? Pair<bool, bool> {!overflowLimits.Top(), !overflowLimits.Bottom()}
             : Pair<bool, bool> {!overflowLimits.Left(), !overflowLimits.Right()};
  }

  bool PositionedLayoutConstraints::AlignmentAppliesStretch(ItemPosition normalAlignment) const noexcept
  {
    auto alignmentPosition = _alignment.Position();
    if (_style.GetPositionArea().IsNone()
        && (ItemPosition::Auto == alignmentPosition || ItemPosition::Normal == alignmentPosition))
    {
      alignmentPosition = normalAlignment;
    }
    return ItemPosition::Stretch == alignmentPosition;
  }

  bool PositionedLayoutConstraints::InsetFitsContent() const noexcept
  {
    return (_insetBefore.IsAuto() || _insetAfter.IsAuto())
           && !_defaultAnchorBox; // position-area and align-center zero out auto insets.
  }

  bool PositionedLayoutConstraints::NeedsGridAreaAdjustmentBeforeStaticPositioning() const noexcept
  {
    if (_containingAxis == LogicalBoxAxis::Block)
    {
      return true;
    }

    auto *parent = _renderer.Parent();
    // When the grid container is a parent we do not take the normal static positioning path.
    if (!_container->IsRenderGrid() || parent == _container)
    {
      return false;
    }

    auto parentWritingMode = parent->Style().GetWritingMode();
    if (parentWritingMode.IsLogicalLeftInlineStart() && !parentWritingMode.IsOrthogonal(_writingMode))
    {
      return false;
    }

    return true;
  }

  // MARK: - Static Position Computation

  void PositionedLayoutConstraints::ComputeStaticPosition() noexcept
  {
    krys_debug_assert(_useStaticPosition);

    if (Is<RenderGrid>(_container))
    {
      // Grid Containers have special behavior, see https://www.w3.org/TR/css-grid/#abspos
      if (_container == _renderer.Parent())
      {
        // Fake the static layout right here so it integrates with grid-area properly.
        _useStaticPosition = false; // Avoid the static position code path.
        _insetBefore = style::InsetEdge::Fixed {0};
        _insetAfter = style::InsetEdge::Fixed {0};

        if (ItemPosition::Auto == _alignment.Position())
        {
          if (LogicalBoxAxis::Inline == _containingAxis)
          {
            if (auto justifyItems = _container->Style().GetJustifyItems(); !justifyItems.IsLegacyNone())
            {
              _alignment = justifyItems.Resolve();
            }
          }
          else
          {
            _alignment = _container->Style().GetAlignItems().Resolve();
          }
        }
        if (ItemPosition::Auto == _alignment.Position() || ItemPosition::Normal == _alignment.Position())
        {
          _alignment.SetPosition(ItemPosition::Start);
        }
        if (OverflowAlignment::Default == _alignment.Overflow())
        {
          _alignment.SetOverflow(OverflowAlignment::Unsafe);
        }

        // Unclear if this is spec-compliant, but it is the current interop behavior.
        if (_marginBefore.IsAuto())
        {
          _marginBefore = style::MarginEdge::Fixed {0};
        }
        if (_marginAfter.IsAuto())
        {
          _marginAfter = style::MarginEdge::Fixed {0};
        }
        return;
      }
      // Rewind grid-area adjustments and fall through to the existing static position code.
      if (NeedsGridAreaAdjustmentBeforeStaticPositioning())
      {
        _containingRange.MoveTo(_originalContainingRange.Min());
      }
    }

    if (_selfAxis == LogicalBoxAxis::Inline)
    {
      ComputeInlineStaticDistance();
    }
    else
    {
      ComputeBlockStaticDistance();
    }
  }

  namespace
  {
    KRYS_NODISCARD LayoutPoint PositionInContainer(const RenderBox &container, const RenderBox &child,
                                                   LayoutPoint positionInChild) noexcept
    {
      auto containerWritingMode = container.Style().GetWritingMode();
      auto childWritingMode = child.Style().GetWritingMode();
      auto childInFlowOffset = childWritingMode.IsHorizontal()
                                 ? child.OffsetForInFlowPosition()
                                 : child.OffsetForInFlowPosition().TransposedSize();
      auto childLogicalLeft = childInFlowOffset.Width() + child.LogicalLeft();
      auto childLogicalTop = childInFlowOffset.Height() + child.LogicalTop();

      if (containerWritingMode.IsOrthogonal(childWritingMode))
      {
        auto topLeft =
          LayoutPoint {childLogicalLeft + positionInChild.X(), childLogicalTop + positionInChild.Y()};
        if (childWritingMode.IsBlockFlipped())
        {
          topLeft.SetY(childLogicalTop + child.LogicalHeight() - positionInChild.Y());
        }
        if (containerWritingMode.IsBlockFlipped())
        {
          topLeft.SetX(childLogicalLeft + child.LogicalWidth() - positionInChild.X());
        }
        return topLeft.TransposedPoint();
      }

      if (containerWritingMode.IsBlockOpposing(childWritingMode))
      {
        return {childLogicalLeft + positionInChild.X(),
                childLogicalTop + child.LogicalHeight() - positionInChild.Y()};
      }

      return {childLogicalLeft + positionInChild.X(), childLogicalTop + positionInChild.Y()};
    }

    KRYS_NODISCARD LayoutPoint StaticDistance(const RenderBoxModelObject &container,
                                              const RenderBox &outOfFlowBox) noexcept
    {
      // Static position is relative to the candidate box's parent (it is computed during normal in-flow
      // layout as if the candidate box was in-flow)
      // 1. traverse the ancestor chain and convert static position relative to each container all the way up
      // to the containing block
      // 2. adjust the final static position with the containing block's border
      // 3. pick x or y depending on what direction we are actually interested in (note that it's always the
      // block directon from the
      //    candidate box's point of view but it could very well be the inline distance from the containing
      //    block's point of view.)

      auto initialStaticPosition = [&]
      {
        // Static position is already in the coordinate system of the container (minus the flip in inline
        // direction).
        auto staticPosition = LayoutPoint {outOfFlowBox.Layer()->StaticInlinePosition(),
                                           outOfFlowBox.Layer()->StaticBlockPosition()};
        // We are relative to a RenderBox ancestor unless the containing block itself is an inline box.
        RenderElement *staticPositionContainingBlock = outOfFlowBox.Parent();
        for (; staticPositionContainingBlock && !Is<RenderBox>(staticPositionContainingBlock)
               && staticPositionContainingBlock != &container;
             staticPositionContainingBlock = staticPositionContainingBlock->Container())
        {
        }
        if (auto *renderBox = DynamicDowncast<RenderBox>(staticPositionContainingBlock);
            renderBox && renderBox->Style().GetWritingMode().IsInlineFlipped())
        {
          staticPosition.SetX(renderBox->LogicalWidth() - staticPosition.X());
        }
        return staticPosition;
      };

      auto staticPosition = LayoutPoint {};
      const RenderBox *child = &outOfFlowBox;
      auto hasSeenNonInlineBoxContainer = false;
      for (auto *ancestorContainer = child->Parent(); ancestorContainer && ancestorContainer != &container;
           ancestorContainer = ancestorContainer->Container())
      {
        auto *containerBox = DynamicDowncast<RenderBox>(*ancestorContainer);
        if (!containerBox || Is<RenderTableRow>(*containerBox))
        {
          continue;
        }

        staticPosition = child == &outOfFlowBox ? initialStaticPosition()
                                                : PositionInContainer(*containerBox, *child, staticPosition);
        child = containerBox;
        hasSeenNonInlineBoxContainer = true;
      }

      if (!hasSeenNonInlineBoxContainer && Is<RenderInline>(container))
      {
        // This is a simple case of when the containing block is formed by a positioned inline box with no
        // block boxes in-between (e.g <span style="position: relative">)
        return initialStaticPosition();
      }

      const auto *containingBlock = DynamicDowncast<RenderBox>(container);
      // m_insetBefore is expected to be relative to the padding box (while staticPosition is relative to the
      // border box).
      auto containingBlockBorderSize = LayoutSize {};
      if (containingBlock)
      {
        containingBlockBorderSize =
          containingBlock->Style().GetWritingMode().IsInlineFlipped()
            ? LayoutSize(containingBlock->BorderEnd(), containingBlock->BorderBefore())
            : LayoutSize(containingBlock->BorderStart(), containingBlock->BorderBefore());
      }
      else
      {
        containingBlock = child->ContainingBlock();
        krys_debug_assert(containingBlock);
        // Note that we don't take the border here as we passed the real containing block.
      }

      staticPosition = child == &outOfFlowBox ? initialStaticPosition()
                                              : PositionInContainer(*containingBlock, *child, staticPosition);
      staticPosition -= containingBlockBorderSize;
      return staticPosition;
    }
  }

  void PositionedLayoutConstraints::ComputeInlineStaticDistance() noexcept
  {
    // Note that at this point staticPosition is relative to the containing block (x is inline direction, y is
    // block direction) which may not match with the box's slef writing mode.
    auto parentWritingMode = _renderer.Parent()->Style().GetWritingMode();
    auto isParentOrthogonal = parentWritingMode.IsOrthogonal(SelfWritingMode());
    auto shouldUseInsetAfter =
      !isParentOrthogonal && parentWritingMode.IsInlineFlipped(); // This is what trunk has.
    if (shouldUseInsetAfter && _containingWritingMode.IsOrthogonal(parentWritingMode)
        && _containingWritingMode.IsBlockFlipped())
    {
      // FIXME: Figure out why.
      shouldUseInsetAfter = false;
    }

    auto staticPosition = StaticDistance(*_container, _renderer);
    auto staticDistance = !IsOrthogonal() ? staticPosition.X() : staticPosition.Y();
    if (auto *gridContainer = DynamicDowncast<RenderGrid>(_container))
    {
      // Special grid handling: move static distance back to relative to border box and adjust with our
      // offset.
      auto containingBlockBorderSize =
        !IsOrthogonal()
          ? (gridContainer->Style().GetWritingMode().IsInlineFlipped() ? gridContainer->BorderEnd()
                                                                       : gridContainer->BorderStart())
          : gridContainer->BorderBefore();
      staticDistance += containingBlockBorderSize;
      staticDistance -= _containingRange.Min();
    }

    // Since the static position is computed during in flow layout, the computed
    // position should already have zoom computed in. We need to divide out the zoom
    // so that we get the same position when evaluating the inset.
    auto usedZoom = _style.UsedZoomForLength().Value;
    if (shouldUseInsetAfter)
    {
      _insetAfter = style::InsetEdge::Fixed {(ContainingSize() - staticDistance).ToFloat() / usedZoom};
      return;
    }
    _insetBefore = style::InsetEdge::Fixed {staticDistance.ToFloat() / usedZoom};
  }

  void PositionedLayoutConstraints::ComputeBlockStaticDistance() noexcept
  {
    // Since the static position is computed during in flow layout, the computed
    // position should already have zoom computed in. We need to divide out the zoom
    // so that we get the same position when evaluating the inset.
    auto usedZoom = _style.UsedZoomForLength().Value;

    // Note that at this point staticPosition is relative to the containing block (x is inline direction, y is
    // block direction) which may not match with the box's slef writing mode.
    auto staticPosition = [&]
    {
      if (!IsOrthogonal())
      {
        return StaticDistance(*_container, _renderer).Y().ToFloat() / usedZoom;
      }
      return StaticDistance(*_container, _renderer).X().ToFloat() / usedZoom;
    };
    _insetBefore = style::InsetEdge::Fixed {staticPosition()};
  }

  namespace
  {
    KRYS_NODISCARD bool
      ShouldInlineStaticDistanceAdjustedWithBoxHeight(WritingMode containinigBlockWritingMode,
                                                      WritingMode parentWritingMode,
                                                      WritingMode outOfFlowBoxWritingMode) noexcept
    {
      if (!containinigBlockWritingMode.IsOrthogonal(parentWritingMode))
      {
        return false;
      }

      if (parentWritingMode.IsOrthogonal(outOfFlowBoxWritingMode))
      {
        return parentWritingMode.IsBlockFlipped();
      }

      return containinigBlockWritingMode.IsBlockFlipped() && !parentWritingMode.IsInlineFlipped();
    }
  }

  void PositionedLayoutConstraints::FixupLogicalLeftPosition(
    RenderBox::LogicalExtentComputedValues &computedValues) const noexcept
  {
    if (_useStaticPosition)
    {
      if (_container != _renderer.Parent()
          && ShouldInlineStaticDistanceAdjustedWithBoxHeight(
            _containingWritingMode, _renderer.Parent()->Style().GetWritingMode(), SelfWritingMode()))
      {
        computedValues.Position -= computedValues.Extent;
      }
      return;
    }

    if (_writingMode.IsHorizontal())
    {
      const auto *containingBox = DynamicDowncast<RenderBox>(&Container());
      if (containingBox && containingBox->Style().ShouldPlaceVerticalScrollbarOnLeft())
      {
        computedValues.Position += LayoutUnit(containingBox->VerticalScrollbarWidth());
      }
    }

    // FIXME: This hack is needed to calculate the logical left position for a 'rtl' relatively
    // positioned, inline because right now, it is using the logical left position
    // of the first line box when really it should use the last line box. When
    // this is fixed elsewhere, this adjustment should be removed.
    // FIXME: Move the box by the distance between the first and last inline boxes of an RTL inline
    // container (lineLeftmostInlineBoxFor).
  }

  namespace
  {
    // FIXME: Let's move this over to RenderBoxModelObject and collapse some of the logic here.
    KRYS_NODISCARD bool
      ShouldBlockStaticDistanceAdjustedWithBoxHeight(const RenderBoxModelObject &containingBlock,
                                                     const RenderElement &parent,
                                                     WritingMode outOfFlowBoxWritingMode) noexcept
    {
      // This is where we check if the final static position needs to be adjusted with the height of the
      // out-of-flow box. In ::computeBlockStaticDistance we convert the static position relative to the
      // containing block but in some cases this final static position still points to the wrong side of the
      // box (i.e. at computeBlockStaticDistance we don't know yet the height which may contribute to the
      // logical top position. see details below.)

      auto parentWritingMode = parent.Style().GetWritingMode();
      auto containinigBlockWritingMode = containingBlock.Style().GetWritingMode();

      if (containinigBlockWritingMode.BlockDirection() == parentWritingMode.BlockDirection()
          && parentWritingMode.BlockDirection() == outOfFlowBoxWritingMode.BlockDirection())
      {
        return false;
      }

      auto isParentInlineFlipped = parentWritingMode.IsInlineFlipped();
      if (containinigBlockWritingMode.IsOrthogonal(outOfFlowBoxWritingMode))
      {
        if (&containingBlock == &parent)
        {
          // <div id=containinigBlock class=rtl>
          //   <div id=outOfFlowBox class=ltr>
          return isParentInlineFlipped;
        }

        if (!containinigBlockWritingMode.IsOrthogonal(parentWritingMode))
        {
          return isParentInlineFlipped;
        }

        return parentWritingMode.IsBlockFlipped();
      }

      krys_debug_assert(containinigBlockWritingMode.BlockDirection()
                          == outOfFlowBoxWritingMode.BlockDirection()
                        || containinigBlockWritingMode.IsBlockOpposing(outOfFlowBoxWritingMode));
      if (!parentWritingMode.IsOrthogonal(containinigBlockWritingMode))
      {
        // inline direction does not matter as all participants are on the same axis.
        if (containinigBlockWritingMode.BlockDirection() == outOfFlowBoxWritingMode.BlockDirection())
        {
          // <div id=containinigBlock class=vrl>
          //   <div id=parent class=vlr>
          //     <div id=outOfFlowBox class=vrl>
          return true;
        }

        // <div id=containinigBlock class=vlr>
        //   <div id=parent class=vrl>
        //     <div id=outOfFlowBox class=vrl>
        return parentWritingMode.IsBlockOpposing(containinigBlockWritingMode);
      }

      // Orhogonal parent.
      // <div id=containinigBlock class=vrl>
      //   <div id=parent class=htb>
      //     <div id=outOfFlowBox class=vlr>
      return containinigBlockWritingMode.IsBlockFlipped() != isParentInlineFlipped;
    }
  }

  void PositionedLayoutConstraints::AdjustLogicalTopWithLogicalHeightIfNeeded(
    RenderBox::LogicalExtentComputedValues &computedValues) const noexcept
  {
    if (!_useStaticPosition || _selfAxis != LogicalBoxAxis::Block)
    {
      return;
    }
    if (ShouldBlockStaticDistanceAdjustedWithBoxHeight(*_container, *_renderer.Parent(), _writingMode))
    {
      computedValues.Position -= computedValues.Extent;
    }
  }
}
