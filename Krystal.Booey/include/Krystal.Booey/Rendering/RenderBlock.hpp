#pragma once

#include "Krystal.Booey/Graphics/TextRun.hpp"
#include "Krystal.Booey/Rendering/RenderBox.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"
#include "Krystal.Core/Types/ListHashSet.hpp"
#include "Krystal.Core/Types/Pair.hpp"

namespace krys::boo
{
  enum class RelayoutChildren : bool
  {
    No,
    Yes
  };

  using TrackedRendererListHashSet = ListHashSet<RenderBox *>;

  class RenderText;
  class TextRun;

  /// @brief What a text run built from a style takes from it besides the visual order override.
  enum class TextRunFlag : uint8
  {
    RespectDirection = 1 << 0,
    RespectDirectionOverride = 1 << 1
  };

  /// @brief A box that contains other boxes: a block container, flex, grid or table.
  // FIXME: Floats, line boxes, continuations, fragmentation, selection and painting come with the rest of
  // layout and with painting.
  class RenderBlock : public RenderBox
  {
    friend class RenderBlockFlow;
    KRYS_NON_COPY_MOVABLE(RenderBlock);
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderBlock);

  public:
    enum ApplyLayoutDeltaMode
    {
      ApplyLayoutDelta,
      DoNotApplyLayoutDelta
    };

    ~RenderBlock() noexcept override;

    KRYS_NODISCARD bool CanHaveChildren() const noexcept override
    {
      return true;
    }

    KRYS_NODISCARD utf8_stringview RenderName() const noexcept override;

    KRYS_NODISCARD virtual bool CanDropAnonymousBlockChild() const noexcept
    {
      return true;
    }

    /// @brief The renderer whose text starts with the block's `::first-letter` (or the first letter's own
    /// renderer, once it's built), and the renderer that holds the first letter.
    KRYS_NODISCARD Pair<RenderObject *, RenderElement *>
      FirstLetterAndContainer(RenderObject *skipThisAsFirstLetter = nullptr) noexcept;

    KRYS_NODISCARD bool
      EstablishesIndependentFormattingContextIgnoringDisplayType(const RenderStyle &style) const noexcept;
    KRYS_NODISCARD bool EstablishesIndependentFormattingContext() const noexcept;
    KRYS_NODISCARD bool CreatesNewFormattingContext() const noexcept;

    KRYS_NODISCARD static TextRun
      ConstructTextRun(utf8_stringview stringView, const RenderStyle &style,
                       ExpansionBehavior expansion = ExpansionBehavior::DefaultBehavior(),
                       EnumSet<TextRunFlag> flags = {}) noexcept;
    KRYS_NODISCARD static TextRun
      ConstructTextRun(const RenderText &text, const RenderStyle &style,
                       ExpansionBehavior expansion = ExpansionBehavior::DefaultBehavior()) noexcept;
    KRYS_NODISCARD static TextRun
      ConstructTextRun(const RenderText &text, size_t offset, size_t length, const RenderStyle &style,
                       ExpansionBehavior expansion = ExpansionBehavior::DefaultBehavior()) noexcept;

    virtual void LayoutBlock(RelayoutChildren relayoutChildren, LayoutUnit pageLogicalHeight = 0_lu) noexcept;

    void AddOutOfFlowBox(RenderBox &outOfFlowBox) noexcept;
    static void RemoveOutOfFlowBox(const RenderBox &rendererToRemove) noexcept;
    enum class ContainingBlockState : bool
    {
      NewContainingBlock,
      SameContainingBlock
    };
    void RemoveOutOfFlowBoxes(
      const RenderBlock *newContainingBlockCandidate,
      ContainingBlockState containingBlockState = ContainingBlockState::SameContainingBlock) noexcept;

    KRYS_NODISCARD TrackedRendererListHashSet *OutOfFlowBoxes() const noexcept;
    KRYS_NODISCARD bool HasOutOfFlowBoxes() const noexcept
    {
      auto *renderers = OutOfFlowBoxes();
      return renderers && !renderers->isEmpty();
    }

    KRYS_NODISCARD bool IsContainingBlockAncestorFor(RenderObject &renderer) const noexcept;

    void MarkOutOfFlowBoxesForLayout() noexcept;

