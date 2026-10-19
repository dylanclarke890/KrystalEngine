#pragma once

#include "Krystal.Booey/Rendering/OrderIterator.hpp"
#include "Krystal.Booey/Rendering/RenderBlock.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/HashSet.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo
{
  class BaselineAlignmentState;

  /// @brief A flex container: lays out its children as flex items, in flex lines.
  // FIXME: Hit testing (hitTestChildren), once there is hit testing.
  class RenderFlexibleBox : public RenderBlock
  {
    KRYS_NON_COPY_MOVABLE(RenderFlexibleBox);
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderFlexibleBox);

  public:
    using Direction = FlowDirection;

    enum class GapType : uint8
    {
      BetweenLines,
      BetweenItems
    };

  private:
    class FlexLayoutItem
    {
    public:
      FlexLayoutItem(RenderBox &flexItem, LayoutUnit flexBaseContentSize, LayoutUnit mainAxisBorderAndPadding,
                     LayoutUnit mainAxisMargin, std::pair<LayoutUnit, LayoutUnit> minMaxSizes,
                     bool everHadLayout) noexcept;

      KRYS_NODISCARD LayoutUnit HypotheticalMainAxisMarginBoxSize() const noexcept;
      KRYS_NODISCARD LayoutUnit FlexBaseMarginBoxSize() const noexcept;
      KRYS_NODISCARD LayoutUnit FlexedMarginBoxSize() const noexcept;
      KRYS_NODISCARD const RenderStyle &Style() const noexcept;
      KRYS_NODISCARD LayoutUnit ConstrainSizeByMinMax(LayoutUnit size) const noexcept;

      RenderBox *Renderer;
      LayoutUnit FlexBaseContentSize;
      LayoutUnit MainAxisBorderAndPadding;
      mutable LayoutUnit MainAxisMargin;
      std::pair<LayoutUnit, LayoutUnit> MinMaxSizes;
      LayoutUnit HypotheticalMainContentSize;
      LayoutUnit FlexedContentSize;
      bool Frozen {false};
      bool EverHadLayout {false};
    };

    enum class FlexSign : uint8
    {
      PositiveFlexibility,
      NegativeFlexibility,
    };

    enum class SizeDefiniteness : uint8
    {
      Definite,
      Indefinite,
      Unknown
    };

    using FlexItemFrameRects = List<LayoutRect>;
    using FlexLayoutItems = List<FlexLayoutItem>;

    struct LineState;
    using FlexLineStates = List<LineState>;

    struct FlexingLineData
    {
      FlexLayoutItems LineItems;
      LayoutUnit SumFlexBaseSize;
      double TotalFlexGrow {0};
      double TotalFlexShrink {0};
      double TotalWeightedFlexShrink {0};
      LayoutUnit SumHypotheticalMainSize;
    };

    // The flex items are keyed by address: the caches are cleared for an item when it leaves the container
    // (RenderTreeBuilder) or goes out of flow (RenderBox::StyleWillChange).

    // This is used to cache the preferred size for orthogonal flow children so we
    // don't have to relayout to get it
    HashMap<const RenderBox *, LayoutUnit> _intrinsicSizeAlongMainAxis;

    // This is used to cache the intrinsic size on the cross axis to avoid
    // relayouts when stretching.
    HashMap<const RenderBox *, LayoutUnit> _intrinsicContentLogicalHeights;

    // This set is used to keep track of which children we laid out in this
    // current layout iteration. We need it because the ones in this set may
    // need an additional layout pass for correct stretch alignment handling, as
    // the first layout likely did not use the correct value for percentage
    // sizing of children.
    HashSet<const RenderBox *> _relaidOutFlexItems;

    mutable OrderIterator _orderIterator {*this};
    size_t _numberOfFlexItemsOnFirstLine {0};
    size_t _numberOfFlexItemsOnLastLine {0};

    struct MarginTrimItems
    {
      HashSet<const RenderBox *> ItemsAtFlexLineStart;
      HashSet<const RenderBox *> ItemsAtFlexLineEnd;
      HashSet<const RenderBox *> ItemsOnFirstFlexLine;
      HashSet<const RenderBox *> ItemsOnLastFlexLine;
    } _marginTrimItems;

    LayoutUnit _alignContentStartOverflow {0};
    LayoutUnit _justifyContentStartOverflow {0};

    // This is SizeIsUnknown outside of layoutBlock()
    SizeDefiniteness _hasDefiniteHeight {SizeDefiniteness::Unknown};
    bool _inLayout {false};
    bool _afterMainAxisItemSizing {false};
    bool _afterCrossAxisItemSizing {false};
    bool _inSimplifiedLayout {false};
    bool _inPostFlexUpdateScrollbarLayout {false};
    mutable bool _inFlexItemIntrinsicWidthComputation {false};
    bool _shouldResetFlexItemLogicalHeightBeforeLayout {false};
    bool _isComputingFlexBaseSizes {false};
    Maybe<bool> _hasFlexFormattingContextLayout;

  public:
    RenderFlexibleBox(Type type, dom::Element &element, RenderStyle &&style) noexcept;
    RenderFlexibleBox(Type type, dom::Document &document, RenderStyle &&style) noexcept;
    ~RenderFlexibleBox() noexcept override;

    KRYS_NODISCARD utf8_stringview RenderName() const noexcept override
    {
      return u8"RenderFlexibleBox";
    }

    KRYS_NODISCARD bool CanDropAnonymousBlockChild() const noexcept final
    {
      return false;
    }
    void LayoutBlock(RelayoutChildren relayoutChildren, LayoutUnit pageLogicalHeight = 0_lu) noexcept final;

    KRYS_NODISCARD Maybe<LayoutUnit> FirstLineBaseline() const noexcept override;
    KRYS_NODISCARD Maybe<LayoutUnit> LastLineBaseline() const noexcept override;

    void StyleDidChange(style::Difference diff, const RenderStyle *oldStyle) noexcept override;

    KRYS_NODISCARD bool IsHorizontalFlow() const noexcept;
    KRYS_NODISCARD Direction CrossAxisDirection() const noexcept;

    KRYS_NODISCARD const OrderIterator &GetOrderIterator() const noexcept
    {
      return _orderIterator;
    }

    KRYS_NODISCARD LayoutOptionalOutsets AllowedLayoutOverflow() const noexcept override;

    KRYS_NODISCARD virtual bool IsFlexibleBoxImpl() const noexcept
    {
      return false;
    }

    KRYS_NODISCARD Maybe<LayoutUnit>
      UsedFlexItemOverridingLogicalHeightForPercentageResolution(const RenderBox &flexItem) noexcept;
    KRYS_NODISCARD bool CanUseFlexItemForPercentageResolution(const RenderBox &flexItem) noexcept;

    void ClearCachedMainSizeForFlexItem(const RenderBox &flexItem) noexcept;

    KRYS_NODISCARD LayoutUnit
      CachedFlexItemIntrinsicContentLogicalHeight(const RenderBox &flexItem) const noexcept;
    void SetCachedFlexItemIntrinsicContentLogicalHeight(const RenderBox &flexItem,
                                                        LayoutUnit height) noexcept;
    void ClearCachedFlexItemIntrinsicContentLogicalHeight(const RenderBox &flexItem) noexcept;

    KRYS_NODISCARD LayoutUnit StaticMainAxisPositionForPositionedFlexItem(const RenderBox &flexItem) noexcept;
    KRYS_NODISCARD LayoutUnit
      StaticCrossAxisPositionForPositionedFlexItem(const RenderBox &flexItem) noexcept;

    KRYS_NODISCARD LayoutUnit StaticInlinePositionForPositionedFlexItem(const RenderBox &flexItem) noexcept;
    KRYS_NODISCARD LayoutUnit StaticBlockPositionForPositionedFlexItem(const RenderBox &flexItem) noexcept;

    // Returns true if the position changed. In that case, the flexItem will have to
    // be laid out again.
    bool SetStaticPositionForPositionedLayout(const RenderBox &flexItem) noexcept;

    KRYS_NODISCARD LayoutUnit ComputeGap(GapType gapType) const noexcept;

    KRYS_NODISCARD bool ShouldApplyMinBlockSizeAutoForFlexItem(const RenderBox &flexItem) const noexcept;

    KRYS_NODISCARD bool IsComputingFlexBaseSizes() const noexcept
    {
      return _isComputingFlexBaseSizes;
    }

    KRYS_NODISCARD static Maybe<TextDirection>
      LeftRightAxisDirectionFromStyle(const RenderStyle &style) noexcept;

    KRYS_NODISCARD bool HasModernLayout() const noexcept
    {
      return _hasFlexFormattingContextLayout && *_hasFlexFormattingContextLayout;
    }

    KRYS_NODISCARD bool ShouldResetFlexItemLogicalHeightBeforeLayout() const noexcept
    {
      return _shouldResetFlexItemLogicalHeightBeforeLayout;
    }

    KRYS_NODISCARD bool IsColumnOrRowReverse() const noexcept;
    KRYS_NODISCARD bool IsWrapReverse() const noexcept;

  protected:
    void PaintChildren(PaintInfo &forSelf, const LayoutPoint &, PaintInfo &forChild) noexcept override;

    void ComputeIntrinsicLogicalWidths(LayoutUnit &minLogicalWidth,
                                       LayoutUnit &maxLogicalWidth) const noexcept override;

  private:
    KRYS_NODISCARD bool MainAxisIsFlexItemInlineAxis(const RenderBox &flexItem) const noexcept;
    KRYS_NODISCARD bool IsColumnFlow() const noexcept;
    KRYS_NODISCARD bool IsLeftToRightFlow() const noexcept;
    KRYS_NODISCARD bool IsMultiline() const noexcept;
    KRYS_NODISCARD style::FlexBasis FlexBasisForFlexItem(const RenderBox &flexItem) const noexcept;
    KRYS_NODISCARD const style::PreferredSize &
      PreferredMainSizeLengthForFlexItem(const RenderBox &flexItem) const noexcept;
    KRYS_NODISCARD const style::MinimumSize &
      MinMainSizeLengthForFlexItem(const RenderBox &flexItem) const noexcept;
    KRYS_NODISCARD const style::MaximumSize &
      MaxMainSizeLengthForFlexItem(const RenderBox &flexItem) const noexcept;
    KRYS_NODISCARD const style::PreferredSize &
      PreferredCrossSizeLengthForFlexItem(const RenderBox &flexItem) const noexcept;
    KRYS_NODISCARD const style::MinimumSize &
      MinCrossSizeLengthForFlexItem(const RenderBox &flexItem) const noexcept;
    KRYS_NODISCARD const style::MaximumSize &
      MaxCrossSizeLengthForFlexItem(const RenderBox &flexItem) const noexcept;
    KRYS_NODISCARD bool ShouldApplyMinSizeAutoForFlexItem(const RenderBox &flexItem) const noexcept;
    KRYS_NODISCARD LayoutUnit CrossAxisExtentForFlexItem(const RenderBox &flexItem) const noexcept;
    KRYS_NODISCARD LayoutUnit CrossAxisIntrinsicExtentForFlexItem(RenderBox &flexItem) noexcept;
    KRYS_NODISCARD LayoutUnit FlexItemIntrinsicLogicalHeight(RenderBox &flexItem) const noexcept;
    KRYS_NODISCARD LayoutUnit FlexItemIntrinsicLogicalWidth(RenderBox &flexItem) noexcept;
    KRYS_NODISCARD LayoutUnit MainAxisExtentForFlexItem(const RenderBox &flexItem) const noexcept;
    KRYS_NODISCARD LayoutUnit
      MainAxisContentExtentForFlexItemIncludingScrollbar(const RenderBox &flexItem) const noexcept;
    KRYS_NODISCARD LayoutUnit CrossAxisExtent() const noexcept;
    KRYS_NODISCARD LayoutUnit MainAxisExtent() const noexcept;
    KRYS_NODISCARD LayoutUnit CrossAxisContentExtent() const noexcept;
    KRYS_NODISCARD LayoutUnit MainAxisContentExtent(LayoutUnit contentLogicalHeight) noexcept;
    template <typename TSize>
    KRYS_NODISCARD Maybe<LayoutUnit> ComputeMainAxisExtentForFlexItem(RenderBox &flexItem,
                                                                      const TSize &size) noexcept;
    KRYS_NODISCARD FlowDirection TransformedBlockFlowDirection() const noexcept;
    KRYS_NODISCARD LayoutUnit FlowAwareBorderStart() const noexcept;
    KRYS_NODISCARD LayoutUnit FlowAwareBorderEnd() const noexcept;
    KRYS_NODISCARD LayoutUnit FlowAwareBorderBefore() const noexcept;
    KRYS_NODISCARD LayoutUnit FlowAwareBorderAfter() const noexcept;
    KRYS_NODISCARD LayoutUnit FlowAwarePaddingStart() const noexcept;
    KRYS_NODISCARD LayoutUnit FlowAwarePaddingEnd() const noexcept;
    KRYS_NODISCARD LayoutUnit FlowAwarePaddingBefore() const noexcept;
    KRYS_NODISCARD LayoutUnit FlowAwarePaddingAfter() const noexcept;
    KRYS_NODISCARD LayoutUnit FlowAwareMarginStartForFlexItem(const RenderBox &flexItem) const noexcept;
    KRYS_NODISCARD LayoutUnit FlowAwareMarginEndForFlexItem(const RenderBox &flexItem) const noexcept;
    KRYS_NODISCARD LayoutUnit FlowAwareMarginBeforeForFlexItem(const RenderBox &flexItem) const noexcept;
    KRYS_NODISCARD LayoutUnit CrossAxisMarginExtentForFlexItem(const RenderBox &flexItem) const noexcept;
    KRYS_NODISCARD LayoutUnit MainAxisMarginExtentForFlexItem(const RenderBox &flexItem) const noexcept;
    KRYS_NODISCARD LayoutUnit CrossAxisScrollbarExtent() const noexcept;
    KRYS_NODISCARD LayoutUnit MainAxisScrollbarExtent() const noexcept;
    KRYS_NODISCARD LayoutPoint FlowAwareLocationForFlexItem(const RenderBox &flexItem) const noexcept;

    KRYS_NODISCARD double PreferredAspectRatioForFlexItem(const RenderBox &flexItem) const noexcept;
    KRYS_NODISCARD bool FlexItemHasComputableAspectRatio(const RenderBox &flexItem) const noexcept;
    KRYS_NODISCARD bool
      FlexItemHasComputableAspectRatioAndCrossSizeIsConsideredDefinite(const RenderBox &flexItem) noexcept;

    KRYS_NODISCARD bool
      FlexItemCrossSizeShouldUseContainerCrossSize(const RenderBox &flexItem) const noexcept;
    KRYS_NODISCARD LayoutUnit
      ComputeCrossSizeForFlexItemUsingContainerCrossSize(const RenderBox &flexItem) const noexcept;
    void ComputeChildIntrinsicLogicalWidths(RenderBox &flexBoxChild, LayoutUnit &minPreferredLogicalWidth,
                                            LayoutUnit &maxPreferredLogicalWidth) const noexcept override;
    template <typename TSize>
    KRYS_NODISCARD LayoutUnit ComputeMainSizeFromAspectRatioUsing(
      const RenderBox &flexItem, const TSize &crossSizeLength) const noexcept;
    void SetFlowAwareLocationForFlexItem(RenderBox &flexItem, const LayoutPoint &location) noexcept;
    KRYS_NODISCARD LayoutUnit ComputeFlexBaseSizeForFlexItem(RenderBox &flexItem,
                                                             LayoutUnit mainAxisBorderAndPadding,
                                                             RelayoutChildren relayoutChildren) noexcept;
    void MaybeCacheFlexItemMainIntrinsicSize(RenderBox &flexItem, RelayoutChildren relayoutChildren) noexcept;
    void AdjustAlignmentForFlexItem(RenderBox &flexItem, LayoutUnit delta) noexcept;
    KRYS_NODISCARD ItemPosition AlignmentForFlexItem(const RenderBox &flexItem) const noexcept;
    KRYS_NODISCARD OverflowAlignment OverflowAlignmentForFlexItem(const RenderBox &flexItem) const noexcept;
    template <typename TSize>
    KRYS_NODISCARD bool
      CanComputePercentageFlexBasis(const RenderBox &flexItem, const TSize &flexBasis,
                                    UpdatePercentageHeightDescendants updateDescendants) noexcept;
    template <typename TSize>
    KRYS_NODISCARD bool FlexItemMainSizeIsDefinite(const RenderBox &flexItem, const TSize &size) noexcept;
    template <typename TSize>
    KRYS_NODISCARD bool FlexItemCrossSizeIsDefinite(const RenderBox &flexItem, const TSize &size) noexcept;
    KRYS_NODISCARD bool NeedToStretchFlexItemLogicalHeight(const RenderBox &flexItem) const noexcept;
    KRYS_NODISCARD bool FlexItemHasIntrinsicMainAxisSize(const RenderBox &flexItem) noexcept;
    KRYS_NODISCARD Overflow MainAxisOverflowForFlexItem(const RenderBox &flexItem) const noexcept;
    KRYS_NODISCARD Overflow CrossAxisOverflowForFlexItem(const RenderBox &flexItem) const noexcept;
    void CacheFlexItemMainSize(const RenderBox &flexItem) noexcept;

    void PerformFlexLayout(RelayoutChildren relayoutChildren) noexcept;

    KRYS_NODISCARD Maybe<FlexingLineData> ComputeNextFlexLine(size_t &nextIndex,
                                                              const FlexLayoutItems &allItems,
                                                              LayoutUnit lineBreakLength,
                                                              LayoutUnit gapBetweenItems) noexcept;

    KRYS_NODISCARD LayoutUnit AutoMarginOffsetInMainAxis(const FlexLayoutItems &flexLayoutItems,
                                                         LayoutUnit &availableFreeSpace) noexcept;
    void UpdateAutoMarginsInMainAxis(RenderBox &flexItem, LayoutUnit autoMarginOffset) noexcept;

    void InitializeMarginTrimState() noexcept;
    // Start margin parallel with the cross axis
    KRYS_NODISCARD bool ShouldTrimMainAxisMarginStart() const noexcept;
    // End margin parallel with the cross axis
    KRYS_NODISCARD bool ShouldTrimMainAxisMarginEnd() const noexcept;
    // Margins parallel with the main axis
    KRYS_NODISCARD bool ShouldTrimCrossAxisMarginStart() const noexcept;
    KRYS_NODISCARD bool ShouldTrimCrossAxisMarginEnd() const noexcept;
    void TrimMainAxisMarginStart(const FlexLayoutItem &flexLayoutItem) noexcept;
    void TrimMainAxisMarginEnd(const FlexLayoutItem &flexLayoutItem) noexcept;
    void TrimCrossAxisMarginStart(const FlexLayoutItem &flexLayoutItem) noexcept;
    void TrimCrossAxisMarginEnd(const FlexLayoutItem &flexLayoutItem) noexcept;
    KRYS_NODISCARD bool IsChildEligibleForMarginTrim(style::MarginTrimSide marginTrimSide,
                                                     const RenderBox &flexItem) const noexcept final;
    KRYS_NODISCARD bool CanFitItemWithTrimmedMarginEnd(const FlexLayoutItem &flexLayoutItem,
                                                       LayoutUnit sumHypotheticalMainSize,
                                                       LayoutUnit lineBreakLength) const noexcept;
    void RemoveMarginEndFromFlexSizes(FlexLayoutItem &flexLayoutItem, LayoutUnit &sumFlexBaseSize,
                                      LayoutUnit &sumHypotheticalMainSize) const noexcept;

    KRYS_NODISCARD bool HasAutoMarginsInCrossAxis(const RenderBox &flexItem) const noexcept;
    bool UpdateAutoMarginsInCrossAxis(RenderBox &flexItem, LayoutUnit availableAlignmentSpace) noexcept;
    void RepositionLogicalHeightDependentFlexItems(FlexLineStates &lineStates,
                                                   LayoutUnit gapBetweenLines) noexcept;

    KRYS_NODISCARD LayoutUnit AvailableAlignmentSpaceForFlexItem(LayoutUnit lineCrossAxisExtent,
                                                                 const RenderBox &flexItem) noexcept;
    KRYS_NODISCARD LayoutUnit MarginBoxAscentForFlexItem(const RenderBox &flexItem) noexcept;

    KRYS_NODISCARD LayoutUnit ComputeFlexItemMarginValue(const style::MarginEdge &margin) noexcept;
    void PrepareOrderIteratorAndMargins() noexcept;
    KRYS_NODISCARD std::pair<LayoutUnit, LayoutUnit> ComputeFlexItemMinMaxSizes(RenderBox &flexItem) noexcept;
    KRYS_NODISCARD LayoutUnit AdjustFlexItemSizeForAspectRatioCrossAxisMinAndMax(
      const RenderBox &flexItem, LayoutUnit flexItemSize) noexcept;
    KRYS_NODISCARD FlexLayoutItem ConstructFlexLayoutItem(RenderBox &flexItem,
                                                          RelayoutChildren relayoutChildren) noexcept;

    void FreezeInflexibleItems(FlexSign flexSign, FlexLayoutItems &flexLayoutItems,
                               LayoutUnit &remainingFreeSpace, double &totalFlexGrow, double &totalFlexShrink,
                               double &totalWeightedFlexShrink) noexcept;
    bool ResolveFlexibleLengths(FlexSign flexSign, FlexLayoutItems &flexLayoutItems,
                                LayoutUnit initialFreeSpace, LayoutUnit &remainingFreeSpace,
                                double &totalFlexGrow, double &totalFlexShrink,
                                double &totalWeightedFlexShrink) noexcept;
    void FreezeViolations(List<FlexLayoutItem *> &violations, LayoutUnit &availableFreeSpace,
                          double &totalFlexGrow, double &totalFlexShrink,
                          double &totalWeightedFlexShrink) noexcept;

    void ResetAutoMarginsAndLogicalTopInCrossAxis(RenderBox &flexItem) noexcept;
    void SetOverridingMainSizeForFlexItem(RenderBox &flexItem, LayoutUnit preferredSize) noexcept;
    void PrepareFlexItemForPositionedLayout(RenderBox &flexItem) noexcept;
    void LayoutAndPlaceFlexItems(LayoutUnit &crossAxisOffset, FlexLayoutItems &flexLayoutItems,
                                 LayoutUnit availableFreeSpace, RelayoutChildren relayoutChildren,
                                 FlexLineStates &lineStates, LayoutUnit gapBetweenItems) noexcept;
    void LayoutColumnReverse(const FlexLayoutItems &flexLayoutItems, LayoutUnit crossAxisOffset,
                             LayoutUnit availableFreeSpace, LayoutUnit gapBetweenItems) noexcept;
    void AlignFlexLines(FlexLineStates &lineStates, LayoutUnit gapBetweenLines) noexcept;
    void AlignFlexItems(FlexLineStates &lineStates) noexcept;
    void ApplyStretchAlignmentToFlexItem(RenderBox &flexItem, LayoutUnit lineCrossAxisExtent) noexcept;
    void PerformBaselineAlignment(LineState &lineState) noexcept;
    void FlipForRightToLeftColumn(const FlexLineStates &lineStates) noexcept;
    void FlipForWrapReverse(const FlexLineStates &lineStates, LayoutUnit crossAxisStartEdge) noexcept;

    KRYS_NODISCARD bool FlexItemHasPercentHeightDescendants(const RenderBox &renderer) const noexcept;

    void ResetHasDefiniteHeight() noexcept
    {
      _hasDefiniteHeight = SizeDefiniteness::Unknown;
    }
    KRYS_NODISCARD const RenderBox *FlexItemForFirstBaseline() const noexcept;
    KRYS_NODISCARD const RenderBox *FlexItemForLastBaseline() const noexcept;
    KRYS_NODISCARD const RenderBox *FirstBaselineCandidateOnLine(OrderIterator flexItemIterator,
                                                                 size_t numberOfItemsOnLine) const noexcept;
    KRYS_NODISCARD const RenderBox *LastBaselineCandidateOnLine(OrderIterator flexItemIterator,
                                                                size_t numberOfItemsOnLine) const noexcept;

    bool LayoutUsingFlexFormattingContext() noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::RenderFlexibleBox)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsRenderFlexibleBox();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
