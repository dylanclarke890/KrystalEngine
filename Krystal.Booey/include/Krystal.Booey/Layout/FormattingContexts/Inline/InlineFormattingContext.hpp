#pragma once

#include "Krystal.Booey/Layout/Floats/FloatingContext.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/Display/InlineDisplayContent.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineFormattingConstraints.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineFormattingUtils.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineLayoutState.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineQuirks.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/LineLayoutResult.hpp"
#include "Krystal.Booey/Layout/Integration/LayoutIntegrationUtils.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::layout
{
  class AbstractLineBuilder;
  class InlineContentCache;
  class LayoutState;
  class InlineDamage;
  class LineBox;

  struct InlineLayoutResult
  {
    inline_display::Content DisplayContent;
    enum class Range : uint8
    {
      Full, // Display content represents the complete inline content -result of full layout
      // Display content represents part of the inline content starting from damaged line until the end of
      // inline content -result of partial layout with continuous damage all the way to the end of the inline
      // content
      FullFromDamage,
      // Display content represents part of the inline content starting from damaged line until damage stops
      // -result of partial layout with damage that does not cover the entire inline content
      PartialFromDamage
    };
    Range ContentRange {Range::Full};
    bool DidDiscardContent {false};
  };

  // This class implements the layout logic for inline formatting context.
  // https://www.w3.org/TR/CSS22/visuren.html#inline-formatting
  class InlineFormattingContext
  {
    KRYS_NON_COPY_MOVABLE(InlineFormattingContext);

  public:
    enum class EscapeReason : uint8
    {
      InkOverflowNeedsInitialContiningBlockForStrokeWidth
    };

  private:
    const ElementBox &_rootBlockContainer;
    LayoutState &_globalLayoutState;
    const FloatingContext _floatingContext;
    const InlineFormattingUtils _inlineFormattingUtils;
    const InlineQuirks _inlineQuirks;
    const IntegrationUtils _integrationUtils;
    InlineContentCache &_inlineContentCache;
    InlineLayoutState _inlineLayoutState;

  public:
    InlineFormattingContext(const ElementBox &formattingContextRoot, LayoutState &globalLayoutState,
                            BlockLayoutState &parentBlockLayoutState) noexcept;

    KRYS_NODISCARD InlineLayoutResult Layout(const ConstraintsForInlineContent &constraints,
                                             InlineDamage *lineDamage = nullptr) noexcept;

    KRYS_NODISCARD std::pair<LayoutUnit, LayoutUnit>
      MinimumMaximumContentSize(InlineDamage *lineDamage = nullptr) noexcept;
    KRYS_NODISCARD LayoutUnit MinimumContentSize(InlineDamage *lineDamage = nullptr) noexcept;
    KRYS_NODISCARD LayoutUnit MaximumContentSize(InlineDamage *lineDamage = nullptr) noexcept;

    KRYS_NODISCARD const ElementBox &Root() const noexcept
    {
      return _rootBlockContainer;
    }
    KRYS_NODISCARD const InlineFormattingUtils &FormattingUtils() const noexcept
    {
      return _inlineFormattingUtils;
    }
    KRYS_NODISCARD const InlineQuirks &Quirks() const noexcept
    {
      return _inlineQuirks;
    }
    KRYS_NODISCARD const FloatingContext &GetFloatingContext() const noexcept
    {
      return _floatingContext;
    }

    KRYS_NODISCARD InlineLayoutState &GetLayoutState() noexcept
    {
      return _inlineLayoutState;
    }
    KRYS_NODISCARD const InlineLayoutState &GetLayoutState() const noexcept
    {
      return _inlineLayoutState;
    }

    KRYS_NODISCARD const BoxGeometry &GeometryForBox(const Box &layoutBox,
                                                     Maybe<EscapeReason> escapeReason = null) const noexcept;
    KRYS_NODISCARD BoxGeometry &GeometryForBox(const Box &layoutBox) noexcept;

    KRYS_NODISCARD const IntegrationUtils &GetIntegrationUtils() const noexcept
    {
      return _integrationUtils;
    }

  private:
    KRYS_NODISCARD InlineLayoutResult LineLayout(AbstractLineBuilder &lineBuilder,
                                                 const InlineItemList &inlineItemList,
                                                 InlineItemRange needsLayoutRange,
                                                 Maybe<PreviousLine> previousLine,
                                                 const ConstraintsForInlineContent &constraints,
                                                 const InlineDamage *lineDamage) noexcept;
    void LayoutFloatContentOnly(const ConstraintsForInlineContent &constraints) noexcept;

    InlineRect CreateDisplayContentForInlineContent(const LineBox &lineBox,
                                                    const LineLayoutResult &lineLayoutResult,
                                                    const ConstraintsForInlineContent &constraints,
                                                    inline_display::Content &displayContent) noexcept;
    void UpdateLayoutStateWithLineLayoutResult(const LineLayoutResult &lineLayoutResult,
                                               const InlineRect &lineLogicalRect,
                                               const FloatingContext &floatingContext) noexcept;
    void UpdateBoxGeometryForPlacedFloats(const LineLayoutResult::PlacedFloatList &placedFloats) noexcept;
    void ResetBoxGeometriesForDiscardedContent(
      const InlineItemRange &discardedRange,
      const LineLayoutResult::SuspendedFloatList &suspendedFloats) noexcept;
    KRYS_NODISCARD bool
      CreateDisplayContentForLineFromCachedContent(const ConstraintsForInlineContent &constraints,
                                                   InlineLayoutResult &layoutResult) noexcept;
    KRYS_NODISCARD bool
      CreateDisplayContentForEmptyInlineContent(const ConstraintsForInlineContent &constraints,
                                                const InlineItemList &inlineItemList,
                                                InlineLayoutResult &layoutResult) noexcept;
    void InitializeInlineLayoutState(const LayoutState &globalLayoutState) noexcept;
    void RebuildInlineItemListIfNeeded(InlineDamage *lineDamage) noexcept;

    KRYS_NODISCARD InlineContentCache &GetInlineContentCache() noexcept
    {
      return _inlineContentCache;
    }
  };
}