    void SetHasMarginBeforeQuirk(bool b) noexcept
    {
      SetRenderBlockHasMarginBeforeQuirk(b);
    }
    void SetHasMarginAfterQuirk(bool b) noexcept
    {
      SetRenderBlockHasMarginAfterQuirk(b);
    }
    void SetShouldForceRelayoutChildren(bool b) noexcept
    {
      SetRenderBlockShouldForceRelayoutChildren(b);
    }

    KRYS_NODISCARD bool HasMarginBeforeQuirk() const noexcept
    {
      return RenderBlockHasMarginBeforeQuirk();
    }
    KRYS_NODISCARD bool HasMarginAfterQuirk() const noexcept
    {
      return RenderBlockHasMarginAfterQuirk();
    }
    KRYS_NODISCARD bool HasBorderOrPaddingLogicalWidthChanged() const noexcept
    {
      return RenderBlockShouldForceRelayoutChildren();
    }

    KRYS_NODISCARD bool HasMarginBeforeQuirk(const RenderBox &child) const noexcept;
    KRYS_NODISCARD bool HasMarginAfterQuirk(const RenderBox &child) const noexcept;

    virtual void ScrollbarsChanged(bool /*horizontalScrollbarChanged*/,
                                   bool /*verticalScrollbarChanged*/) noexcept
    {
    }

    void AddPercentHeightDescendant(RenderBox &descendant) noexcept;
    static void RemovePercentHeightDescendant(RenderBox &descendant) noexcept;
    KRYS_NODISCARD TrackedRendererListHashSet *PercentHeightDescendants() const noexcept;
    KRYS_NODISCARD bool HasPercentHeightDescendants() const noexcept
    {
      auto *renderers = PercentHeightDescendants();
      return renderers && !renderers->isEmpty();
    }
    KRYS_NODISCARD static bool HasPercentHeightContainerMap() noexcept;
    static void ClearPercentHeightDescendantsFrom(RenderBox &parent) noexcept;

    // FIXME: Floats come with block layout.
    KRYS_NODISCARD virtual bool ContainsFloats() const noexcept
    {
      return false;
    }

    KRYS_NODISCARD LayoutUnit AvailableLogicalWidthForLine(LayoutUnit position,
                                                           LayoutUnit logicalHeight) const noexcept;
    KRYS_NODISCARD LayoutUnit LogicalRightOffsetForLine(LayoutUnit position,
                                                        LayoutUnit logicalHeight = 0_lu) const noexcept;
    KRYS_NODISCARD LayoutUnit LogicalLeftOffsetForLine(LayoutUnit position,
                                                       LayoutUnit logicalHeight = 0_lu) const noexcept;
    KRYS_NODISCARD LayoutUnit StartOffsetForLine(LayoutUnit position,
                                                 LayoutUnit logicalHeight) const noexcept;
    KRYS_NODISCARD LayoutUnit EndOffsetForLine(LayoutUnit position, LayoutUnit logicalHeight) const noexcept;

