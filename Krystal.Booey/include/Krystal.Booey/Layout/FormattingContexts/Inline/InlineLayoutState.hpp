#pragma once

#include "Krystal.Booey/Layout/FormattingContexts/Block/BlockLayoutState.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/AvailableLineWidthOverride.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutElementBox.hpp"
#include "Krystal.Booey/Layout/LayoutUnits.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::layout
{
  /// @brief The state of an inline formatting context's layout: its parent block layout state, clearance
  /// gaps, line counts and the switches that come from outside.
  class InlineLayoutState
  {
  private:
    BlockLayoutState &_parentBlockLayoutState;
    InlineLayoutUnit _clearGapBeforeFirstLine {0.f};
    InlineLayoutUnit _clearGapAfterLastLine {0.f};
    InlineLayoutUnit _firstLineStartTrimForInitialLetter {0.f};
    Maybe<size_t> _legacyClampedLineIndex {};
    Maybe<size_t> _hyphenateLimitLines {};
    size_t _successiveHyphenatedLineCount {0};
    // Note that this does not include lines from nested blocks and it does not include lines with no content
    // either.
    size_t _lineCount {0};
    size_t _lineCountWithInlineContentIncludingNestedBlocks {0};
    // FIXME: This is required by the integaration codepath.
    HashMap<const ElementBox *, LayoutUnit> _nestedListMarkerOffsets;
    AvailableLineWidthOverride _availableLineWidthOverride;
    bool _shouldNotSynthesizeInlineBlockBaseline {false};
    bool _inStandardsMode {false};
    bool _shouldShapeTextAcrossInlineBoxes {false};

  public:
    explicit InlineLayoutState(BlockLayoutState &parentBlockLayoutState) noexcept
        : _parentBlockLayoutState(parentBlockLayoutState)
    {
    }

    void SetClearGapAfterLastLine(InlineLayoutUnit verticalGap) noexcept
    {
      krys_debug_assert(verticalGap >= 0);
      _clearGapAfterLastLine = verticalGap;
    }
    KRYS_NODISCARD InlineLayoutUnit ClearGapAfterLastLine() const noexcept
    {
      return _clearGapAfterLastLine;
    }

    void SetClearGapBeforeFirstLine(InlineLayoutUnit verticalGap) noexcept
    {
      _clearGapBeforeFirstLine = verticalGap;
    }
    KRYS_NODISCARD InlineLayoutUnit ClearGapBeforeFirstLine() const noexcept
    {
      return _clearGapBeforeFirstLine;
    }

    KRYS_NODISCARD const BlockLayoutState &ParentBlockLayoutState() const noexcept
    {
      return _parentBlockLayoutState;
    }
    KRYS_NODISCARD BlockLayoutState &ParentBlockLayoutState() noexcept
    {
      return _parentBlockLayoutState;
    }

    KRYS_NODISCARD const PlacedFloats &GetPlacedFloats() const noexcept
    {
      return _parentBlockLayoutState.GetPlacedFloats();
    }
    KRYS_NODISCARD PlacedFloats &GetPlacedFloats() noexcept
    {
      return _parentBlockLayoutState.GetPlacedFloats();
    }

    void SetAvailableLineWidthOverride(AvailableLineWidthOverride availableLineWidthOverride) noexcept
    {
      _availableLineWidthOverride = krys::move(availableLineWidthOverride);
    }
    KRYS_NODISCARD const AvailableLineWidthOverride &GetAvailableLineWidthOverride() const noexcept
    {
      return _availableLineWidthOverride;
    }

    void SetLegacyClampedLineIndex(size_t lineIndex) noexcept
    {
      _legacyClampedLineIndex = lineIndex;
    }
    KRYS_NODISCARD Maybe<size_t> LegacyClampedLineIndex() const noexcept
    {
      return _legacyClampedLineIndex;
    }

    void SetLineCount(size_t lineCount) noexcept
    {
      _lineCount = lineCount;
    }
    KRYS_NODISCARD size_t LineCount() const noexcept
    {
      return _lineCount;
    }

    void SetLineCountWithInlineContentIncludingNestedBlocks(size_t lineCount) noexcept
    {
      _lineCountWithInlineContentIncludingNestedBlocks = lineCount;
    }
    KRYS_NODISCARD size_t LineCountWithInlineContentIncludingNestedBlocks() const noexcept
    {
      return _lineCountWithInlineContentIncludingNestedBlocks;
    }

    void SetHyphenationLimitLines(size_t hyphenateLimitLines) noexcept
    {
      _hyphenateLimitLines = hyphenateLimitLines;
    }
    void IncrementSuccessiveHyphenatedLineCount() noexcept
    {
      ++_successiveHyphenatedLineCount;
    }
    void ResetSuccessiveHyphenatedLineCount() noexcept
    {
      _successiveHyphenatedLineCount = 0;
    }
    KRYS_NODISCARD bool IsHyphenationDisabled() const noexcept
    {
      return _hyphenateLimitLines && *_hyphenateLimitLines <= _successiveHyphenatedLineCount;
    }

    void SetFirstLineStartTrimForInitialLetter(InlineLayoutUnit trimmedThisMuch) noexcept
    {
      _firstLineStartTrimForInitialLetter = trimmedThisMuch;
    }
    KRYS_NODISCARD InlineLayoutUnit FirstLineStartTrimForInitialLetter() const noexcept
    {
      return _firstLineStartTrimForInitialLetter;
    }

    void SetInStandardsMode() noexcept
    {
      _inStandardsMode = true;
    }
    KRYS_NODISCARD bool InStandardsMode() const noexcept
    {
      return _inStandardsMode;
    }

    void SetShouldShapeTextAcrossInlineBoxes() noexcept
    {
      _shouldShapeTextAcrossInlineBoxes = true;
    }
    KRYS_NODISCARD bool ShouldShapeTextAcrossInlineBoxes() const noexcept
    {
      return _shouldShapeTextAcrossInlineBoxes;
    }

    // Integration codepath
    void
      SetNestedListMarkerOffsets(HashMap<const ElementBox *, LayoutUnit> &&nestedListMarkerOffsets) noexcept
    {
      _nestedListMarkerOffsets = krys::move(nestedListMarkerOffsets);
    }
    KRYS_NODISCARD LayoutUnit NestedListMarkerOffset(const ElementBox &listMarkerBox) const noexcept
    {
      auto it = _nestedListMarkerOffsets.find(&listMarkerBox);
      return it == _nestedListMarkerOffsets.end() ? LayoutUnit {} : it->second;
    }
    void SetShouldNotSynthesizeInlineBlockBaseline() noexcept
    {
      _shouldNotSynthesizeInlineBlockBaseline = true;
    }
    KRYS_NODISCARD bool ShouldNotSynthesizeInlineBlockBaseline() const noexcept
    {
      return _shouldNotSynthesizeInlineBlockBaseline;
    }
  };
}
