#pragma once

#include "Krystal.Booey/Core/BoxExtents.hpp"
#include "Krystal.Booey/Core/BoxSides.hpp"
#include "Krystal.Booey/Graphics/LayoutRange.hpp"
#include "Krystal.Booey/Graphics/LayoutRect.hpp"
#include "Krystal.Booey/Rendering/RenderBoxModelObject.hpp"
#include "Krystal.Booey/Rendering/RenderOverflow.hpp"
#include "Krystal.Booey/Style/Values/Box/MarginTrim.hpp"
#include "Krystal.Booey/Style/Values/Flexbox/FlexBasis.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/Pair.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo
{
  class FloatingObject;
  class PositionedLayoutConstraints;
  class ShapeOutsideInfo;
  class RenderBlock;

  enum class AvailableLogicalHeightType : bool
  {
    ExcludeMarginBorderPadding,
    IncludeMarginBorderPadding
  };

  enum class StretchingMode : uint8
  {
    Any,
    Explicit
  };

  /// @brief A renderer that generates a box: its frame rect (relative to its container), margins, overflow
  /// and the computation of its sizes.
  // FIXME: Painting, hit testing, the repaint rects, scrolling (and autoscrolling), the coordinate mapping,
  // the shape outside, fragmentation, and the sizing of out-of-flow positioned boxes
  // (PositionedLayoutConstraints) come with painting, scrolling and the later layout slices.
  class RenderBox : public RenderBoxModelObject
  {
    KRYS_NON_COPY_MOVABLE(RenderBox);
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderBox);

  private:
    // The width/height of the contents + borders + padding. The x/y location is relative to our container
    // (which is not always our parent).
    LayoutRect _frameRect;

  protected:
    LayoutBoxExtent _marginBox;

    // The preferred logical width of the element if it were to break its lines at every possible opportunity.
    LayoutUnit _minPreferredLogicalWidth;

    // The preferred logical width of the element if it never breaks any lines at all.
    LayoutUnit _maxPreferredLogicalWidth;

    // Our overflow information.
    UniquePtr<RenderOverflow> _overflow;

  public:
    ~RenderBox() noexcept override;

#pragma region Frame rect

    KRYS_NODISCARD LayoutUnit X() const noexcept
    {
      return _frameRect.X();
    }
    KRYS_NODISCARD LayoutUnit Y() const noexcept
    {
      return _frameRect.Y();
    }
    KRYS_NODISCARD LayoutUnit Width() const noexcept
    {
      return _frameRect.Width();
    }
    KRYS_NODISCARD LayoutUnit Height() const noexcept
    {
      return _frameRect.Height();
    }

    // These represent your location relative to your container as a physical offset.
    // In layout related methods you almost always want the logical location (e.g. x() and y()).
    KRYS_NODISCARD LayoutUnit Top() const noexcept
    {
      return TopLeftLocation().Y();
    }
    KRYS_NODISCARD LayoutUnit Left() const noexcept
    {
      return TopLeftLocation().X();
    }

    void SetX(LayoutUnit x) noexcept
    {
      _frameRect.SetX(x);
    }
    void SetY(LayoutUnit y) noexcept
    {
      _frameRect.SetY(y);
    }
    void SetWidth(LayoutUnit width) noexcept
    {
      _frameRect.SetWidth(width);
    }
    void SetHeight(LayoutUnit height) noexcept
    {
      _frameRect.SetHeight(height);
    }

    KRYS_NODISCARD LayoutUnit LogicalLeft() const noexcept;
    KRYS_NODISCARD LayoutUnit LogicalRight() const noexcept;
    KRYS_NODISCARD LayoutUnit LogicalTop() const noexcept;
    KRYS_NODISCARD LayoutUnit LogicalBottom() const noexcept;
    KRYS_NODISCARD LayoutUnit LogicalWidth() const noexcept;
    KRYS_NODISCARD LayoutUnit LogicalHeight() const noexcept;

    void SetLogicalLeft(LayoutUnit left) noexcept;
    void SetLogicalTop(LayoutUnit top) noexcept;
    void SetLogicalLocation(LayoutPoint location) noexcept;
    void SetLogicalWidth(LayoutUnit size) noexcept;
    void SetLogicalHeight(LayoutUnit size) noexcept;
    void SetLogicalSize(LayoutSize size) noexcept;

    KRYS_NODISCARD LayoutPoint Location() const noexcept
    {
      return _frameRect.Location();
    }
    KRYS_NODISCARD LayoutSize LocationOffset() const noexcept
    {
      return {X(), Y()};
    }
    KRYS_NODISCARD LayoutSize Size() const noexcept
    {
      return _frameRect.Size();
    }
    KRYS_NODISCARD LayoutSize LogicalSize() const noexcept;

    void SetLocation(const LayoutPoint &location) noexcept
    {
      _frameRect.SetLocation(location);
    }
    void SetSize(const LayoutSize &size) noexcept
    {
      _frameRect.SetSize(size);
    }
    void Move(LayoutUnit dx, LayoutUnit dy) noexcept
    {
      _frameRect.Move(dx, dy);
    }

    KRYS_NODISCARD LayoutRect FrameRect() const noexcept
    {
      return _frameRect;
    }
    void SetFrameRect(const LayoutRect &rect) noexcept
    {
      _frameRect = rect;
    }

    KRYS_NODISCARD LayoutRect MarginBoxRect() const noexcept;
    KRYS_NODISCARD LayoutRect BorderBoxRect() const noexcept
    {
      return LayoutRect(LayoutPoint(), Size());
    }
    KRYS_NODISCARD LayoutRect BorderBoundingBox() const noexcept final
    {
      return BorderBoxRect();
    }
    KRYS_NODISCARD LayoutSize BorderBoxLogicalSize() const noexcept
    {
      return LogicalSize();
    }

    // The content area of the box (excludes padding - and intrinsic padding for table cells, etc... - and
    // border).
    KRYS_NODISCARD LayoutRect ContentBoxRect() const noexcept;
    KRYS_NODISCARD LayoutPoint ContentBoxLocation() const noexcept;

    // This returns the content area of the box (excluding padding and border). The only difference with
    // contentBoxRect is that computedCSSContentBoxRect does include the intrinsic padding in the content box
    // as this is what some callers expect (like getComputedStyle).
    KRYS_NODISCARD LayoutRect ComputedCSSContentBoxRect() const noexcept;

    // These represent your location relative to your container as a physical offset.
    // In layout related methods you almost always want the logical location (e.g. x() and y()).
    KRYS_NODISCARD LayoutPoint TopLeftLocation() const noexcept;
    KRYS_NODISCARD LayoutSize TopLeftLocationOffset() const noexcept;

#pragma endregion

#pragma region Tree

    // Note these functions are not equivalent of childrenOfType<RenderBox>
    KRYS_NODISCARD RenderBox *ParentBox() const noexcept;
    KRYS_NODISCARD RenderBox *FirstChildBox() const noexcept;
    KRYS_NODISCARD RenderBox *FirstInFlowChildBox() const noexcept;
    KRYS_NODISCARD RenderBox *LastChildBox() const noexcept;
    KRYS_NODISCARD RenderBox *LastInFlowChildBox() const noexcept;
    KRYS_NODISCARD RenderBox *PreviousSiblingBox() const noexcept;
    KRYS_NODISCARD RenderBox *PreviousInFlowSiblingBox() const noexcept;
    KRYS_NODISCARD RenderBox *NextSiblingBox() const noexcept;
    KRYS_NODISCARD RenderBox *NextInFlowSiblingBox() const noexcept;

    KRYS_NODISCARD bool IsGridItem() const noexcept
    {
      return Parent() && Parent()->IsRenderGrid() && !IsExcludedFromNormalLayout();
    }
    KRYS_NODISCARD bool IsFlexItem() const noexcept
    {
      return Parent() && Parent()->IsRenderFlexibleBox() && !IsExcludedFromNormalLayout();
    }
    KRYS_NODISCARD bool IsColumnSpanner() const noexcept;

#pragma endregion

#pragma region Overflow

    // Visual and layout overflow are in the coordinate space of the box.  This means that they aren't purely
    // physical directions. For horizontal-tb and vertical-lr they will match physical directions, but for
    // horizontal-bt and vertical-rl, the top/bottom and left/right respectively are flipped when compared to
    // their physical counterparts.  For example minX is on the left in vertical-lr, but it is on the right in
    // vertical-rl.
    KRYS_NODISCARD LayoutRect FlippedClientBoxRect() const noexcept;
    KRYS_NODISCARD LayoutRect LayoutOverflowRect() const noexcept
    {
      return _overflow ? _overflow->LayoutOverflowRect() : FlippedClientBoxRect();
    }
    KRYS_NODISCARD LayoutUnit LogicalLeftLayoutOverflow() const noexcept;
    KRYS_NODISCARD LayoutUnit LogicalRightLayoutOverflow() const noexcept;

    KRYS_NODISCARD LayoutRect VisualOverflowRect() const noexcept
    {
      return _overflow ? _overflow->VisualOverflowRect() : BorderBoxRect();
    }
    KRYS_NODISCARD LayoutUnit LogicalLeftVisualOverflow() const noexcept;
    KRYS_NODISCARD LayoutUnit LogicalRightVisualOverflow() const noexcept;

    // RenderBox's basic implementation accounts for the writing mode (only).
    KRYS_NODISCARD virtual LayoutOptionalOutsets AllowedLayoutOverflow() const noexcept;
    void AddLayoutOverflow(const LayoutRect &rect) noexcept;
    void AddVisualOverflow(const LayoutRect &rect) noexcept;
    void ClearOverflow() noexcept;
    RenderOverflow &EnsureOverflow() noexcept;

    void AddVisualEffectOverflow() noexcept;
    KRYS_NODISCARD LayoutRect ApplyVisualEffectOverflow(const LayoutRect &borderBox) const noexcept;

    enum class ComputeOverflowOptions : uint8
    {
      RecomputeFloats,
    };
    virtual void AddOverflowFromInFlowChildren(EnumSet<ComputeOverflowOptions> options = {}) noexcept;
    void AddOverflowFromContainedBox(const RenderBox &child,
                                     EnumSet<ComputeOverflowOptions> options = {}) noexcept;

    KRYS_NODISCARD LayoutRect
      LogicalVisualOverflowRectForPropagation(WritingMode parentWritingMode) const noexcept;
    KRYS_NODISCARD LayoutRect VisualOverflowRectForPropagation(WritingMode parentWritingMode) const noexcept;
    KRYS_NODISCARD LayoutRect
      LogicalLayoutOverflowRectForPropagation(WritingMode parentWritingMode) const noexcept;
    KRYS_NODISCARD LayoutRect LayoutOverflowRectForPropagation(WritingMode parentWritingMode) const noexcept;

    KRYS_NODISCARD bool HasRenderOverflow() const noexcept
    {
      return static_cast<bool>(_overflow);
    }
    KRYS_NODISCARD bool HasVisualOverflow() const noexcept
    {
      return _overflow && !BorderBoxRect().Contains(_overflow->VisualOverflowRect());
    }
    KRYS_NODISCARD bool HasHorizontalLayoutOverflow() const noexcept;
    KRYS_NODISCARD bool HasVerticalLayoutOverflow() const noexcept;

#pragma endregion

#pragma region Content, padding and client boxes

    KRYS_NODISCARD LayoutSize ContentBoxSize() const noexcept;
    KRYS_NODISCARD LayoutUnit ContentBoxWidth() const noexcept;
    KRYS_NODISCARD LayoutUnit ContentBoxHeight() const noexcept;
    KRYS_NODISCARD LayoutSize ContentBoxLogicalSize() const noexcept;
    KRYS_NODISCARD LayoutUnit ContentBoxLogicalWidth() const noexcept;
    KRYS_NODISCARD LayoutUnit ContentBoxLogicalHeight() const noexcept;
    KRYS_NODISCARD LayoutUnit ContentBoxLogicalWidth(LayoutUnit overridingBorderBoxWidth) const noexcept;
    KRYS_NODISCARD LayoutUnit ContentBoxLogicalHeight(LayoutUnit overridingBorderBoxHeight) const noexcept;

    KRYS_NODISCARD LayoutUnit PaddingBoxWidth() const noexcept;
    KRYS_NODISCARD LayoutUnit PaddingBoxHeight() const noexcept;
    KRYS_NODISCARD LayoutRect PaddingBoxRect() const noexcept;
    KRYS_NODISCARD LayoutRect PaddingBoxRectIncludingScrollbar() const noexcept;

    // IE extensions. Used to calculate offsetWidth/Height.  Overridden by inlines (RenderFlow)
    // to return the remaining width on a given line (and the height of a single line).
    KRYS_NODISCARD LayoutUnit OffsetWidth() const noexcept override
    {
      return Width();
    }
    KRYS_NODISCARD LayoutUnit OffsetHeight() const noexcept override
    {
      return Height();
    }
    KRYS_NODISCARD LayoutUnit OffsetLeft() const noexcept override;
    KRYS_NODISCARD LayoutUnit OffsetTop() const noexcept override;

    // More IE extensions.  clientWidth and clientHeight represent the interior of an object
    // excluding border and scrollbar.  clientLeft/Top are just the borderLeftWidth and borderTopWidth.
    KRYS_NODISCARD LayoutUnit ClientLeft() const noexcept;
    KRYS_NODISCARD LayoutUnit ClientTop() const noexcept;
    KRYS_NODISCARD LayoutUnit ClientWidth() const noexcept;
    KRYS_NODISCARD LayoutUnit ClientHeight() const noexcept;
    KRYS_NODISCARD LayoutUnit ClientLogicalWidth() const noexcept;
    KRYS_NODISCARD LayoutUnit ClientLogicalHeight() const noexcept;
    KRYS_NODISCARD LayoutUnit ClientLogicalBottom() const noexcept;
    KRYS_NODISCARD LayoutRect ClientBoxRect() const noexcept;

    // scrollWidth/scrollHeight will be the same as clientWidth/clientHeight unless the
    // object has overflow:hidden/scroll/auto specified and also has overflow.
    // FIXME: scrollLeft/Top, the scroll position and setting it, once there's scrolling.
    KRYS_NODISCARD virtual int32 ScrollWidth() const noexcept;
    KRYS_NODISCARD virtual int32 ScrollHeight() const noexcept;

#pragma endregion

#pragma region Margins

    KRYS_NODISCARD const LayoutBoxExtent &MarginBox() const noexcept
    {
      return _marginBox;
    }
    KRYS_NODISCARD LayoutUnit MarginTop() const noexcept override
    {
      return _marginBox.Top();
    }
    KRYS_NODISCARD LayoutUnit MarginBottom() const noexcept override
    {
      return _marginBox.Bottom();
    }
    KRYS_NODISCARD LayoutUnit MarginLeft() const noexcept override
    {
      return _marginBox.Left();
    }
    KRYS_NODISCARD LayoutUnit MarginRight() const noexcept override
    {
      return _marginBox.Right();
    }
    void SetMarginTop(LayoutUnit margin) noexcept
    {
      _marginBox.Top() = margin;
    }
    void SetMarginBottom(LayoutUnit margin) noexcept
    {
      _marginBox.Bottom() = margin;
    }
    void SetMarginLeft(LayoutUnit margin) noexcept
    {
      _marginBox.Left() = margin;
    }
    void SetMarginRight(LayoutUnit margin) noexcept
    {
      _marginBox.Right() = margin;
    }

    KRYS_NODISCARD LayoutUnit MarginLogicalLeft(WritingMode writingMode) const noexcept
    {
      return _marginBox.LogicalLeft(writingMode);
    }
    KRYS_NODISCARD LayoutUnit MarginLogicalRight(WritingMode writingMode) const noexcept
    {
      return _marginBox.LogicalRight(writingMode);
    }
    KRYS_NODISCARD LayoutUnit MarginLogicalLeft() const noexcept;
    KRYS_NODISCARD LayoutUnit MarginLogicalRight() const noexcept;

    using RenderBoxModelObject::MarginAfter;
    using RenderBoxModelObject::MarginBefore;
    using RenderBoxModelObject::MarginEnd;
    using RenderBoxModelObject::MarginStart;
    KRYS_NODISCARD LayoutUnit MarginBefore(WritingMode writingMode) const noexcept override
    {
      return _marginBox.Before(writingMode);
    }
    KRYS_NODISCARD LayoutUnit MarginAfter(WritingMode writingMode) const noexcept override
    {
      return _marginBox.After(writingMode);
    }
    KRYS_NODISCARD LayoutUnit MarginStart(WritingMode writingMode) const noexcept override
    {
      return _marginBox.Start(writingMode);
    }
    KRYS_NODISCARD LayoutUnit MarginEnd(WritingMode writingMode) const noexcept override
    {
      return _marginBox.End(writingMode);
    }

    KRYS_NODISCARD LayoutUnit MarginBoxLogicalHeight(WritingMode writingMode) const noexcept;

    void SetMarginBefore(LayoutUnit value, WritingMode writingMode) noexcept
    {
      _marginBox.Before(writingMode) = value;
    }
    void SetMarginAfter(LayoutUnit value, WritingMode writingMode) noexcept
    {
      _marginBox.After(writingMode) = value;
    }
    void SetMarginStart(LayoutUnit value, WritingMode writingMode) noexcept
    {
      _marginBox.Start(writingMode) = value;
    }
    void SetMarginEnd(LayoutUnit value, WritingMode writingMode) noexcept
    {
      _marginBox.End(writingMode) = value;
    }
    void SetMarginBefore(LayoutUnit value) noexcept;
    void SetMarginAfter(LayoutUnit value) noexcept;
    void SetMarginStart(LayoutUnit value) noexcept;
    void SetMarginEnd(LayoutUnit value) noexcept;

    KRYS_NODISCARD virtual bool IsSelfCollapsingBlock() const noexcept
    {
      return false;
    }
    KRYS_NODISCARD virtual LayoutUnit CollapsedMarginBefore() const noexcept
    {
      return MarginBefore();
    }
    KRYS_NODISCARD virtual LayoutUnit CollapsedMarginAfter() const noexcept
    {
      return MarginAfter();
    }

    KRYS_NODISCARD LayoutUnit
      ConstrainBlockMarginInAvailableSpaceOrTrim(const RenderBox &containingBlock, LayoutUnit availableSpace,
                                                 style::MarginTrimSide marginSide) const noexcept;

    void MarkMarginAsTrimmed(style::MarginTrimSide newTrimmedMargin) noexcept;
    void ClearTrimmedMarginsMarkings() noexcept;
    KRYS_NODISCARD bool HasTrimmedMargin(Maybe<style::MarginTrimSide> marginTrimType) const noexcept;

    struct ComputedMarginValues
    {
      LayoutUnit Before;
      LayoutUnit After;
      LayoutUnit Start;
      LayoutUnit End;
    };
    struct LogicalExtentComputedValues
    {
      LayoutUnit Extent;
      LayoutUnit Position;
      ComputedMarginValues Margins;
    };
    // Resolve auto margins in the inline direction of the containing block so that objects can be pushed to
    // the start, middle or end of the containing block.
    void ComputeInlineDirectionMargins(const RenderBlock &containingBlock, LayoutUnit containerWidth,
                                       Maybe<LayoutUnit> availableSpaceAdjustedWithFloats,
                                       LayoutUnit childWidth, LayoutUnit &marginStart,
                                       LayoutUnit &marginEnd) const noexcept;

    // Used to resolve margins in the containing block's block-flow direction.
    void ComputeBlockDirectionMargins(const RenderBlock &containingBlock, LayoutUnit &marginBefore,
                                      LayoutUnit &marginAfter) const noexcept;
    void ComputeAndSetBlockDirectionMargins(const RenderBlock &containingBlock) noexcept;

#pragma endregion

#pragma region Sizing

    void Layout() noexcept override;

    KRYS_NODISCARD virtual LayoutUnit MinPreferredLogicalWidth() const noexcept;
    KRYS_NODISCARD virtual LayoutUnit MaxPreferredLogicalWidth() const noexcept;
    virtual void ComputeIntrinsicLogicalWidths(LayoutUnit &minLogicalWidth,
                                               LayoutUnit &maxLogicalWidth) const noexcept = 0;

    KRYS_NODISCARD Maybe<LayoutUnit> OverridingBorderBoxLogicalWidth() const noexcept;
    KRYS_NODISCARD Maybe<LayoutUnit> OverridingBorderBoxLogicalHeight() const noexcept;
    void SetOverridingBorderBoxLogicalHeight(LayoutUnit height) noexcept;
    void SetOverridingBorderBoxLogicalWidth(LayoutUnit width) noexcept;
    void ClearOverridingBorderBoxLogicalHeight() noexcept;
    void ClearOverridingBorderBoxLogicalWidth() noexcept;
    void ClearOverridingSize() noexcept;

    // Grid item's containing block is not the grid container, but the grid area, for which we don't have a
    // renderer.
    using GridAreaSize = Maybe<LayoutUnit>;
    KRYS_NODISCARD Maybe<GridAreaSize> GridAreaContentWidth(WritingMode writingMode) const noexcept;
    KRYS_NODISCARD Maybe<GridAreaSize> GridAreaContentHeight(WritingMode writingMode) const noexcept;
    KRYS_NODISCARD Maybe<GridAreaSize> GridAreaContentLogicalWidth() const noexcept;
    KRYS_NODISCARD Maybe<GridAreaSize> GridAreaContentLogicalHeight() const noexcept;
    void SetGridAreaContentLogicalWidth(GridAreaSize logicalWidth) noexcept;
    void SetGridAreaContentLogicalHeight(GridAreaSize logicalHeight) noexcept;
    void ClearGridAreaContentSize() noexcept;
    void ClearGridAreaContentLogicalHeight() noexcept;

    // These are currently only used by Flexbox code. In some cases we must layout flex items with a different
    // main size (the size in the main direction) than the one specified by the item in order to compute the
    // value of flex basis, i.e., the initial main size of the flex item before the free space is distributed.
    KRYS_NODISCARD Maybe<style::PreferredSize>
      OverridingLogicalHeightForFlexBasisComputation() const noexcept;
    KRYS_NODISCARD Maybe<style::PreferredSize> OverridingLogicalWidthForFlexBasisComputation() const noexcept;
    void SetOverridingBorderBoxLogicalHeightForFlexBasisComputation(
      const style::PreferredSize &logicalHeight) noexcept;
    void SetOverridingBorderBoxLogicalWidthForFlexBasisComputation(
      const style::PreferredSize &logicalWidth) noexcept;
    void ClearOverridingLogicalHeightForFlexBasisComputation() noexcept;
    void ClearOverridingLogicalWidthForFlexBasisComputation() noexcept;

    KRYS_NODISCARD LayoutUnit AdjustBorderBoxLogicalWidthForBoxSizing(
      const style::Length<css::NonNegativeUnzoomed> &logicalWidth) const noexcept;
    KRYS_NODISCARD LayoutUnit
      AdjustBorderBoxLogicalWidthForBoxSizing(LayoutUnit computedLogicalWidth) const noexcept;
    KRYS_NODISCARD LayoutUnit AdjustContentBoxLogicalWidthForBoxSizing(
      const style::Length<css::NonNegativeUnzoomed> &logicalWidth) const noexcept;
    KRYS_NODISCARD LayoutUnit
      AdjustContentBoxLogicalWidthForBoxSizing(LayoutUnit computedLogicalWidth) const noexcept;
    // Overridden by fieldsets to subtract out the intrinsic border.
    KRYS_NODISCARD virtual LayoutUnit
      AdjustBorderBoxLogicalHeightForBoxSizing(LayoutUnit height) const noexcept;
    KRYS_NODISCARD virtual LayoutUnit
      AdjustContentBoxLogicalHeightForBoxSizing(Maybe<LayoutUnit> height) const noexcept;
    KRYS_NODISCARD virtual LayoutUnit
      AdjustIntrinsicLogicalHeightForBoxSizing(LayoutUnit height) const noexcept;

    enum class AllowIntrinsic : bool
    {
      No,
      Yes
    };
    KRYS_NODISCARD LayoutUnit ConstrainLogicalWidthByMinMax(
      LayoutUnit logicalWidth, LayoutUnit availableWidth, const RenderBlock &containingBlock,
      AllowIntrinsic allowIntrinsic = AllowIntrinsic::Yes) const noexcept;
    KRYS_NODISCARD LayoutUnit ConstrainLogicalHeightByMinMax(
      LayoutUnit logicalHeight, Maybe<LayoutUnit> intrinsicContentHeight) const noexcept;
    KRYS_NODISCARD LayoutUnit ConstrainContentBoxLogicalHeightByMinMax(
      LayoutUnit logicalHeight, Maybe<LayoutUnit> intrinsicContentHeight) const noexcept;

    KRYS_NODISCARD LayoutUnit ContainingBlockLogicalWidthForContent() const noexcept override;
    KRYS_NODISCARD LayoutUnit
      ContainingBlockLogicalHeightForContent(AvailableLogicalHeightType heightType) const noexcept;
    KRYS_NODISCARD LayoutUnit ContainingBlockAvailableLineWidth() const noexcept;
    KRYS_NODISCARD LayoutUnit PerpendicularContainingBlockLogicalHeight() const noexcept;

    virtual void UpdateLogicalWidth() noexcept;
    void UpdateLogicalHeight() noexcept;
    void CacheIntrinsicContentLogicalHeightForFlexItem(LayoutUnit height) const noexcept;
    KRYS_NODISCARD virtual LogicalExtentComputedValues
      ComputeLogicalHeight(LayoutUnit logicalHeight, LayoutUnit logicalTop) const noexcept;
    void ComputeLogicalWidth(LogicalExtentComputedValues &computedValues) const noexcept;

    void OverrideLogicalHeightForSizeContainment() noexcept;

    // This function will compute the logical border-box height, without laying
    // out the box. This means that the result is only "correct" when the height
    // is explicitly specified. This function exists so that intrinsic width
    // calculations have a way to deal with children that have orthogonal writing modes.
    // When there is no explicit height, this function assumes a content height of
    // zero (and returns just border + padding).
    KRYS_NODISCARD LayoutUnit ComputeLogicalHeightWithoutLayout() const noexcept;

    KRYS_NODISCARD bool StretchesToViewport() const noexcept;

    KRYS_NODISCARD virtual LayoutSize IntrinsicSize() const noexcept
    {
      return {};
    }
    KRYS_NODISCARD LayoutUnit IntrinsicLogicalWidth() const noexcept;
    KRYS_NODISCARD LayoutUnit IntrinsicLogicalHeight() const noexcept;

    // Whether or not the element shrinks to its intrinsic width (rather than filling the width
    // of a containing block).  HTML4 buttons, <select>s, <input>s, legends, and floating/compact elements do
    // this.
    KRYS_NODISCARD bool SizesPreferredLogicalWidthToFitContent() const noexcept;

    KRYS_NODISCARD bool HasStretchedLogicalHeight() const noexcept;
    KRYS_NODISCARD bool
      HasStretchedLogicalWidth(StretchingMode stretchingMode = StretchingMode::Any) const noexcept;
    KRYS_NODISCARD bool IsStretchingColumnFlexItem() const noexcept;
    KRYS_NODISCARD bool ColumnFlexItemHasStretchAlignment() const noexcept;

    KRYS_NODISCARD LayoutUnit
      ShrinkLogicalWidthToAvoidFloats(LayoutUnit childMarginStart, LayoutUnit childMarginEnd,
                                      const RenderBlock &containingBlock) const noexcept;

    KRYS_NODISCARD LayoutUnit ComputeLogicalWidthUsing(const style::PreferredSize &logicalWidth,
                                                       LayoutUnit availableLogicalWidth,
                                                       const RenderBlock &containingBlock) const noexcept;
    KRYS_NODISCARD LayoutUnit ComputeLogicalWidthUsing(const style::MinimumSize &logicalWidth,
                                                       LayoutUnit availableLogicalWidth,
                                                       const RenderBlock &containingBlock) const noexcept;
    KRYS_NODISCARD LayoutUnit ComputeLogicalWidthUsing(const style::MaximumSize &logicalWidth,
                                                       LayoutUnit availableLogicalWidth,
                                                       const RenderBlock &containingBlock) const noexcept;
    KRYS_NODISCARD LayoutUnit ComputeLogicalWidthUsing(const style::FlexBasis &logicalWidth,
                                                       LayoutUnit availableLogicalWidth,
                                                       const RenderBlock &containingBlock) const noexcept;

    KRYS_NODISCARD Maybe<LayoutUnit>
      ComputeLogicalHeightUsing(const style::PreferredSize &logicalHeight,
                                Maybe<LayoutUnit> intrinsicContentHeight) const noexcept;
    KRYS_NODISCARD Maybe<LayoutUnit>
      ComputeLogicalHeightUsing(const style::MinimumSize &logicalHeight,
                                Maybe<LayoutUnit> intrinsicContentHeight) const noexcept;
    KRYS_NODISCARD Maybe<LayoutUnit>
      ComputeLogicalHeightUsing(const style::MaximumSize &logicalHeight,
                                Maybe<LayoutUnit> intrinsicContentHeight) const noexcept;

    KRYS_NODISCARD Maybe<LayoutUnit>
      ComputeContentLogicalHeight(const style::PreferredSize &logicalHeight,
                                  Maybe<LayoutUnit> intrinsicContentHeight) const noexcept;
    KRYS_NODISCARD Maybe<LayoutUnit>
      ComputeContentLogicalHeight(const style::MinimumSize &logicalHeight,
                                  Maybe<LayoutUnit> intrinsicContentHeight) const noexcept;
    KRYS_NODISCARD Maybe<LayoutUnit>
      ComputeContentLogicalHeight(const style::MaximumSize &logicalHeight,
                                  Maybe<LayoutUnit> intrinsicContentHeight) const noexcept;
    KRYS_NODISCARD Maybe<LayoutUnit>
      ComputeContentLogicalHeight(const style::FlexBasis &logicalHeight,
                                  Maybe<LayoutUnit> intrinsicContentHeight) const noexcept;

    enum class UpdatePercentageHeightDescendants : bool
    {
      No,
      Yes
    };
    KRYS_NODISCARD Maybe<LayoutUnit>
      ComputePercentageLogicalHeight(const style::PreferredSize &logicalHeight,
                                     UpdatePercentageHeightDescendants updateDescendants =
                                       UpdatePercentageHeightDescendants::Yes) const noexcept;
    KRYS_NODISCARD Maybe<LayoutUnit>
      ComputePercentageLogicalHeight(const style::MinimumSize &logicalHeight,
                                     UpdatePercentageHeightDescendants updateDescendants =
                                       UpdatePercentageHeightDescendants::Yes) const noexcept;
    KRYS_NODISCARD Maybe<LayoutUnit>
      ComputePercentageLogicalHeight(const style::MaximumSize &logicalHeight,
                                     UpdatePercentageHeightDescendants updateDescendants =
                                       UpdatePercentageHeightDescendants::Yes) const noexcept;
    KRYS_NODISCARD Maybe<LayoutUnit>
      ComputePercentageLogicalHeight(const style::FlexBasis &logicalHeight,
                                     UpdatePercentageHeightDescendants updateDescendants =
                                       UpdatePercentageHeightDescendants::Yes) const noexcept;
    KRYS_NODISCARD bool HasAutoHeightOrContainingBlockWithAutoHeight(
      UpdatePercentageHeightDescendants updatePercentageDescendants =
        UpdatePercentageHeightDescendants::Yes) const noexcept;

    KRYS_NODISCARD virtual LayoutUnit
      AvailableLogicalHeight(AvailableLogicalHeightType heightType) const noexcept;
    KRYS_NODISCARD LayoutUnit AvailableLogicalHeightUsing(
      const style::PreferredSize &logicalHeight, AvailableLogicalHeightType heightType) const noexcept;

    KRYS_NODISCARD bool PercentageLogicalHeightIsResolvable() const noexcept;

    KRYS_NODISCARD bool ShrinkToAvoidFloats() const noexcept;
    KRYS_NODISCARD bool AvoidsFloats() const noexcept;

    KRYS_NODISCARD virtual Maybe<LayoutUnit> FirstLineBaseline() const noexcept
    {
      return null;
    }
    KRYS_NODISCARD virtual Maybe<LayoutUnit> LastLineBaseline() const noexcept
    {
      return null;
    }

    KRYS_NODISCARD LayoutUnit LineHeight() const noexcept;

    KRYS_NODISCARD virtual bool ShouldInvalidatePreferredWidths() const noexcept;

    KRYS_NODISCARD virtual bool HasRelativeDimensions() const noexcept;
    KRYS_NODISCARD virtual bool HasRelativeLogicalHeight() const noexcept;
    KRYS_NODISCARD virtual bool HasRelativeLogicalWidth() const noexcept;

    KRYS_NODISCARD bool ShouldComputeLogicalHeightFromAspectRatio() const noexcept;

    KRYS_NODISCARD bool ShouldIgnoreLogicalMinMaxWidthSizes() const noexcept;
    KRYS_NODISCARD bool ShouldIgnoreLogicalMinMaxHeightSizes() const noexcept;

    // The explicit intrinsic inner size of contain-intrinsic-size
    KRYS_NODISCARD Maybe<LayoutUnit> ExplicitIntrinsicInnerWidth() const noexcept;
    KRYS_NODISCARD Maybe<LayoutUnit> ExplicitIntrinsicInnerHeight() const noexcept;
    KRYS_NODISCARD Maybe<LayoutUnit> ExplicitIntrinsicInnerLogicalWidth() const noexcept;
    KRYS_NODISCARD Maybe<LayoutUnit> ExplicitIntrinsicInnerLogicalHeight() const noexcept;

    KRYS_NODISCARD bool ComputeHasTransformRelatedProperty(const RenderStyle &styleToUse) const noexcept;

    KRYS_NODISCARD LayoutUnit ComputeIntrinsicLogicalWidthUsing(const style::PreferredSize &logicalWidth,
                                                                LayoutUnit availableLogicalWidth,
                                                                LayoutUnit borderAndPadding) const noexcept;
    KRYS_NODISCARD LayoutUnit ComputeIntrinsicLogicalWidthUsing(const style::MinimumSize &logicalWidth,
                                                                LayoutUnit availableLogicalWidth,
                                                                LayoutUnit borderAndPadding) const noexcept;
    KRYS_NODISCARD LayoutUnit ComputeIntrinsicLogicalWidthUsing(const style::MaximumSize &logicalWidth,
                                                                LayoutUnit availableLogicalWidth,
                                                                LayoutUnit borderAndPadding) const noexcept;
    KRYS_NODISCARD LayoutUnit ComputeIntrinsicLogicalWidthUsing(const style::FlexBasis &logicalWidth,
                                                                LayoutUnit availableLogicalWidth,
                                                                LayoutUnit borderAndPadding) const noexcept;

    KRYS_NODISCARD bool IncludeVerticalScrollbarSize() const noexcept;
    KRYS_NODISCARD bool IncludeHorizontalScrollbarSize() const noexcept;
    KRYS_NODISCARD bool CanUseOverlayScrollbars() const noexcept;
    KRYS_NODISCARD bool HasAutoScrollbar(ScrollbarOrientation orientation) const noexcept;
    KRYS_NODISCARD bool HasAlwaysPresentScrollbar(ScrollbarOrientation orientation) const noexcept;
    KRYS_NODISCARD bool CanBeProgramaticallyScrolled() const noexcept;
    KRYS_NODISCARD bool CanBeScrolledAndHasScrollableArea() const noexcept;
    KRYS_NODISCARD bool RequiresLayerWithScrollableArea() const noexcept;
    KRYS_NODISCARD int32 ScrollLeft() const noexcept;
    KRYS_NODISCARD int32 ScrollTop() const noexcept;
    KRYS_NODISCARD virtual int32 VerticalScrollbarWidth() const noexcept;
    KRYS_NODISCARD virtual int32 HorizontalScrollbarHeight() const noexcept;
    KRYS_NODISCARD int32 IntrinsicScrollbarLogicalWidthIncludingGutter() const noexcept;
    KRYS_NODISCARD int32 ScrollbarLogicalWidth() const noexcept;
    KRYS_NODISCARD int32 ScrollbarLogicalHeight() const noexcept;

    KRYS_NODISCARD bool ScrollsOverflow() const noexcept;
    KRYS_NODISCARD bool ScrollsOverflowX() const noexcept;
    KRYS_NODISCARD bool ScrollsOverflowY() const noexcept;
    KRYS_NODISCARD bool HasHorizontalOverflow() const noexcept;
    KRYS_NODISCARD bool HasVerticalOverflow() const noexcept;
    KRYS_NODISCARD bool HasScrollableOverflowX() const noexcept;
    KRYS_NODISCARD bool HasScrollableOverflowY() const noexcept;
    KRYS_NODISCARD bool IsScrollContainerX() const noexcept;
    KRYS_NODISCARD bool IsScrollContainerY() const noexcept;

    KRYS_NODISCARD virtual bool HasIntrinsicAspectRatio() const noexcept;

#pragma endregion

#pragma region Writing mode flipping

    KRYS_NODISCARD LayoutPoint FlipForWritingModeForChild(const RenderBox &child,
                                                          const LayoutPoint &point) const noexcept;
    // The offset is in the block direction (y for horizontal writing modes, x for vertical writing modes).
    KRYS_NODISCARD LayoutUnit FlipForWritingMode(LayoutUnit position) const noexcept;
    KRYS_NODISCARD LayoutPoint FlipForWritingMode(const LayoutPoint &position) const noexcept;
    KRYS_NODISCARD LayoutSize FlipForWritingMode(const LayoutSize &offset) const noexcept;
    void FlipForWritingMode(LayoutRect &rect) const noexcept;
    void FlipForWritingMode(FloatRect &rect) const noexcept;

#pragma endregion

    // Called when an out-of-flow box moves but doesn't necessarily change size. A simplified layout is
    // attempted that just updates the object's position. If the size does change, the object remains dirty.
    bool TryLayoutDoingOutOfFlowMovementOnly() noexcept
    {
      LayoutUnit oldWidth = Width();
      UpdateLogicalWidth();
      // If we shrink to fit our width may have changed, so we still need full layout.
      if (oldWidth != Width())
      {
        return false;
      }
      UpdateLogicalHeight();
      return true;
    }

    /// @brief Collapses `enclosingClippingBoxForStickyPosition().first`.
    // FIXME: Sticky positioning, once there's scrolling.
    KRYS_NODISCARD const RenderBox &EnclosingClippingBoxForStickyPosition() const noexcept;
    KRYS_NODISCARD LayoutRange ContainingBlockRangeForPositioned(const RenderBoxModelObject &container,
                                                                 BoxAxis physicalAxis) const noexcept;

  protected:
    RenderBox(Type type, dom::Element &element, RenderStyle &&style, EnumSet<TypeFlag> baseTypeFlags = {},
              TypeSpecificFlags typeSpecificFlags = {}) noexcept;
    RenderBox(Type type, dom::Document &document, RenderStyle &&style, EnumSet<TypeFlag> baseTypeFlags = {},
              TypeSpecificFlags typeSpecificFlags = {}) noexcept;

    void StyleWillChange(style::Difference diff, const RenderStyle &newStyle) noexcept override;
    void StyleDidChange(style::Difference diff, const RenderStyle *oldStyle) noexcept override;
    void UpdateFromStyle() noexcept override;

  public:
    KRYS_NODISCARD bool RequiresLayer() const noexcept override;

    void ApplyTransform(TransformationMatrix &transform, const RenderStyle &style,
                        const FloatRect &boundingBox,
                        EnumSet<style::TransformResolverOption> options) const noexcept override;
    using RenderLayerModelObject::ApplyTransform;

    KRYS_NODISCARD FloatRect ReferenceBoxRect(CSSBoxType boxType) const noexcept override;

    KRYS_NODISCARD virtual LayoutRect
      OverflowClipRect(const LayoutPoint &location,
                       OverlayScrollbarSizeRelevancy relevancy =
                         OverlayScrollbarSizeRelevancy::IgnoreOverlayScrollbarSize) const noexcept;
    KRYS_NODISCARD virtual LayoutRect
      OverflowClipRectForChildLayers(const LayoutPoint &location,
                                     OverlayScrollbarSizeRelevancy relevancy) const noexcept
    {
      return OverflowClipRect(location, relevancy);
    }
    KRYS_NODISCARD LayoutRect ClipRect(const LayoutPoint &location) const noexcept;

  public:
    void RemoveFloatingOrOutOfFlowChildFromBlockLists() noexcept;
    void RemoveFloatingAndInvalidateForLayout() noexcept;

    void MarkShapeOutsideDependentsForLayout() noexcept
    {
      if (IsFloating())
      {
        RemoveFloatingOrOutOfFlowChildFromBlockLists();
      }
    }

    /// @brief The box's `shape-outside`, if it's a float with one.
    KRYS_NODISCARD ShapeOutsideInfo *GetShapeOutsideInfo() const noexcept;

    void AddOverflowFromFloatBox(const FloatingObject &floatBox) noexcept;

  protected:
    void WillBeDestroyed() noexcept override;

    void UpdateShapeOutsideInfoAfterStyleChange(const RenderStyle &style,
                                                const RenderStyle *oldStyle) noexcept;
    void UpdateGridPositionAfterStyleChange(const RenderStyle &style, const RenderStyle *oldStyle) noexcept;
    ShapeOutsideInfo &EnsureShapeOutsideInfo() noexcept;
    void RemoveShapeOutsideInfo() noexcept;

    KRYS_NODISCARD bool ShouldTrimChildMargin(style::MarginTrimSide type,
                                              const RenderBox &child) const noexcept;
    KRYS_NODISCARD virtual bool IsChildEligibleForMarginTrim(style::MarginTrimSide,
                                                             const RenderBox &) const noexcept
    {
      return false;
    }

    KRYS_NODISCARD virtual bool ShouldResetLogicalHeightBeforeLayout() const noexcept;
    void ResetLogicalHeightBeforeLayoutIfNeeded() noexcept;

    KRYS_NODISCARD Maybe<LayoutUnit>
      ComputeIntrinsicLogicalContentHeightUsing(const style::PreferredSize &logicalHeight,
                                                Maybe<LayoutUnit> intrinsicContentHeight,
                                                LayoutUnit borderAndPadding) const noexcept;
    KRYS_NODISCARD Maybe<LayoutUnit>
      ComputeIntrinsicLogicalContentHeightUsing(const style::MinimumSize &logicalHeight,
                                                Maybe<LayoutUnit> intrinsicContentHeight,
                                                LayoutUnit borderAndPadding) const noexcept;
    KRYS_NODISCARD Maybe<LayoutUnit>
      ComputeIntrinsicLogicalContentHeightUsing(const style::MaximumSize &logicalHeight,
                                                Maybe<LayoutUnit> intrinsicContentHeight,
                                                LayoutUnit borderAndPadding) const noexcept;
    KRYS_NODISCARD Maybe<LayoutUnit>
      ComputeIntrinsicLogicalContentHeightUsing(const style::FlexBasis &logicalHeight,
                                                Maybe<LayoutUnit> intrinsicContentHeight,
                                                LayoutUnit borderAndPadding) const noexcept;

    KRYS_NODISCARD bool
      SkipContainingBlockForPercentHeightCalculation(const RenderBox &containingBlock,
                                                     bool isPerpendicularWritingMode) const noexcept;

    KRYS_NODISCARD Maybe<double> ResolveAspectRatio() const noexcept;
    KRYS_NODISCARD bool ShouldIgnoreAspectRatio() const noexcept;
    KRYS_NODISCARD bool IsRenderReplacedWithIntrinsicRatio() const noexcept;
    KRYS_NODISCARD bool ShouldComputeLogicalWidthFromAspectRatio() const noexcept;
    KRYS_NODISCARD LayoutUnit ComputeLogicalWidthFromAspectRatioInternal() const noexcept;
    KRYS_NODISCARD LayoutUnit ComputeLogicalWidthFromAspectRatio() const noexcept;
    KRYS_NODISCARD Pair<LayoutUnit, LayoutUnit> ComputeMinMaxLogicalWidthFromAspectRatio() const noexcept;
    KRYS_NODISCARD Pair<LayoutUnit, LayoutUnit> ComputeMinMaxLogicalHeightFromAspectRatio() const noexcept;
    enum class ConstrainDimension : uint8
    {
      Width,
      Height
    };
    enum class MinimumSizeIsAutomaticContentBased : bool
    {
      No,
      Yes
    };
    void ConstrainLogicalMinMaxSizesByAspectRatio(LayoutUnit &computedMinSize, LayoutUnit &computedMaxSize,
                                                  LayoutUnit computedSize,
                                                  MinimumSizeIsAutomaticContentBased minimumSizeType,
                                                  ConstrainDimension dimension) const noexcept;

    KRYS_NODISCARD static LayoutUnit
      BlockSizeFromAspectRatio(LayoutUnit borderPaddingInlineSum, LayoutUnit borderPaddingBlockSum,
                               double aspectRatioValue, BoxSizing boxSizing, LayoutUnit inlineSize,
                               const style::AspectRatio &aspectRatio, bool isRenderReplaced) noexcept;

    void ComputePreferredLogicalWidths(const style::MinimumSize &minLogicalWidth,
                                       const style::MaximumSize &maxLogicalWidth,
                                       LayoutUnit borderAndPaddingLogicalWidth) noexcept;

    KRYS_NODISCARD bool IsAspectRatioDegenerate(double aspectRatio) const noexcept
    {
      return !aspectRatio || std::isnan(aspectRatio);
    }

    KRYS_NODISCARD bool OverflowChangesMayAffectLayout() const noexcept final;

    // This function calculates the minimum and maximum preferred widths for an object.
    // These values are used in shrink-to-fit layout systems.
    // These include tables, positioned objects, floats and flexible boxes.
    virtual void ComputePreferredLogicalWidths() noexcept;
    KRYS_NODISCARD bool ShouldComputePreferredLogicalWidthsFromStyle() const noexcept;

    KRYS_NODISCARD virtual ItemPosition
      SelfAlignmentNormalBehavior(const RenderBox * = nullptr) const noexcept
    {
      return ItemPosition::Stretch;
    }

    virtual void ComputeIntrinsicKeywordLogicalWidths(LayoutUnit &minLogicalWidth,
                                                      LayoutUnit &maxLogicalWidth) const noexcept
    {
      ComputeIntrinsicLogicalWidths(minLogicalWidth, maxLogicalWidth);
    }

    KRYS_NODISCARD LayoutPoint TopLeftLocationWithFlipping() const noexcept;

    void ComputeOutOfFlowPositionedLogicalWidth(LogicalExtentComputedValues &computedValues) const noexcept;

  private:
    void AddOverflowWithRendererOffset(const RenderBox &renderer, LayoutSize offsetFromThis,
                                       EnumSet<ComputeOverflowOptions> options = {}) noexcept;

    template <typename Function>
    KRYS_NODISCARD LayoutUnit ComputeOrTrimInlineMargin(const RenderBlock &containingBlock,
                                                        style::MarginTrimSide marginSide,
                                                        const Function &computeInlineMargin) const noexcept;

    KRYS_NODISCARD LayoutUnit FillAvailableMeasure(LayoutUnit availableLogicalWidth) const noexcept;
    KRYS_NODISCARD LayoutUnit FillAvailableMeasure(LayoutUnit availableLogicalWidth, LayoutUnit &marginStart,
                                                   LayoutUnit &marginEnd) const noexcept;

    void ComputeOutOfFlowPositionedLogicalHeight(LogicalExtentComputedValues &computedValues) const noexcept;
    template <typename SizeType>
    KRYS_NODISCARD LayoutUnit ComputeOutOfFlowPositionedLogicalWidthUsing(
      const SizeType &logicalWidth, const PositionedLayoutConstraints &inlineConstraints) const noexcept;
    KRYS_NODISCARD LayoutUnit ComputeOutOfFlowPositionedLogicalHeightUsing(
      const style::PreferredSize &logicalHeight, LayoutUnit computedHeight,
      const PositionedLayoutConstraints &blockConstraints) const noexcept;
    KRYS_NODISCARD LayoutUnit ComputeOutOfFlowPositionedLogicalHeightUsing(
      const style::MinimumSize &logicalHeight, LayoutUnit computedHeight,
      const PositionedLayoutConstraints &blockConstraints) const noexcept;
    KRYS_NODISCARD LayoutUnit ComputeOutOfFlowPositionedLogicalHeightUsing(
      const style::MaximumSize &logicalHeight, LayoutUnit computedHeight,
      const PositionedLayoutConstraints &blockConstraints) const noexcept;

    enum class IntrinsicKeyword : uint8
    {
      WebkitFillAvailable,
      MinIntrinsic,
      MaxContent,
      MinContent,
      FitContent
    };
    void ComputeIntrinsicKeywordLogicalWidths(IntrinsicKeyword keyword, LayoutUnit borderAndPadding,
                                              LayoutUnit &minLogicalWidth,
                                              LayoutUnit &maxLogicalWidth) const noexcept;
    KRYS_NODISCARD LayoutUnit ComputeIntrinsicLogicalWidthUsing(IntrinsicKeyword keyword,
                                                                LayoutUnit availableLogicalWidth,
                                                                LayoutUnit borderAndPadding) const noexcept;

    template <typename SizeType>
    KRYS_NODISCARD LayoutUnit
      ComputeLogicalWidthUsingGeneric(const SizeType &logicalWidth, LayoutUnit availableLogicalWidth,
                                      const RenderBlock &containingBlock) const noexcept;
    template <typename SizeType>
    KRYS_NODISCARD LayoutUnit
      ComputeIntrinsicLogicalWidthUsingGeneric(const SizeType &logicalWidth, LayoutUnit availableLogicalWidth,
                                               LayoutUnit borderAndPadding) const noexcept;

    template <typename SizeType>
    KRYS_NODISCARD Maybe<LayoutUnit>
      ComputeLogicalHeightUsingGeneric(const SizeType &logicalHeight,
                                       Maybe<LayoutUnit> intrinsicContentHeight) const noexcept;
    template <typename SizeType>
    KRYS_NODISCARD Maybe<LayoutUnit>
      ComputeIntrinsicLogicalContentHeightUsingGeneric(const SizeType &logicalHeight,
                                                       Maybe<LayoutUnit> intrinsicContentHeight,
                                                       LayoutUnit borderAndPadding) const noexcept;
    template <typename SizeType>
    KRYS_NODISCARD Maybe<LayoutUnit>
      ComputePercentageLogicalHeightGeneric(const SizeType &logicalHeight,
                                            UpdatePercentageHeightDescendants updateDescendants =
                                              UpdatePercentageHeightDescendants::Yes) const noexcept;
    template <typename SizeType>
    KRYS_NODISCARD Maybe<LayoutUnit>
      ComputeContentLogicalHeightGeneric(const SizeType &logicalHeight,
                                         Maybe<LayoutUnit> intrinsicContentHeight) const noexcept;
    template <typename SizeType>
    KRYS_NODISCARD Maybe<LayoutUnit>
      ComputeContentAndScrollbarLogicalHeightUsing(const SizeType &logicalHeight,
                                                   Maybe<LayoutUnit> intrinsicContentHeight) const noexcept;

    void InvalidateAncestorBackgroundObscurationStatus() noexcept;
  };

  KRYS_NODISCARD inline LayoutUnit ResolveHeightForRatio(LayoutUnit borderAndPaddingLogicalWidth,
                                                         LayoutUnit borderAndPaddingLogicalHeight,
                                                         LayoutUnit logicalWidth, double aspectRatio,
                                                         BoxSizing boxSizing) noexcept
  {
    if (boxSizing == BoxSizing::BorderBox)
    {
      return LayoutUnit((logicalWidth + borderAndPaddingLogicalWidth) * aspectRatio)
             - borderAndPaddingLogicalHeight;
    }
    return LayoutUnit(logicalWidth * aspectRatio);
  }
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::RenderBox)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsRenderBox();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