    // Accessors for logical width/height and margins in the containing block's block-flow direction.
    KRYS_NODISCARD LayoutUnit LogicalWidthForChild(const RenderBox &child) const noexcept
    {
      return IsHorizontalWritingMode() ? child.Width() : child.Height();
    }
    KRYS_NODISCARD LayoutUnit LogicalHeightForChild(const RenderBox &child) const noexcept
    {
      return IsHorizontalWritingMode() ? child.Height() : child.Width();
    }
    KRYS_NODISCARD LayoutUnit LogicalMarginBoxHeightForChild(const RenderBox &child) const noexcept
    {
      return IsHorizontalWritingMode() ? child.MarginBoxRect().Height() : child.MarginBoxRect().Width();
    }
    KRYS_NODISCARD LayoutSize LogicalSizeForChild(const RenderBox &child) const noexcept
    {
      return IsHorizontalWritingMode() ? child.Size() : child.Size().TransposedSize();
    }
    KRYS_NODISCARD LayoutUnit LogicalTopForChild(const RenderBox &child) const noexcept
    {
      return IsHorizontalWritingMode() ? child.Y() : child.X();
    }
    KRYS_NODISCARD LayoutUnit LogicalLeftForChild(const RenderBox &child) const noexcept
    {
      return IsHorizontalWritingMode() ? child.X() : child.Y();
    }
    void SetLogicalLeftForChild(RenderBox &child, LayoutUnit logicalLeft,
                                ApplyLayoutDeltaMode applyDelta = DoNotApplyLayoutDelta) noexcept;
    void SetLogicalTopForChild(RenderBox &child, LayoutUnit logicalTop,
                               ApplyLayoutDeltaMode applyDelta = DoNotApplyLayoutDelta) noexcept;
    KRYS_NODISCARD LayoutUnit MarginBeforeForChild(const RenderBoxModelObject &child) const noexcept
    {
      return child.MarginBefore(Style().GetWritingMode());
    }
    KRYS_NODISCARD LayoutUnit MarginAfterForChild(const RenderBoxModelObject &child) const noexcept
    {
      return child.MarginAfter(Style().GetWritingMode());
    }
    KRYS_NODISCARD LayoutUnit MarginStartForChild(const RenderBoxModelObject &child) const noexcept
    {
      return child.MarginStart(Style().GetWritingMode());
    }
    KRYS_NODISCARD LayoutUnit MarginEndForChild(const RenderBoxModelObject &child) const noexcept
    {
      return child.MarginEnd(Style().GetWritingMode());
    }
    void SetMarginStartForChild(RenderBox &child, LayoutUnit value) const noexcept
    {
      child.SetMarginStart(value, Style().GetWritingMode());
    }
    void SetMarginEndForChild(RenderBox &child, LayoutUnit value) const noexcept
    {
      child.SetMarginEnd(value, Style().GetWritingMode());
    }
    void SetMarginBeforeForChild(RenderBox &child, LayoutUnit value) const noexcept
    {
      child.SetMarginBefore(value, Style().GetWritingMode());
    }
    void SetMarginAfterForChild(RenderBox &child, LayoutUnit value) const noexcept
    {
      child.SetMarginAfter(value, Style().GetWritingMode());
    }
    void SetTrimmedMarginForChild(RenderBox &child, style::MarginTrimSide side) noexcept;
    KRYS_NODISCARD LayoutUnit CollapsedMarginBeforeForChild(const RenderBox &child) const noexcept;
    KRYS_NODISCARD LayoutUnit CollapsedMarginAfterForChild(const RenderBox &child) const noexcept;

    KRYS_NODISCARD LayoutUnit AvailableLogicalWidthForContent() const noexcept
    {
      return std::max(0_lu, LogicalRightOffsetForContent() - LogicalLeftOffsetForContent());
    }
    KRYS_NODISCARD LayoutUnit LogicalLeftOffsetForContent() const noexcept;
    KRYS_NODISCARD LayoutUnit LogicalRightOffsetForContent() const noexcept;
    KRYS_NODISCARD LayoutUnit StartOffsetForContent() const noexcept;
    KRYS_NODISCARD LayoutUnit EndOffsetForContent() const noexcept;

    KRYS_NODISCARD bool HasDefiniteLogicalHeight() const noexcept;
    KRYS_NODISCARD Maybe<LayoutUnit> AvailableLogicalHeightForPercentageComputation() const noexcept;

    void ComputeChildPreferredLogicalWidths(RenderBox &childBox, LayoutUnit &minPreferredLogicalWidth,
                                            LayoutUnit &maxPreferredLogicalWidth) const noexcept;

    KRYS_NODISCARD virtual bool HasLineIfEmpty() const noexcept;

    void UpdateDescendantTransformsAfterLayout() noexcept;

    KRYS_NODISCARD virtual bool CanPerformSimplifiedLayout() const noexcept;

    void ClearLayoutOverflow() noexcept;

    /// @brief Lays out the children that normal layout skips: a fieldset's legend.
    virtual void LayoutExcludedChildren(RelayoutChildren relayoutChildren) noexcept;
    /// @brief Computes the preferred widths of the children that normal layout skips.
    /// @return Whether there were any.
    KRYS_NODISCARD virtual bool
      ComputePreferredWidthsForExcludedChildren(LayoutUnit &minWidth, LayoutUnit &maxWidth) const noexcept;

    KRYS_NODISCARD LayoutUnit MarginIntrinsicLogicalWidthForChild(RenderBox &child) const noexcept;

    void UpdateScrollInfoAfterLayout() noexcept;

    // Delay updating scrollbars until endAndCommitUpdateScrollInfoAfterLayoutTransaction() is called. These
    // functions are used when a flexbox is laying out its descendants. If multiple calls are made to
    // beginUpdateScrollInfoAfterLayoutTransaction() then endAndCommitUpdateScrollInfoAfterLayoutTransaction()
    // will do nothing until it is called the same number of times.
    void BeginUpdateScrollInfoAfterLayoutTransaction() noexcept;
    void EndAndCommitUpdateScrollInfoAfterLayoutTransaction() noexcept;

