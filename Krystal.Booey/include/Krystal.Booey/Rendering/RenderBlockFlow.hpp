#pragma once

#include "Krystal.Booey/Rendering/RenderBlock.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo
{
  class RenderBlockFlow;

  /// @brief The maximal positive and negative margins on each block-axis side, collapsed so far.
  class MarginValues
  {
  private:
    LayoutUnit _positiveMarginBefore;
    LayoutUnit _negativeMarginBefore;
    LayoutUnit _positiveMarginAfter;
    LayoutUnit _negativeMarginAfter;

  public:
    MarginValues(LayoutUnit beforePos, LayoutUnit beforeNeg, LayoutUnit afterPos,
                 LayoutUnit afterNeg) noexcept
        : _positiveMarginBefore(beforePos), _negativeMarginBefore(beforeNeg), _positiveMarginAfter(afterPos),
          _negativeMarginAfter(afterNeg)
    {
    }

    KRYS_NODISCARD LayoutUnit PositiveMarginBefore() const noexcept
    {
      return _positiveMarginBefore;
    }
    KRYS_NODISCARD LayoutUnit NegativeMarginBefore() const noexcept
    {
      return _negativeMarginBefore;
    }
    KRYS_NODISCARD LayoutUnit PositiveMarginAfter() const noexcept
    {
      return _positiveMarginAfter;
    }
    KRYS_NODISCARD LayoutUnit NegativeMarginAfter() const noexcept
    {
      return _negativeMarginAfter;
    }

    void SetPositiveMarginBefore(LayoutUnit pos) noexcept
    {
      _positiveMarginBefore = pos;
    }
    void SetNegativeMarginBefore(LayoutUnit neg) noexcept
    {
      _negativeMarginBefore = neg;
    }
    void SetPositiveMarginAfter(LayoutUnit pos) noexcept
    {
      _positiveMarginAfter = pos;
    }
    void SetNegativeMarginAfter(LayoutUnit neg) noexcept
    {
      _negativeMarginAfter = neg;
    }
  };

  /// @brief The block flow's fields that rarely differ from their defaults. Allocated only when some of these
  /// fields have non-default values.
  // FIXME: The line to break at to avoid a widow and the multi-column flow, once there's pagination and
  // multi-column layout.
  struct RenderBlockFlowRareData
  {
    KRYS_NON_COPY_MOVABLE(RenderBlockFlowRareData);

    MarginValues Margins;
    LayoutUnit AlignContentShift; // Caches negative shifts for overflow calculation.

    explicit RenderBlockFlowRareData(const RenderBlockFlow &block) noexcept;

    KRYS_NODISCARD static LayoutUnit PositiveMarginBeforeDefault(const RenderBlock &block) noexcept
    {
      return std::max<LayoutUnit>(block.MarginBefore(), 0_lu);
    }
    KRYS_NODISCARD static LayoutUnit NegativeMarginBeforeDefault(const RenderBlock &block) noexcept
    {
      return std::max<LayoutUnit>(-block.MarginBefore(), 0_lu);
    }
    KRYS_NODISCARD static LayoutUnit PositiveMarginAfterDefault(const RenderBlock &block) noexcept
    {
      return std::max<LayoutUnit>(block.MarginAfter(), 0_lu);
    }
    KRYS_NODISCARD static LayoutUnit NegativeMarginAfterDefault(const RenderBlock &block) noexcept
    {
      return std::max<LayoutUnit>(-block.MarginAfter(), 0_lu);
    }
  };

  /// @brief A block container that lays out block or inline content.
  // FIXME: Inline layout (line boxes), floats, out-of-flow static positions, pagination, multi-column flows,
  // text autosizing, hit testing, selection and painting come with the rest of layout and with painting.
  class RenderBlockFlow : public RenderBlock
  {
    KRYS_NON_COPY_MOVABLE(RenderBlockFlow);
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderBlockFlow);

  protected:
    UniquePtr<RenderBlockFlowRareData> _rareBlockFlowData;

  public:
    RenderBlockFlow(Type type, dom::Element &element, RenderStyle &&style,
                    EnumSet<BlockFlowFlag> flags = {}) noexcept;
    RenderBlockFlow(Type type, dom::Document &document, RenderStyle &&style,
                    EnumSet<BlockFlowFlag> flags = {}) noexcept;
    ~RenderBlockFlow() noexcept override;

    void LayoutBlock(RelayoutChildren relayoutChildren,
                     LayoutUnit pageLogicalHeight = 0_lu) noexcept override;

    /// @brief The margin collapsing state while a block lays out its block children.
    class MarginInfo
    {
    public:
      enum class IgnoreScrollbarForAfterMargin : bool
      {
        No,
        Yes
      };

    private:
      // Collapsing flags for whether we can collapse our margins with our children's margins.
      bool _canCollapseWithChildren : 1 {false};
      bool _canCollapseMarginBeforeWithChildren : 1 {false};
      bool _canCollapseMarginAfterWithChildren : 1 {false};

      // Whether or not we are a quirky container, i.e., do we collapse away top and bottom
      // margins in our container. Table cells and the body are the common examples. We
      // also have a custom style property for Safari RSS to deal with TypePad blog articles.
      bool _quirkContainer : 1 {false};

      // This flag tracks whether we are still looking at child margins that can all collapse together at the
      // beginning of a block. They may or may not collapse with the top margin of the block
      // (|m_canCollapseTopWithChildren| tells us that), but they will always be collapsing with one another.
      // This variable can remain set to true through multiple iterations as long as we keep encountering
      // self-collapsing blocks.
      bool _atBeforeSideOfBlock : 1 {true};

      // This flag is set when we know we're examining bottom margins and we know we're at the bottom of the
      // block.
      bool _atAfterSideOfBlock : 1 {false};

      // These variables are used to detect quirky margins that we need to collapse away (in table cells
      // and in the body element).
      bool _hasMarginBeforeQuirk : 1 {false};
      bool _hasMarginAfterQuirk : 1 {false};
      bool _determinedMarginBeforeQuirk : 1 {false};

      // These flags track the previous maximal positive and negative margins.
      LayoutUnit _positiveMargin;
      LayoutUnit _negativeMargin;

    public:
      explicit MarginInfo(const RenderBlockFlow &block,
                          IgnoreScrollbarForAfterMargin ignoreScrollbarForAfterMargin =
                            IgnoreScrollbarForAfterMargin::Yes) noexcept;
      MarginInfo(bool canCollapseWithChildren, bool canCollapseMarginBeforeWithChildren,
                 bool canCollapseMarginAfterWithChildren, bool quirkContainer, bool atBeforeSideOfBlock,
                 bool atAfterSideOfBlock, bool hasMarginBeforeQuirk, bool hasMarginAfterQuirk,
                 bool determinedMarginBeforeQuirk, LayoutUnit positiveMargin,
                 LayoutUnit negativeMargin) noexcept;
      MarginInfo() noexcept = default;

      void SetAtBeforeSideOfBlock(bool atBeforeSideOfBlock) noexcept
      {
        _atBeforeSideOfBlock = atBeforeSideOfBlock;
      }
      void SetAtAfterSideOfBlock(bool atAfterSideOfBlock) noexcept
      {
        _atAfterSideOfBlock = atAfterSideOfBlock;
      }
      void ClearMargin() noexcept
      {
        _positiveMargin = 0_lu;
        _negativeMargin = 0_lu;
      }
      void SetHasMarginBeforeQuirk(bool hasMarginBeforeQuirk) noexcept
      {
        _hasMarginBeforeQuirk = hasMarginBeforeQuirk;
      }
      void SetHasMarginAfterQuirk(bool hasMarginAfterQuirk) noexcept
      {
        _hasMarginAfterQuirk = hasMarginAfterQuirk;
      }
      void SetDeterminedMarginBeforeQuirk(bool determinedMarginBeforeQuirk) noexcept
      {
        _determinedMarginBeforeQuirk = determinedMarginBeforeQuirk;
      }
      void SetPositiveMargin(LayoutUnit positiveMargin) noexcept
      {
        _positiveMargin = positiveMargin;
      }
      void SetNegativeMargin(LayoutUnit negativeMargin) noexcept
      {
        _negativeMargin = negativeMargin;
      }
      void SetPositiveMarginIfLarger(LayoutUnit positiveMargin) noexcept
      {
        _positiveMargin = std::max(positiveMargin, _positiveMargin);
      }
      void SetNegativeMarginIfLarger(LayoutUnit negativeMargin) noexcept
      {
        _negativeMargin = std::max(negativeMargin, _negativeMargin);
      }
      void SetMargin(LayoutUnit positiveMargin, LayoutUnit negativeMargin) noexcept
      {
        SetPositiveMargin(positiveMargin);
        SetNegativeMargin(negativeMargin);
      }
      void SetCanCollapseMarginAfterWithChildren(bool canCollapse) noexcept
      {
        _canCollapseMarginAfterWithChildren = canCollapse;
      }

      KRYS_NODISCARD bool AtBeforeSideOfBlock() const noexcept
      {
        return _atBeforeSideOfBlock;
      }
      KRYS_NODISCARD bool AtAfterSideOfBlock() const noexcept
      {
        return _atAfterSideOfBlock;
      }
      KRYS_NODISCARD bool CanCollapseWithMarginBefore() const noexcept
      {
        return _atBeforeSideOfBlock && _canCollapseMarginBeforeWithChildren;
      }
      KRYS_NODISCARD bool CanCollapseWithMarginAfter() const noexcept
      {
        return _atAfterSideOfBlock && _canCollapseMarginAfterWithChildren;
      }
      KRYS_NODISCARD bool CanCollapseMarginBeforeWithChildren() const noexcept
      {
        return _canCollapseMarginBeforeWithChildren;
      }
      KRYS_NODISCARD bool CanCollapseMarginAfterWithChildren() const noexcept
      {
        return _canCollapseMarginAfterWithChildren;
      }
      KRYS_NODISCARD bool CanCollapseWithChildren() const noexcept
      {
        return _canCollapseWithChildren;
      }
      KRYS_NODISCARD bool QuirkContainer() const noexcept
      {
        return _quirkContainer;
      }
      KRYS_NODISCARD bool DeterminedMarginBeforeQuirk() const noexcept
      {
        return _determinedMarginBeforeQuirk;
      }
      KRYS_NODISCARD bool HasMarginBeforeQuirk() const noexcept
      {
        return _hasMarginBeforeQuirk;
      }
      KRYS_NODISCARD bool HasMarginAfterQuirk() const noexcept
      {
        return _hasMarginAfterQuirk;
      }
      KRYS_NODISCARD LayoutUnit PositiveMargin() const noexcept
      {
        return _positiveMargin;
      }
      KRYS_NODISCARD LayoutUnit NegativeMargin() const noexcept
      {
        return _negativeMargin;
      }
      KRYS_NODISCARD LayoutUnit Margin() const noexcept
      {
        return _positiveMargin - _negativeMargin;
      }
    };

    KRYS_NODISCARD MarginValues MarginValuesForChild(RenderBox &child) const noexcept;

    void PerformBlockStepSizing(RenderBox &child, LayoutUnit blockStepSizeForChild) const noexcept;

    void LayoutBlockChild(RenderBox &child, MarginInfo &marginInfo, LayoutUnit &previousFloatLogicalBottom,
                          LayoutUnit &maxFloatLogicalBottom) noexcept;

    void TrimBlockEndChildrenMargins() noexcept;

    LayoutUnit CollapseMargins(RenderBox &child, MarginInfo &marginInfo) noexcept;
    LayoutUnit CollapseMarginsWithChildInfo(RenderBox *child, MarginInfo &marginInfo) noexcept;

    LayoutUnit ClearFloatsIfNeeded(RenderBox &child, MarginInfo &marginInfo, LayoutUnit oldTopPosMargin,
                                   LayoutUnit oldTopNegMargin, LayoutUnit yPos) noexcept;
    KRYS_NODISCARD LayoutUnit EstimateLogicalTopPosition(RenderBox &child, const MarginInfo &marginInfo,
                                                         LayoutUnit &estimateWithoutPagination) noexcept;
    void MarginBeforeEstimateForChild(RenderBox &child, LayoutUnit &positiveMarginBefore,
                                      LayoutUnit &negativeMarginBefore) const noexcept;
    LayoutUnit HandleAfterSideOfBlock(MarginInfo &marginInfo, LayoutUnit contentBoxLogicalHeight) noexcept;
    void SetCollapsedBottomMargin(const MarginInfo &marginInfo) noexcept;

    KRYS_NODISCARD bool ChildrenPreventSelfCollapsing() const noexcept final;

    // FIXME: Floats (the floating objects, containsFloat, lowestFloatLogicalBottom, the overhanging and
    // intruding floats) come with float layout; until then a block flow has no floats.
    KRYS_NODISCARD bool ContainsFloat(const RenderBox &) const noexcept
    {
      return false;
    }
    KRYS_NODISCARD LayoutUnit LowestFloatLogicalBottom() const noexcept
    {
      return 0_lu;
    }
    void MarkAllDescendantsWithFloatsForLayout(RenderBox *floatToRemove = nullptr,
                                               bool inLayout = true) noexcept;

    KRYS_NODISCARD LayoutUnit LogicalHeightForChildForFragmentation(const RenderBox &child) const noexcept
    {
      return LogicalHeightForChild(child);
    }

    KRYS_NODISCARD bool HasRareBlockFlowData() const noexcept
    {
      return static_cast<bool>(_rareBlockFlowData);
    }
    KRYS_NODISCARD RenderBlockFlowRareData *RareBlockFlowData() const noexcept
    {
      krys_assert(HasRareBlockFlowData());
      return _rareBlockFlowData.get();
    }
    RenderBlockFlowRareData &EnsureRareBlockFlowData() noexcept;
    void MaterializeRareBlockFlowData() noexcept;

  protected:
    void LayoutBlockWithNoChildren() noexcept;

    // This method is called at the start of layout to wipe away all of the floats in our floating objects
    // list. It also repopulates the list with any floats that intrude from previous siblings or parents.
    // Floats that were added by descendants are gone when this call completes and will get added back later
    // on after the children have gotten a relayout.
    void RebuildFloatingObjectSetFromIntrudingFloats() noexcept;

    // RenderBlockFlow always contains either lines or paragraphs. When the children are all blocks (e.g.
    // paragraphs), we call layoutBlockChildren. When the children are all inline (e.g., lines), we call
    // layoutInlineChildren.
    void LayoutInFlowChildren(RelayoutChildren relayoutChildren, LayoutUnit previousHeight,
                              LayoutUnit &repaintLogicalTop, LayoutUnit &repaintLogicalBottom,
                              LayoutUnit &maxFloatLogicalBottom) noexcept;
    void LayoutBlockChildren(RelayoutChildren relayoutChildren, LayoutUnit &maxFloatLogicalBottom) noexcept;
    void LayoutInlineChildren(RelayoutChildren relayoutChildren, LayoutUnit previousHeight,
                              LayoutUnit &repaintLogicalTop, LayoutUnit &repaintLogicalBottom) noexcept;

    void SimplifiedNormalFlowLayout() noexcept override;
    LayoutUnit ShiftForAlignContent(LayoutUnit intrinsicLogicalHeight, LayoutUnit &repaintLogicalTop,
                                    LayoutUnit &repaintLogicalBottom) noexcept;

    void AddOverflowFromInFlowChildren(EnumSet<ComputeOverflowOptions> options = {}) noexcept override;

    // RenderBlockFlows override these methods, since they are the only class that supports margin collapsing.
    KRYS_NODISCARD LayoutUnit CollapsedMarginBefore() const noexcept final
    {
      return MaxPositiveMarginBefore() - MaxNegativeMarginBefore();
    }
    KRYS_NODISCARD LayoutUnit CollapsedMarginAfter() const noexcept final
    {
      return MaxPositiveMarginAfter() - MaxNegativeMarginAfter();
    }

    KRYS_NODISCARD bool IsChildEligibleForMarginTrim(style::MarginTrimSide marginTrimSide,
                                                     const RenderBox &child) const noexcept final;

    KRYS_NODISCARD bool ShouldResetLogicalHeightBeforeLayout() const noexcept override
    {
      return true;
    }

    void ComputeIntrinsicLogicalWidths(LayoutUnit &minLogicalWidth,
                                       LayoutUnit &maxLogicalWidth) const noexcept override;

    KRYS_NODISCARD LayoutUnit MaxPositiveMarginBefore() const noexcept
    {
      return HasRareBlockFlowData() ? RareBlockFlowData()->Margins.PositiveMarginBefore()
                                    : RenderBlockFlowRareData::PositiveMarginBeforeDefault(*this);
    }
    KRYS_NODISCARD LayoutUnit MaxNegativeMarginBefore() const noexcept
    {
      return HasRareBlockFlowData() ? RareBlockFlowData()->Margins.NegativeMarginBefore()
                                    : RenderBlockFlowRareData::NegativeMarginBeforeDefault(*this);
    }
    KRYS_NODISCARD LayoutUnit MaxPositiveMarginAfter() const noexcept
    {
      return HasRareBlockFlowData() ? RareBlockFlowData()->Margins.PositiveMarginAfter()
                                    : RenderBlockFlowRareData::PositiveMarginAfterDefault(*this);
    }
    KRYS_NODISCARD LayoutUnit MaxNegativeMarginAfter() const noexcept
    {
      return HasRareBlockFlowData() ? RareBlockFlowData()->Margins.NegativeMarginAfter()
                                    : RenderBlockFlowRareData::NegativeMarginAfterDefault(*this);
    }

    void InitMaxMarginValues() noexcept
    {
      if (!HasRareBlockFlowData())
      {
        return;
      }

      RareBlockFlowData()->Margins = MarginValues(RenderBlockFlowRareData::PositiveMarginBeforeDefault(*this),
                                                  RenderBlockFlowRareData::NegativeMarginBeforeDefault(*this),
                                                  RenderBlockFlowRareData::PositiveMarginAfterDefault(*this),
                                                  RenderBlockFlowRareData::NegativeMarginAfterDefault(*this));
    }

    void SetMaxMarginBeforeValues(LayoutUnit pos, LayoutUnit neg) noexcept;
    void SetMaxMarginAfterValues(LayoutUnit pos, LayoutUnit neg) noexcept;

    void StyleDidChange(style::Difference diff, const RenderStyle *oldStyle) noexcept override;

    KRYS_NODISCARD Maybe<LayoutUnit> FirstLineBaseline() const noexcept override;
    KRYS_NODISCARD Maybe<LayoutUnit> LastLineBaseline() const noexcept override;

    KRYS_NODISCARD LayoutOptionalOutsets AllowedLayoutOverflow() const noexcept override;

  private:
    bool RecomputeLogicalWidthAndColumnWidth() noexcept;

    KRYS_NODISCARD LayoutUnit AddOverhangingFloats(RenderBlockFlow &child,
                                                   bool makeChildPaintOtherFloats) noexcept;
    KRYS_NODISCARD LayoutUnit ComputedClearDeltaForChild(RenderBox &child, LayoutUnit yPos) noexcept;

    void
      DetermineLogicalLeftPositionForChild(RenderBox &child,
                                           ApplyLayoutDeltaMode applyDelta = DoNotApplyLayoutDelta) noexcept;

    void DirtyForLayoutFromPercentageHeightDescendants() noexcept;

    KRYS_NODISCARD Maybe<LayoutUnit> SelfCollapsingMarginBeforeWithClear(RenderObject *candidate) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::RenderBlockFlow)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsRenderBlockFlow();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
