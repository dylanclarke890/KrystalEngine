#pragma once

#include "Krystal.Booey/Core/BoxSides.hpp"
#include "Krystal.Booey/Core/ScrollTypes.hpp"
#include "Krystal.Booey/Graphics/LayoutRange.hpp"
#include "Krystal.Booey/Rendering/RenderBox.hpp"
#include "Krystal.Booey/Rendering/Style/StyleSelfAlignmentData.hpp"
#include "Krystal.Booey/Style/Values/Box/Margin.hpp"
#include "Krystal.Booey/Style/Values/Position/Inset.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/Pair.hpp"

namespace krys::boo
{
  /// @brief The constraints an out-of-flow positioned box is sized and positioned under, along one of its
  /// axes: its containing block's range, its insets and margins, and its alignment.
  class PositionedLayoutConstraints
  {
  private:
    const RenderBox &_renderer;
    const RenderBoxModelObject *_container {nullptr};
    const WritingMode _containingWritingMode;
    const WritingMode _writingMode;
    const LogicalBoxAxis _selfAxis;
    const LogicalBoxAxis _containingAxis;
    const BoxAxis _physicalAxis;
    const RenderStyle &_style;
    StyleSelfAlignmentData _alignment;
    const RenderBoxModelObject *const _defaultAnchorBox {nullptr}; // Only set if needed.

    LayoutRange _anchorArea; // Only valid if defaultAnchor exists.
    LayoutRange _containingRange;
    LayoutRange _originalContainingRange;
    LayoutRange _insetModifiedContainingRange;
    LayoutUnit _containingInlineSize;

    LayoutUnit _bordersPlusPadding;
    style::MarginEdge _marginBefore;
    style::MarginEdge _marginAfter;
    style::InsetEdge _insetBefore;
    style::InsetEdge _insetAfter;
    bool _useStaticPosition {false};
#if KRYS_DEBUG
    mutable bool _isEligibleForStaticRangeAlignment {false};
#endif

  public:
    PositionedLayoutConstraints(const RenderBox &renderer, LogicalBoxAxis selfAxis) noexcept;
    PositionedLayoutConstraints(const RenderBox &renderer, const RenderStyle &selfStyleOverride,
                                LogicalBoxAxis selfAxis) noexcept;
    void ComputeInsets() noexcept;

    /*** The following are available without calling computeInsets(). ***/

    KRYS_NODISCARD const RenderBoxModelObject &Container() const noexcept
    {
      return *_container;
    }
    KRYS_NODISCARD LayoutUnit ContainingSize() const noexcept
    {
      return _containingRange.Size();
    }
    KRYS_NODISCARD LayoutUnit ContainingInlineSize() const noexcept
    {
      return _containingInlineSize;
    }
    KRYS_NODISCARD LayoutRange ContainingRange() const noexcept
    {
      return _containingRange;
    }
    KRYS_NODISCARD LayoutRange OriginalContainingRange() const noexcept
    {
      return _originalContainingRange;
    }
    KRYS_NODISCARD LayoutRange ExtractRange(LayoutRect anchorRect) noexcept;

    KRYS_NODISCARD BoxAxis PhysicalAxis() const noexcept
    {
      return _physicalAxis;
    }
    KRYS_NODISCARD LogicalBoxAxis ContainingAxis() const noexcept
    {
      return _containingAxis;
    }
    KRYS_NODISCARD WritingMode ContainingWritingMode() const noexcept
    {
      return _containingWritingMode;
    }
    KRYS_NODISCARD WritingMode SelfWritingMode() const noexcept
    {
      return _writingMode;
    }

    KRYS_NODISCARD bool NeedsAnchor() const noexcept;
    KRYS_NODISCARD const RenderBoxModelObject *DefaultAnchorBox() const noexcept
    {
      return _defaultAnchorBox;
    }
    KRYS_NODISCARD const StyleSelfAlignmentData &Alignment() const noexcept
    {
      return _alignment;
    }
    KRYS_NODISCARD ItemPosition ResolveAlignmentValue() const noexcept; // Convert auto/normal as necessary.
    KRYS_NODISCARD bool AlignmentAppliesStretch(ItemPosition normalAlignment) const noexcept;