    void RemoveFromUpdateScrollInfoAfterLayoutTransaction() noexcept;

    /// @brief The logical bottom of a flex or grid container's layout overflow: its in-flow children's margin
    /// boxes plus its after padding, and at least its client box.
    KRYS_NODISCARD static LayoutUnit LayoutOverflowLogicalBottom(const RenderBlock &renderer) noexcept;

  protected:
    RenderBlock(Type type, dom::Element &element, RenderStyle &&style, EnumSet<TypeFlag> baseTypeFlags = {},
                TypeSpecificFlags typeSpecificFlags = {}) noexcept;
    RenderBlock(Type type, dom::Document &document, RenderStyle &&style, EnumSet<TypeFlag> baseTypeFlags = {},
                TypeSpecificFlags typeSpecificFlags = {}) noexcept;

    void WillBeDestroyed() noexcept override;

    void Layout() noexcept override;

    void LayoutOutOfFlowBoxes(RelayoutChildren relayoutChildren,
                              bool fixedPositionObjectsOnly = false) noexcept;
    virtual void LayoutOutOfFlowBox(RenderBox &outOfFlowBox, RelayoutChildren relayoutChildren,
                                    bool fixedPositionObjectsOnly) noexcept;

    void MarkFixedPositionBoxForLayoutIfNeeded(RenderBox &positionedChild) noexcept;

    void StyleWillChange(style::Difference diff, const RenderStyle &newStyle) noexcept override;
    void StyleDidChange(style::Difference diff, const RenderStyle *oldStyle) noexcept override;

    bool SimplifiedLayout() noexcept;
    virtual void SimplifiedNormalFlowLayout() noexcept;

    virtual void ComputeOverflow(LayoutUnit oldClientAfterEdge,
                                 EnumSet<ComputeOverflowOptions> options = {}) noexcept;
    void AddOverflowFromOutOfFlowBoxes() noexcept;
    void AddVisualOverflowFromTheme() noexcept;

    void UpdateBlockChildDirtyBitsBeforeLayout(RelayoutChildren relayoutChildren, RenderBox &child) noexcept;

    bool RecomputeLogicalWidth() noexcept;

    KRYS_NODISCARD Maybe<LayoutUnit> FirstLineBaseline() const noexcept override;
    KRYS_NODISCARD Maybe<LayoutUnit> LastLineBaseline() const noexcept override;

    void ComputeIntrinsicLogicalWidths(LayoutUnit &minLogicalWidth,
                                       LayoutUnit &maxLogicalWidth) const noexcept override;
    void ComputePreferredLogicalWidths() noexcept override;
    void ComputeBlockPreferredLogicalWidths(LayoutUnit &minLogicalWidth,
                                            LayoutUnit &maxLogicalWidth) const noexcept;
    virtual void ComputeChildIntrinsicLogicalWidths(RenderBox &child, LayoutUnit &minPreferredLogicalWidth,
                                                    LayoutUnit &maxPreferredLogicalWidth) const noexcept;

    KRYS_NODISCARD virtual LayoutUnit LogicalRightFloatOffsetForLine(LayoutUnit, LayoutUnit fixedOffset,
                                                                     LayoutUnit) const noexcept
    {
      return fixedOffset;
    }
    KRYS_NODISCARD virtual LayoutUnit LogicalLeftFloatOffsetForLine(LayoutUnit, LayoutUnit fixedOffset,
                                                                    LayoutUnit) const noexcept
    {
      return fixedOffset;
    }

  private:
    KRYS_NODISCARD bool IsSelfCollapsingBlock() const noexcept override;
    KRYS_NODISCARD virtual bool ChildrenPreventSelfCollapsing() const noexcept;

    KRYS_NODISCARD bool ContentBoxLogicalWidthChanged(const RenderStyle &oldStyle,
                                                      const RenderStyle &newStyle) const noexcept;
    KRYS_NODISCARD bool PaddingBoxLogicaHeightChanged(const RenderStyle &oldStyle,
                                                      const RenderStyle &newStyle) const noexcept;
    KRYS_NODISCARD bool ScrollbarWidthDidChange(const RenderStyle &oldStyle, const RenderStyle &newStyle,
                                                ScrollbarOrientation orientation) const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::RenderBlock)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsRenderBlock();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
