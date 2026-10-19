#pragma once

#include "Krystal.Booey/Graphics/FloatRect.hpp"
#include "Krystal.Booey/Graphics/LayoutRect.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineFormattingConstraints.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineFormattingContext.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/Invalidation/InlineDamage.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/InlineIteratorInlineBox.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/InlineIteratorLineBox.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/InlineIteratorTextBox.hpp"
#include "Krystal.Booey/Layout/Integration/LayoutIntegrationBoxGeometryUpdater.hpp"
#include "Krystal.Booey/Layout/LayoutState.hpp"
#include "Krystal.Booey/Rendering/RenderBlockFlow.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/CheckedPtr.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo
{
  class RenderBox;
  class RenderElement;
  class RenderInline;
  struct PaintInfo;
  class RenderObject;
  class RenderText;
  class RenderView;
}

namespace krys::boo::dom
{
  class Document;
}

namespace krys::boo::layout
{
  class BlockFormattingState;
  class InlineContentCache;
  class LayoutState;
}

namespace krys::boo::layout_integration
{
  class InlineContent;

  /// @brief Lays out a block container's inline content with the inline formatting context and hands the
  /// result to the render tree: the lines and boxes, the geometry of the boxes in it, its height and
  /// baselines.
  // FIXME: Hit testing, pagination and SVG text, once there are those.
  class LineLayout final : public CanMakeCheckedPtr<LineLayout>
  {
    KRYS_NON_COPY_MOVABLE(LineLayout);
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(LineLayout);

  public:
    enum class ForceFullLayout : bool
    {
      No,
      Yes
    };

  private:
    CheckedPtr<layout::ElementBox> _rootLayoutBox;
    CheckedPtr<dom::Document> _document;
    WeakPtr<layout::LayoutState> _layoutState;
    layout::BlockFormattingState &_blockFormattingState;
    layout::InlineContentCache &_inlineContentCache;
    Maybe<layout::ConstraintsForInlineContent> _inlineContentConstraints;
    UniquePtr<InlineContent> _inlineContent;
    BoxGeometryUpdater _boxGeometryUpdater;
    UniquePtr<layout::InlineDamage> _lineDamage;

  public:
    explicit LineLayout(RenderBlockFlow &flow) noexcept;
    ~LineLayout() noexcept;

    KRYS_NODISCARD static RenderBlockFlow *BlockContainer(const RenderObject &renderer) noexcept;
    KRYS_NODISCARD static LineLayout *Containing(RenderObject &renderer) noexcept;
    KRYS_NODISCARD static const LineLayout *Containing(const RenderObject &renderer) noexcept;

    KRYS_NODISCARD static bool CanUseFor(const RenderBlockFlow &flow) noexcept;
    KRYS_NODISCARD static bool CanUseForPreferredWidthComputation(const RenderBlockFlow &flow) noexcept;
    KRYS_NODISCARD static bool
      ShouldInvalidateLineLayoutAfterContentChange(const RenderBlockFlow &parent,
                                                   const RenderObject &rendererWithNewContent,
                                                   const LineLayout &lineLayout) noexcept;
    KRYS_NODISCARD static bool ShouldInvalidateLineLayoutAfterTreeMutation(const RenderBlockFlow &parent,
                                                                           const RenderObject &renderer,
                                                                           const LineLayout &lineLayout,
                                                                           bool isRemoval) noexcept;

    void UpdateFormattingContexGeometries(LayoutUnit availableLogicalWidth) noexcept;
    void UpdateOverflow() noexcept;
    static void UpdateStyle(const RenderObject &renderer) noexcept;

    // Partial invalidation.
    bool InsertedIntoTree(const RenderElement &parent, RenderObject &child) noexcept;
    bool RemovedFromTree(const RenderElement &parent, RenderObject &child) noexcept;
    bool UpdateTextContent(const RenderText &textRenderer, Maybe<size_t> offset, size_t oldLength) noexcept;
    bool RootStyleWillChange(const RenderBlockFlow &root, const RenderStyle &newStyle) noexcept;
    bool StyleWillChange(const RenderElement &renderer, const RenderStyle &newStyle,
                         style::Difference diff) noexcept;
    bool BoxContentWillChange(const RenderBox &renderer) noexcept;

    KRYS_NODISCARD std::pair<LayoutUnit, LayoutUnit> ComputeIntrinsicWidthConstraints() noexcept;

    /// @brief Paints the inline content for @p paintInfo's phase.
    /// @param layerRenderer The inline box whose layer is being painted, if any: only its content is painted.
    void Paint(PaintInfo &paintInfo, const LayoutPoint &paintOffset,
               const RenderInline *layerRenderer = nullptr) noexcept;

