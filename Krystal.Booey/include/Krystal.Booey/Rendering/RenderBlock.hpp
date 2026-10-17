#pragma once

#include "Krystal.Booey/Rendering/RenderBox.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/TypeCast.hpp"
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

  /// @brief A box that contains other boxes: a block container, flex, grid or table.
  // FIXME: Layout (block layout, positioned and percent-height descendants, floats), line boxes,
  // continuations, selection and painting come with layout and painting.
  class RenderBlock : public RenderBox
  {
    KRYS_NON_COPY_MOVABLE(RenderBlock);
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderBlock);

  public:
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

    void Layout() noexcept override;

    void UpdateScrollInfoAfterLayout() noexcept;
    virtual void LayoutBlock(RelayoutChildren relayoutChildren, LayoutUnit pageLogicalHeight = 0_lu) noexcept;

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

  protected:
    RenderBlock(Type type, dom::Element &element, RenderStyle &&style, EnumSet<TypeFlag> baseTypeFlags = {},
                TypeSpecificFlags typeSpecificFlags = {}) noexcept;
    RenderBlock(Type type, dom::Document &document, RenderStyle &&style, EnumSet<TypeFlag> baseTypeFlags = {},
                TypeSpecificFlags typeSpecificFlags = {}) noexcept;

    void WillBeDestroyed() noexcept override;

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
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::RenderBlock)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsRenderBlock();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