    KRYS_NODISCARD bool IsOrthogonal() const noexcept
    {
      return _containingWritingMode.IsOrthogonal(_writingMode);
    }
    KRYS_NODISCARD bool IsOpposing() const noexcept
    {
      return _containingAxis == LogicalBoxAxis::Inline ? _containingWritingMode.IsInlineOpposing(_writingMode)
                                                       : _containingWritingMode.IsBlockOpposing(_writingMode);
    }
    KRYS_NODISCARD bool IsBlockOpposing() const noexcept
    {
      return _containingWritingMode.IsBlockOpposing(_writingMode);
    }
    KRYS_NODISCARD bool IsBlockFlipped() const noexcept
    {
      return _containingWritingMode.IsBlockFlipped();
    }
    KRYS_NODISCARD bool StartIsBefore() const noexcept
    {
      return _containingAxis == LogicalBoxAxis::Block || _containingWritingMode.IsLogicalLeftInlineStart();
    }

    /*** For everything else, you MUST call computeInsets() after the constructor. ***/

    // Before = logical top or left wrt containing block (i.e. the lower-coordinate side).
    // After = logical bottom or right wrt containing block (i.e. the higher-coordinate side).
    KRYS_NODISCARD LayoutUnit BordersPlusPadding() const noexcept
    {
      return _bordersPlusPadding;
    }
    KRYS_NODISCARD const style::MarginEdge &MarginBefore() const noexcept
    {
      return _marginBefore;
    }
    KRYS_NODISCARD const style::MarginEdge &MarginAfter() const noexcept
    {
      return _marginAfter;
    }
    KRYS_NODISCARD const style::InsetEdge &InsetBefore() const noexcept
    {
      return _insetBefore;
    }
    KRYS_NODISCARD const style::InsetEdge &InsetAfter() const noexcept
    {
      return _insetAfter;
    }
    KRYS_NODISCARD LayoutUnit MarginBeforeValue() const noexcept;
    KRYS_NODISCARD LayoutUnit MarginAfterValue() const noexcept;
    KRYS_NODISCARD LayoutUnit InsetBeforeValue() const noexcept;
    KRYS_NODISCARD LayoutUnit InsetAfterValue() const noexcept;
    KRYS_NODISCARD bool InsetFitsContent() const noexcept; // One or both insets are auto for sizing purposes.

    KRYS_NODISCARD LayoutUnit InsetModifiedContainingSize() const noexcept
    {
      return _insetModifiedContainingRange.Size();
    }
    KRYS_NODISCARD LayoutRange InsetModifiedContainingRange() const noexcept
    {
      return _insetModifiedContainingRange;
    }
    KRYS_NODISCARD LayoutUnit AvailableContentSpace() const noexcept // This may be negative.
    {
      return InsetModifiedContainingSize() - MarginBeforeValue() - BordersPlusPadding() - MarginAfterValue();
    }

    void ResolvePosition(RenderBox::LogicalExtentComputedValues &computedValues) const noexcept;
    KRYS_NODISCARD LayoutUnit ResolveAlignmentShift(LayoutUnit unusedSpace,
                                                    LayoutUnit itemSize) const noexcept;

    void FixupLogicalLeftPosition(RenderBox::LogicalExtentComputedValues &computedValues) const noexcept;
    void AdjustLogicalTopWithLogicalHeightIfNeeded(
      RenderBox::LogicalExtentComputedValues &computedValues) const noexcept;

  private:
    KRYS_NODISCARD bool ContainingCoordsAreFlipped() const noexcept;

    void CaptureInsets() noexcept;
    void CaptureGridArea() noexcept;
    void CaptureAnchorGeometry() noexcept;
    void ExpandToScrollableArea(LayoutRange &containingRange,
                                Maybe<ScrollPosition> fromScrollPosition = null) const noexcept;
    KRYS_NODISCARD LayoutRange AdjustForPositionArea(LayoutRange rangeToAdjust, LayoutRange anchorArea,
                                                     BoxAxis containerAxis) noexcept;
    KRYS_NODISCARD Pair<bool, bool> ContainerAllowsInfiniteOverflow() const noexcept;

    KRYS_NODISCARD bool NeedsGridAreaAdjustmentBeforeStaticPositioning() const noexcept;
    KRYS_NODISCARD Maybe<LayoutUnit> RemainingSpaceForStaticAlignment(LayoutUnit itemSize) const noexcept;
    void ComputeStaticPosition() noexcept;
    void ComputeInlineStaticDistance() noexcept;
    void ComputeBlockStaticDistance() noexcept;
  };
}
