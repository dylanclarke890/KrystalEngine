#pragma once

#include "Krystal.Booey/Graphics/FloatRect.hpp"
#include "Krystal.Booey/Graphics/LayoutRect.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/Display/InlineDisplayContent.hpp"
#include "Krystal.Booey/Rendering/RenderBlockFlow.hpp"
#include "Krystal.Booey/Rendering/SVG/SVGTextFragment.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/CheckedPtr.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/Span.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::layout
{
  class Box;
}

namespace krys::boo::layout_integration
{
  class InlineContentBuilder;
  class LineLayout;

  /// @brief The display content (lines and boxes) of a block container's inline formatting context, with its
  /// overflow and lookups from layout boxes to their display boxes.
  class InlineContent : public CanMakeWeakPtr<InlineContent>, public CanMakeCheckedPtr<InlineContent>
  {
    KRYS_NON_COPY_MOVABLE(InlineContent);
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(InlineContent);
    friend class InlineContentBuilder;
    friend class LineLayout;

    using FirstBoxIndexCache = HashMap<const layout::Box *, size_t>;
    using InlineBoxIndexCache = HashMap<const layout::Box *, List<size_t>>;

  private:
    CheckedRef<const RenderBlockFlow> _formattingContextRoot;
    inline_display::Content _displayContent;
    mutable UniquePtr<FirstBoxIndexCache> _firstBoxIndexCache;
    mutable UniquePtr<InlineBoxIndexCache> _inlineBoxIndexCache;
    FloatRect _scrollableOverflow;
    FloatRect _inkOverflow;
    float _clearGapBeforeFirstLine {0};
    float _clearGapAfterLastLine {0};
    Maybe<float> _firstLinePaginationOffset {};
    bool _hasMultilinePaintOverlap {false};
    bool _hasBlockLevelBoxes {false};
    bool _hasPaintedInlineLevelBoxes {false};

    List<List<SVGTextFragment>> _svgTextFragmentsForBoxes;

  public:
    explicit InlineContent(const RenderBlockFlow &formattingContextRoot) noexcept;

    KRYS_NODISCARD inline_display::Content &DisplayContent() noexcept
    {
      return _displayContent;
    }
    KRYS_NODISCARD const inline_display::Content &DisplayContent() const noexcept
    {
      return _displayContent;
    }

    KRYS_NODISCARD bool HasContentfulInFlowBox() const noexcept;
    KRYS_NODISCARD bool HasContentfulInlineLevelBox() const noexcept;

    KRYS_NODISCARD FloatRect ScrollableOverflow() const noexcept
    {
      return _scrollableOverflow;
    }
    KRYS_NODISCARD FloatRect InkOverflow() const noexcept
    {
      return _inkOverflow;
    }
    KRYS_NODISCARD bool HasInkOverflow() const noexcept
    {
      return InkOverflow() != ScrollableOverflow();
    }

    KRYS_NODISCARD bool IsPaginated() const noexcept
    {
      return _firstLinePaginationOffset.has_value();
    }
    KRYS_NODISCARD float FirstLinePaginationOffset() const noexcept
    {
      return _firstLinePaginationOffset.value_or(0.f);
    }
    KRYS_NODISCARD float ClearBeforeAfterGaps() const noexcept
    {
      return _clearGapBeforeFirstLine + _clearGapAfterLastLine;
    }
    KRYS_NODISCARD float ClearGapBeforeFirstLine() const noexcept
    {
      return _clearGapBeforeFirstLine;
    }

    KRYS_NODISCARD bool HasBlockLevelBoxes() const noexcept
    {
      return _hasBlockLevelBoxes;
    }
    KRYS_NODISCARD bool HasPaintedInlineLevelBoxes() const noexcept
    {
      return _hasPaintedInlineLevelBoxes;
    }

    KRYS_NODISCARD Span<const inline_display::Box> BoxesForRect(const LayoutRect &rect) const noexcept;

    KRYS_NODISCARD const inline_display::Line &LineForBox(const inline_display::Box &box) const noexcept
    {
      return DisplayContent().LineList[box.LineIndex()];
    }
    KRYS_NODISCARD size_t IndexForBox(const inline_display::Box &box) const noexcept;

    KRYS_NODISCARD const inline_display::Box *
      FirstBoxForLayoutBox(const layout::Box &layoutBox) const noexcept;
    KRYS_NODISCARD Maybe<size_t> FirstBoxIndexForLayoutBox(const layout::Box &layoutBox) const noexcept;

    /// @brief The block level box of the line, if the line is for block-in-inline.
    KRYS_NODISCARD const inline_display::Box *
      BlockLevelBoxForLine(const inline_display::Line &line) const noexcept;
    KRYS_NODISCARD bool IsInlineBoxWrapperForBlockLevelBox(const inline_display::Box &box) const noexcept;

    template <typename TFunction>
    void TraverseNonRootInlineBoxes(const layout::Box &layoutBox, TFunction &&function) const noexcept
    {
      for (auto index : NonRootInlineBoxIndexesForLayoutBox(layoutBox))
      {
        function(DisplayContent().BoxList[index]);
      }
    }

    KRYS_NODISCARD const RenderBlockFlow &FormattingContextRoot() const noexcept
    {
      return *_formattingContextRoot;
    }

    KRYS_NODISCARD const List<SVGTextFragment> &SvgTextFragments(size_t boxIndex) const noexcept;
    KRYS_NODISCARD List<List<SVGTextFragment>> &SvgTextFragmentsForBoxes() noexcept
    {
      return _svgTextFragmentsForBoxes;
    }

    void ShrinkToFit() noexcept;
    void ReleaseCaches() noexcept;

  private:
    void SetInkOverflow(const FloatRect &inkOverflow) noexcept
    {
      _inkOverflow = inkOverflow;
    }
    void SetScrollableOverflow(const FloatRect &scrollableOverflow) noexcept
    {
      _scrollableOverflow = scrollableOverflow;
    }
    void SetHasMultilinePaintOverlap() noexcept
    {
      _hasMultilinePaintOverlap = true;
    }
    void SetClearGapBeforeFirstLine(float clearGapBeforeFirstLine) noexcept
    {
      _clearGapBeforeFirstLine = clearGapBeforeFirstLine;
    }
    void SetClearGapAfterLastLine(float clearGapAfterLastLine) noexcept
    {
      _clearGapAfterLastLine = clearGapAfterLastLine;
    }
    void SetFirstLinePaginationOffset(float firstLinePaginationOffset) noexcept
    {
      _firstLinePaginationOffset = firstLinePaginationOffset;
    }
    void SetHasBlockLevelBoxes() noexcept
    {
      _hasBlockLevelBoxes = true;
    }
    void SetHasPaintedInlineLevelBoxes() noexcept
    {
      _hasPaintedInlineLevelBoxes = true;
    }

    KRYS_NODISCARD const List<size_t> &
      NonRootInlineBoxIndexesForLayoutBox(const layout::Box &layoutBox) const noexcept;
  };
}
