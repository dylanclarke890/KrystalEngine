#pragma once

#include "Krystal.Booey/Core/WritingMode.hpp"
#include "Krystal.Booey/Graphics/IntSize.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/Display/InlineDisplayContent.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineFormattingConstraints.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineLineBox.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineRect.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/LineLayoutResult.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/List.hpp"

namespace krys::boo::layout
{
  class BoxGeometry;
  class ElementBox;
  class InlineFormattingContext;
  struct AncestorStack;
  struct DisplayBoxTree;
  struct IsFirstLastIndex;

  /// @brief Builds the display boxes of a line from its line layout result and line box, in visual order, and
  /// sets the geometry of the boxes on the line.
  class InlineDisplayContentBuilder
  {
  private:
    InlineFormattingContext &_formattingContext;
    const ConstraintsForInlineContent &_constraints;
    const LineBox &_lineBox;
    const inline_display::Line &_displayLine;
    IntSize _initialContaingBlockSize;
    // FIXME: This should take inline_display::Line::IsFullyTruncatedInBlockDirection() for non-prefixed
    // line-clamp.
    bool _lineIsFullyTruncatedInBlockDirection {false};
    bool _contentHasInkOverflow {false};
    bool _hasSeenTextDecoration {false};
    bool _hasSeenNestedInlineBoxesWithDifferentFontCascade {false};
    bool _hasSeenRubyBase {false};

  public:
    InlineDisplayContentBuilder(InlineFormattingContext &formattingContext,
                                const ConstraintsForInlineContent &constraints, const LineBox &lineBox,
                                const inline_display::Line &displayLine) noexcept;

    KRYS_NODISCARD inline_display::Boxes Build(const LineLayoutResult &lineLayoutResult) noexcept;

  private:
    void ProcessNonBidiContent(const LineLayoutResult &lineLayoutResult,
                               inline_display::Boxes &boxes) noexcept;
    void ProcessBidiContent(const LineLayoutResult &lineLayoutResult, inline_display::Boxes &boxes) noexcept;
    void CollectInkOverflowForInlineBoxes(inline_display::Boxes &boxes) noexcept;
    void CollectInkOverflowForTextDecorations(inline_display::Boxes &boxes) noexcept;
    void AppendTextDisplayBox(const Line::Run &lineRun, const InlineRect &textRunRect,
                              inline_display::Boxes &boxes) noexcept;
    void AppendSoftLineBreakDisplayBox(const Line::Run &lineRun, const InlineRect &softLineBreakRunRect,
                                       inline_display::Boxes &boxes) noexcept;
    void AppendHardLineBreakDisplayBox(const Line::Run &lineRun, const InlineRect &lineBreakBoxRect,
                                       inline_display::Boxes &boxes) noexcept;
    void AppendAtomicInlineLevelDisplayBox(const Line::Run &lineRun, const InlineRect &borderBoxRect,
                                           inline_display::Boxes &boxes) noexcept;
    void AppendBlockLevelDisplayBox(const Line::Run &lineRun, const InlineRect &borderBoxRect,
                                    inline_display::Boxes &boxes) noexcept;
    void AppendRootInlineBoxDisplayBox(const InlineRect &rootInlineBoxVisualRect, bool lineHasContent,
                                       inline_display::Boxes &boxes) noexcept;
    void AppendInlineBoxDisplayBox(const Line::Run &lineRun, const InlineLevelBox &inlineBox,
                                   const InlineRect &inlineBoxBorderBox,
                                   inline_display::Boxes &boxes) noexcept;
    void AppendInlineDisplayBoxAtBidiBoundary(const Box &layoutBox, inline_display::Boxes &boxes) noexcept;
    void InsertRubyAnnotationBox(const Box &annotationBox, size_t insertionPosition,
                                 const InlineRect &borderBoxRect, inline_display::Boxes &boxes) noexcept;

    KRYS_NODISCARD size_t ProcessRubyBase(size_t rubyBaseStart, inline_display::Boxes &displayBoxes,
                                          List<IndexRange> &interlinearRubyColumnRangeList,
                                          List<size_t> &rubyBaseStartIndexListWithAnnotation) noexcept;
    void ProcessRubyContent(inline_display::Boxes &displayBoxes,
                            const LineLayoutResult &lineLayoutResult) noexcept;

    KRYS_NODISCARD static InlineRect MapInlineRectLogicalToVisual(const InlineRect &logicalRect,
                                                                  const InlineRect &containerLogicalRect,
                                                                  WritingMode writingMode) noexcept;
    void SetInlineBoxGeometry(const Box &inlineBox, BoxGeometry &boxGeometry, const InlineRect &logicalRect,
                              bool isFirstInlineBoxFragment) noexcept;
    void AdjustVisualGeometryForDisplayBox(
      size_t displayBoxNodeIndex, InlineLayoutUnit &contentLineRightEdge, InlineLayoutUnit lineBoxLogicalTop,
      const DisplayBoxTree &displayBoxTree, inline_display::Boxes &boxes,
      const HashMap<const Box *, IsFirstLastIndex> &isFirstLastIndexesMap) noexcept;
    KRYS_NODISCARD size_t EnsureDisplayBoxForContainer(const ElementBox &elementBox,
                                                       DisplayBoxTree &displayBoxTree,
                                                       AncestorStack &ancestorStack,
                                                       inline_display::Boxes &boxes) noexcept;

    void SetLogicalLeft(inline_display::Box &box, InlineLayoutUnit logicalLeft,
                        WritingMode writingMode) const noexcept;
    void SetGeometryForBlockLevelOutOfFlowBoxes(const List<size_t> &indexList, const Line::RunList &lineRuns,
                                                const List<int32> &visualOrderList = {}) noexcept;

    KRYS_NODISCARD bool IsLineFullyTruncatedInBlockDirection() const noexcept
    {
      return _lineIsFullyTruncatedInBlockDirection;
    }
    KRYS_NODISCARD bool IsFirstFormattedLine() const noexcept
    {
      return GetLineBox().IsFirstFormattedLine();
    }
    KRYS_NODISCARD const LineBox &GetLineBox() const noexcept
    {
      return _lineBox;
    }
    KRYS_NODISCARD size_t LineIndex() const noexcept
    {
      return GetLineBox().LineIndex();
    }
    KRYS_NODISCARD const ConstraintsForInlineContent &GetConstraints() const noexcept
    {
      return _constraints;
    }
    KRYS_NODISCARD const ElementBox &Root() const noexcept;
    KRYS_NODISCARD InlineFormattingContext &GetFormattingContext() noexcept
    {
      return _formattingContext;
    }
    KRYS_NODISCARD const InlineFormattingContext &GetFormattingContext() const noexcept
    {
      return _formattingContext;
    }
  };
}