    Maybe<LayoutRect> Layout(RenderBlockFlow::MarginInfo &marginInfo,
                             ForceFullLayout forceFullLayout = ForceFullLayout::No) noexcept;

    void ShiftLinesBy(LayoutUnit blockShift) noexcept;

    void CollectOverflow() noexcept;
    KRYS_NODISCARD LayoutRect InkOverflowBoundingBoxRectFor(const RenderInline &renderInline) const noexcept;
    KRYS_NODISCARD List<FloatRect> CollectInlineBoxRects(const RenderInline &renderInline) const noexcept;

    KRYS_NODISCARD LayoutUnit ContentLogicalHeight() const noexcept;
    KRYS_NODISCARD Maybe<LayoutUnit> ClampedContentLogicalHeight() const noexcept;
    KRYS_NODISCARD bool HasEllipsisInBlockDirectionOnLastFormattedLine() const noexcept;

    KRYS_NODISCARD bool Contains(const RenderElement &renderer) const noexcept;

    KRYS_NODISCARD bool IsPaginated() const noexcept;
    KRYS_NODISCARD bool IsDamaged() const noexcept
    {
      return static_cast<bool>(_lineDamage);
    }
    KRYS_NODISCARD const layout::InlineDamage *Damage() const noexcept
    {
      return _lineDamage.get();
    }
    KRYS_NODISCARD bool HasDetachedContent() const noexcept
    {
      return _lineDamage && _lineDamage->HasDetachedContent();
    }
    KRYS_NODISCARD size_t LineCount() const noexcept;
    KRYS_NODISCARD bool HasContentfulInlineOrBlockLine() const noexcept;
    KRYS_NODISCARD bool HasContentfulInlineLine() const noexcept;
    KRYS_NODISCARD bool IsSelfCollapsingContent() const noexcept;
    KRYS_NODISCARD bool HasInkOverflow() const noexcept;

    KRYS_NODISCARD Maybe<LayoutUnit> FirstLineBaseline() const noexcept;
    KRYS_NODISCARD Maybe<LayoutUnit> LastLineBaseline() const noexcept;

    KRYS_NODISCARD LayoutRect FirstInlineBoxRect(const RenderInline &renderInline) const noexcept;
    KRYS_NODISCARD LayoutRect EnclosingBorderBoxRectFor(const RenderInline &renderInline) const noexcept;

    KRYS_NODISCARD inline_iterator::TextBoxIterator TextBoxesFor(const RenderText &renderText) const noexcept;
    KRYS_NODISCARD inline_iterator::LeafBoxIterator BoxFor(const RenderElement &renderElement) const noexcept;
    KRYS_NODISCARD inline_iterator::InlineBoxIterator
      FirstInlineBoxFor(const RenderInline &renderInline) const noexcept;
    KRYS_NODISCARD inline_iterator::InlineBoxIterator FirstRootInlineBox() const noexcept;
    KRYS_NODISCARD inline_iterator::LineBoxIterator FirstLineBox() const noexcept;
    KRYS_NODISCARD inline_iterator::LineBoxIterator LastLineBox() const noexcept;

    KRYS_NODISCARD const RenderBlockFlow &Flow() const noexcept;
    KRYS_NODISCARD RenderBlockFlow &Flow() noexcept;

    static void ReleaseCaches(RenderView &view) noexcept;

    /// @note This is temporary, required by partial bailout check.
    KRYS_NODISCARD bool ContentNeedsVisualReordering() const noexcept;

    KRYS_NODISCARD bool HasBlocks() const noexcept;

  private:
    KRYS_NODISCARD bool IsContentConsideredStale() const noexcept;
    void PreparePlacedFloats() noexcept;
    FloatRect ConstructContent(const layout::InlineLayoutState &inlineLayoutState,
                               layout::InlineLayoutResult &&layoutResult) noexcept;
    void UpdateRenderTreePositions(const layout::InlineLayoutState &inlineLayoutState,
                                   bool didDiscardContent) noexcept;

    InlineContent &EnsureInlineContent() noexcept;

    KRYS_NODISCARD layout::LayoutState &GetLayoutState() noexcept
    {
      return *_layoutState;
    }
    KRYS_NODISCARD const layout::LayoutState &GetLayoutState() const noexcept
    {
      return *_layoutState;
    }

    KRYS_NODISCARD const layout::ElementBox &RootLayoutBox() const noexcept
    {
      return *_rootLayoutBox;
    }
    KRYS_NODISCARD layout::ElementBox &RootLayoutBox() noexcept
    {
      return *_rootLayoutBox;
    }

    void ClearInlineContent() noexcept;
    void ReleaseCachesAndResetDamage() noexcept;

    layout::InlineDamage &EnsureLineDamage() noexcept;

    KRYS_NODISCARD LayoutUnit BaselineForLine(const inline_display::Line &line) const noexcept;
  };
}
