#pragma once

#include "Krystal.Booey/Layout/FormattingContexts/Inline/Display/InlineDisplayContent.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineContentBreaker.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineLevelBox.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineLine.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineLineTypes.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/List.hpp"

namespace krys::boo::layout
{
  class InlineFormattingContext;
  class LineBox;
  class Rect;

  /// @brief Ruby layout inside an inline formatting context: the room annotations take on the line, ruby
  /// alignment, annotation placement and sizing, the annotations' contribution to the line box and overhang.
  class RubyFormattingContext
  {
  public:
    // Line building
    KRYS_NODISCARD static InlineLayoutUnit
      AnnotationBoxLogicalWidth(const Box &rubyBaseLayoutBox,
                                InlineFormattingContext &inlineFormattingContext) noexcept;
    KRYS_NODISCARD static InlineLayoutUnit
      BaseEndAdditionalLogicalWidth(const Box &rubyBaseLayoutBox, const Line::RunList &lineRuns,
                                    const InlineContentBreaker::ContinuousContent::RunList &candidateRuns,
                                    InlineFormattingContext &inlineFormattingContext) noexcept;
    KRYS_NODISCARD static HashMap<const Box *, InlineLayoutUnit>
      ApplyRubyAlign(Line &line, InlineFormattingContext &inlineFormattingContext) noexcept;
    KRYS_NODISCARD static InlineLayoutUnit
      ApplyRubyAlignOnAnnotationBox(Line &line, InlineLayoutUnit spaceToDistribute,
                                    InlineFormattingContext &inlineFormattingContext) noexcept;

    // Line box building
    static void ApplyAnnotationContributionToLayoutBounds(
      LineBox &lineBox, const InlineFormattingContext &inlineFormattingContext) noexcept;

    // Display content building
    KRYS_NODISCARD static InlineLayoutUnit
      BaseEndAdditionalLogicalWidth(const Box &rubyBaseLayoutBox, const inline_display::Box &baseDisplayBox,
                                    InlineLayoutUnit baseContentWidth,
                                    InlineFormattingContext &inlineFormattingContext) noexcept;
    KRYS_NODISCARD static InlineLayoutPoint
      PlaceAnnotationBox(const Box &rubyBaseLayoutBox, const Rect &rubyBaseMarginBoxLogicalRect,
                         InlineFormattingContext &inlineFormattingContext) noexcept;
    KRYS_NODISCARD static InlineLayoutSize
      SizeAnnotationBox(const Box &rubyBaseLayoutBox, const Rect &rubyBaseMarginBoxLogicalRect,
                        InlineFormattingContext &inlineFormattingContext) noexcept;

    static void ApplyRubyOverhang(InlineFormattingContext &parentFormattingContext,
                                  InlineLayoutUnit lineLogicalHeight, inline_display::Boxes &displayBoxes,
                                  const List<IndexRange> &interlinearRubyColumnRangeList) noexcept;

    enum class RubyBasesMayNeedResizing : bool
    {
      No,
      Yes
    };
    static void ApplyAlignmentOffsetList(inline_display::Boxes &displayBoxes,
                                         const HashMap<const Box *, InlineLayoutUnit> &alignmentOffsetList,
                                         RubyBasesMayNeedResizing rubyBasesMayNeedResizing,
                                         InlineFormattingContext &inlineFormattingContext) noexcept;
    static void ApplyAnnotationAlignmentOffset(inline_display::Boxes &displayBoxes,
                                               InlineLayoutUnit alignmentOffset,
                                               InlineFormattingContext &inlineFormattingContext) noexcept;

    // Miscellaneous helpers
    KRYS_NODISCARD static bool HasInterlinearAnnotation(const Box &rubyBaseLayoutBox) noexcept;
    KRYS_NODISCARD static bool HasInterCharacterAnnotation(const Box &rubyBaseLayoutBox) noexcept;

  private:
    using MaximumLayoutBoundsStretchMap = HashMap<const InlineLevelBox *, InlineLevelBox::AscentAndDescent>;
    static void AdjustLayoutBoundsAndStretchAncestorRubyBase(
      LineBox &lineBox, InlineLevelBox &rubyBaseInlineBox, MaximumLayoutBoundsStretchMap &descendantRubySet,
      const InlineFormattingContext &inlineFormattingContext) noexcept;

    KRYS_NODISCARD static size_t
      ApplyRubyAlignOnBaseContent(size_t rubyBaseStart, Line &line,
                                  HashMap<const Box *, InlineLayoutUnit> &alignmentOffsetList,
                                  InlineFormattingContext &inlineFormattingContext) noexcept;
    KRYS_NODISCARD static InlineLayoutUnit
      OverhangForAnnotationBefore(const Box &rubyBaseLayoutBox, size_t rubyBaseStart,
                                  const inline_display::Boxes &boxes, InlineLayoutUnit lineLogicalHeight,
                                  InlineFormattingContext &inlineFormattingContext) noexcept;
    KRYS_NODISCARD static InlineLayoutUnit
      OverhangForAnnotationAfter(const Box &rubyBaseLayoutBox, IndexRange rubyBaseRange,
                                 const inline_display::Boxes &boxes, InlineLayoutUnit lineLogicalHeight,
                                 InlineFormattingContext &inlineFormattingContext) noexcept;
  };
}
