#pragma once

#include "Krystal.Booey/Rendering/FloatingObjects.hpp"
#include "Krystal.Booey/Rendering/RenderBlock.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/Pair.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo::layout_integration
{
  class LineLayout;
}

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
  // FIXME: Inline layout (line boxes), pagination, multi-column flows,
  // text autosizing, hit testing, selection and painting come with the rest of layout and with painting.
  class RenderBlockFlow : public RenderBlock
  {
    KRYS_NON_COPY_MOVABLE(RenderBlockFlow);
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderBlockFlow);

  private:
    // Used to store state between styleWillChange and styleDidChange
    static bool s_canPropagateFloatIntoSibling;

  public:
    enum class InvalidationReason : uint8
    {
      InternalMove,       // (anon) block is moved or collapsed
      InsertionOrRemoval, // child renderer gets constructed/goes away
      ContentChange       // existing renderer gets changed (text content only atm)
    };

    // SVG text (the legacy line layout) isn't supported, so the path is either undetermined or inline.
    enum class LineLayoutPath : bool
    {
      UndeterminedPath,
      InlinePath
    };

  protected:
    UniquePtr<FloatingObjects> _floatingObjects;
    UniquePtr<RenderBlockFlowRareData> _rareBlockFlowData;

  private:
    UniquePtr<layout_integration::LineLayout> _lineLayout;
    LineLayoutPath _lineLayoutPath {LineLayoutPath::UndeterminedPath};
    Maybe<Pair<float, float>> _previousInlineLayoutContentTopAndBottomIncludingInkOverflow;

  public:
    RenderBlockFlow(Type type, dom::Element &element, RenderStyle &&style,
                    EnumSet<BlockFlowFlag> flags = {}) noexcept;
    RenderBlockFlow(Type type, dom::Document &document, RenderStyle &&style,
                    EnumSet<BlockFlowFlag> flags = {}) noexcept;
    ~RenderBlockFlow() noexcept override;

    void LayoutBlock(RelayoutChildren relayoutChildren,
                     LayoutUnit pageLogicalHeight = 0_lu) noexcept override;

    void SetChildrenInline(bool value) noexcept final;

    KRYS_NODISCARD bool HasContentfulInlineOrBlockLine() const noexcept;
    KRYS_NODISCARD bool HasContentfulInlineLine() const noexcept;
    KRYS_NODISCARD bool HasBlocksInInlineLayout() const noexcept;

    void InvalidateLineLayout(InvalidationReason invalidationReason) noexcept;
    void ComputeAndSetLineLayoutPath() noexcept;

    KRYS_NODISCARD LineLayoutPath GetLineLayoutPath() const noexcept
    {
      return _lineLayoutPath;
    }
    void SetLineLayoutPath(LineLayoutPath path) noexcept
    {
      _lineLayoutPath = path;
    }

    KRYS_NODISCARD size_t LineCount() const noexcept;

    KRYS_NODISCARD const layout_integration::LineLayout *InlineLayout() const noexcept
    {
      return _lineLayout.get();
    }
    KRYS_NODISCARD layout_integration::LineLayout *InlineLayout() noexcept
    {
      return _lineLayout.get();
    }

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

    struct BlockPositionAndMargin
    {
      LayoutUnit ChildLogicalTop;
      // Note that with self collapsing block, the container's logical bottom is not necessarily the child's
      // logical top plus its logical height.
      LayoutUnit ContainerLogicalBottom;
      MarginInfo ChildMarginInfo;
    };
    /// @brief Lays out a block in inline content, called from the inline formatting context.
    BlockPositionAndMargin LayoutBlockChildFromInlineLayout(RenderBox &child, LayoutUnit blockLogicalTop,
                                                            MarginInfo marginInfo) noexcept;

    void AdjustOutOfFlowBlock(RenderBox &child, const MarginInfo &marginInfo) noexcept;

    void TrimBlockEndChildrenMargins() noexcept;

    void SetStaticInlinePositionForChild(RenderBox &child, LayoutUnit inlinePosition) noexcept;
    void UpdateStaticInlinePositionForChild(RenderBox &child, LayoutUnit logicalTop) noexcept;

    KRYS_NODISCARD LayoutUnit StaticInlinePositionForOriginalDisplayInline(LayoutUnit logicalTop) noexcept;

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

    void AdjustFloatingBlock(const MarginInfo &marginInfo) noexcept;

    KRYS_NODISCARD bool ContainsFloats() const noexcept override
    {
      return _floatingObjects && !_floatingObjects->Set().isEmpty();
    }
    KRYS_NODISCARD bool ContainsFloat(const RenderBox &renderer) const noexcept;
    KRYS_NODISCARD bool SubtreeContainsFloats() const noexcept;
    KRYS_NODISCARD bool SubtreeContainsFloat(const RenderBox &renderer) const noexcept;

    KRYS_NODISCARD LayoutUnit LowestFloatLogicalBottom(
      FloatingObject::Type floatType = FloatingObject::FloatLeftRight) const noexcept;

    void RemoveFloatingObjects() noexcept;
    void MarkAllDescendantsWithFloatsForLayout(RenderBox *floatToRemove = nullptr,
                                               bool inLayout = true) noexcept;
    void MarkSiblingsWithFloatsForLayout(RenderBox *floatToRemove = nullptr) noexcept;

    KRYS_NODISCARD const FloatingObjectSet *GetFloatingObjectSet() const noexcept
    {
      return _floatingObjects ? &_floatingObjects->Set() : nullptr;
    }

    FloatingObject &InsertFloatingBox(RenderBox &floatBox) noexcept;

    KRYS_NODISCARD LayoutUnit LogicalTopForFloat(const FloatingObject &floatingObject) const noexcept
    {
      return IsHorizontalWritingMode() ? floatingObject.Y() : floatingObject.X();
    }
    KRYS_NODISCARD LayoutUnit LogicalBottomForFloat(const FloatingObject &floatingObject) const noexcept
    {
      return IsHorizontalWritingMode() ? floatingObject.MaxY() : floatingObject.MaxX();
    }
    KRYS_NODISCARD LayoutUnit LogicalLeftForFloat(const FloatingObject &floatingObject) const noexcept
    {
      return IsHorizontalWritingMode() ? floatingObject.X() : floatingObject.Y();
    }
    KRYS_NODISCARD LayoutUnit LogicalRightForFloat(const FloatingObject &floatingObject) const noexcept
    {
      return IsHorizontalWritingMode() ? floatingObject.MaxX() : floatingObject.MaxY();
    }
    KRYS_NODISCARD LayoutUnit LogicalWidthForFloat(const FloatingObject &floatingObject) const noexcept
    {
      return IsHorizontalWritingMode() ? floatingObject.Width() : floatingObject.Height();
    }
    KRYS_NODISCARD LayoutUnit LogicalHeightForFloat(const FloatingObject &floatingObject) const noexcept
    {
      return IsHorizontalWritingMode() ? floatingObject.Height() : floatingObject.Width();
    }

    void SetLogicalTopForFloat(FloatingObject &floatingObject, LayoutUnit logicalTop) const noexcept
    {
      if (IsHorizontalWritingMode())
      {
        floatingObject.SetY(logicalTop);
      }
      else
      {
        floatingObject.SetX(logicalTop);
      }
    }
    void SetLogicalLeftForFloat(FloatingObject &floatingObject, LayoutUnit logicalLeft) const noexcept
    {
      if (IsHorizontalWritingMode())
      {
        floatingObject.SetX(logicalLeft);
      }
      else
      {
        floatingObject.SetY(logicalLeft);
      }
    }
    void SetLogicalHeightForFloat(FloatingObject &floatingObject, LayoutUnit logicalHeight) const noexcept
    {
      if (IsHorizontalWritingMode())
      {
        floatingObject.SetHeight(logicalHeight);
      }
      else
      {
        floatingObject.SetWidth(logicalHeight);
      }
    }
    void SetLogicalWidthForFloat(FloatingObject &floatingObject, LayoutUnit logicalWidth) const noexcept
    {
      if (IsHorizontalWritingMode())
      {
        floatingObject.SetWidth(logicalWidth);
      }
      else
      {
        floatingObject.SetHeight(logicalWidth);
      }
    }
    void SetLogicalMarginsForFloat(FloatingObject &floatingObject, LayoutUnit logicalLeftMargin,
                                   LayoutUnit logicalBeforeMargin) const noexcept
    {
      if (IsHorizontalWritingMode())
      {
        floatingObject.SetMarginOffset(LayoutSize(logicalLeftMargin, logicalBeforeMargin));
      }
      else
      {
        floatingObject.SetMarginOffset(LayoutSize(logicalBeforeMargin, logicalLeftMargin));
      }
    }

    KRYS_NODISCARD LayoutPoint FlipFloatForWritingModeForChild(const FloatingObject &child,
                                                               const LayoutPoint &point) const noexcept;

    /// @brief Copies the floats this block has, and @p toBlockFlow doesn't, to @p toBlockFlow, when the tree
    /// merges this anonymous block into it.
    void AddFloatsToNewParent(RenderBlockFlow &toBlockFlow) const noexcept;

    KRYS_NODISCARD Maybe<LayoutUnit> LowestInitialLetterLogicalBottom() const noexcept;

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
    void PaintFloats(PaintInfo &, const LayoutPoint &, bool preservePhase = false) noexcept override;
    void PaintInlineChildren(PaintInfo &, const LayoutPoint &) noexcept override;

  public:
    /// @brief Paints the blocks in this block's inline content, when it has no other painted content.
    void PaintBlockLevelContentInInline(PaintInfo &, const LayoutPoint &) noexcept;

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

    void StyleWillChange(style::Difference diff, const RenderStyle &newStyle) noexcept override;
    void StyleDidChange(style::Difference diff, const RenderStyle *oldStyle) noexcept override;

    void ComputeOverflow(LayoutUnit oldClientAfterEdge,
                         EnumSet<ComputeOverflowOptions> options = {}) noexcept override;

    void CreateFloatingObjects() noexcept;

    KRYS_NODISCARD LayoutUnit LogicalRightFloatOffsetForLine(
      LayoutUnit logicalTop, LayoutUnit fixedOffset, LayoutUnit logicalHeight) const noexcept override;
    KRYS_NODISCARD LayoutUnit LogicalLeftFloatOffsetForLine(LayoutUnit logicalTop, LayoutUnit fixedOffset,
                                                            LayoutUnit logicalHeight) const noexcept override;

    KRYS_NODISCARD Maybe<LayoutUnit> FirstLineBaseline() const noexcept override;
    KRYS_NODISCARD Maybe<LayoutUnit> LastLineBaseline() const noexcept override;

    KRYS_NODISCARD LayoutOptionalOutsets AllowedLayoutOverflow() const noexcept override;

  private:
    bool RecomputeLogicalWidthAndColumnWidth() noexcept;

    struct InlineContentStatus
    {
      bool HasSimpleOutOfFlowContentOnly {false};
      Maybe<bool> OnlyBlockContentNeedsLayout {};
    };
    void LayoutInlineContent(RelayoutChildren relayoutChildren, LayoutUnit previousHeight,
                             LayoutUnit &repaintLogicalTop, LayoutUnit &repaintLogicalBottom) noexcept;
    InlineContentStatus MarkInlineContentDirtyForLayout(RelayoutChildren relayoutChildren) noexcept;
    bool LayoutSimpleBlockContentInInline(MarginInfo &marginInfo) noexcept;
    void UpdateRepaintTopAndBottomAfterLayout(RelayoutChildren relayoutChildren,
                                              Maybe<LayoutRect> partialRepaintRect,
                                              Pair<float, float> oldContentTopAndBottomIncludingInkOverflow,
                                              LayoutUnit &repaintLogicalTop,
                                              LayoutUnit &repaintLogicalBottom) noexcept;
    bool TryComputePreferredWidthsUsingInlinePath(LayoutUnit &minLogicalWidth,
                                                  LayoutUnit &maxLogicalWidth) noexcept;
    void ComputeInlinePreferredLogicalWidths(LayoutUnit &minLogicalWidth,
                                             LayoutUnit &maxLogicalWidth) const noexcept;
    void SetStaticPositionsForSimpleOutOfFlowContent() noexcept;
    KRYS_NODISCARD Pair<float, float> InlineContentTopAndBottomIncludingInkOverflow() const noexcept;

    KRYS_NODISCARD RenderBlockFlow *
      PreviousSiblingWithOverhangingFloats(bool &parentHasFloats) const noexcept;

    void InsertFloatingBoxAndMarkForLayout(RenderBox &floatBox) noexcept;
    void RemoveFloatingBox(RenderBox &floatBox) noexcept;
    void ComputeLogicalLocationForFloat(FloatingObject &floatingObject,
                                        LayoutUnit &logicalTopOffset) noexcept;
    void AdjustInitialLetterPosition(RenderBox &childBox, LayoutUnit &logicalTopOffset,
                                     LayoutUnit &marginBeforeOffset) noexcept;

    // Called from lineWidth, to position the floats added in the last line.
    // Returns true if and only if it has positioned any floats.
    bool PositionNewFloats() noexcept;
    void ClearFloats(UsedClear usedClear) noexcept;

    KRYS_NODISCARD LayoutUnit LogicalRightOffsetForPositioningFloat(
      LayoutUnit logicalTop, LayoutUnit fixedOffset, LayoutUnit *heightRemaining) const noexcept;
    KRYS_NODISCARD LayoutUnit LogicalLeftOffsetForPositioningFloat(
      LayoutUnit logicalTop, LayoutUnit fixedOffset, LayoutUnit *heightRemaining) const noexcept;

    KRYS_NODISCARD LayoutUnit NextFloatLogicalBottomBelow(LayoutUnit logicalHeight) const noexcept;
    KRYS_NODISCARD LayoutUnit NextFloatLogicalBottomBelowForBlock(LayoutUnit logicalHeight) const noexcept;

    LayoutUnit AddOverhangingFloats(RenderBlockFlow &child, bool makeChildPaintOtherFloats) noexcept;
    KRYS_NODISCARD bool HasOverhangingFloat(RenderBox &renderer) const noexcept;
    void AddIntrudingFloats(RenderBlockFlow *previousBlock, RenderBlockFlow *container,
                            LayoutUnit logicalLeftOffset, LayoutUnit logicalTopOffset) noexcept;
    KRYS_NODISCARD bool HasOverhangingFloats() const noexcept
    {
      return Parent() && ContainsFloats() && LowestFloatLogicalBottom() > LogicalHeight();
    }
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
