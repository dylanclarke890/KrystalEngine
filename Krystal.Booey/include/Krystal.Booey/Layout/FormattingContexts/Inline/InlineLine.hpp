#pragma once

#include "Krystal.Booey/Core/WritingMode.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/Display/InlineDisplayBox.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineItem.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineLineTypes.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineTextItem.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/SmallList.hpp"
#include <unicode/ubidi.h>

namespace krys::boo::layout
{
  class InlineContentAligner;
  class InlineFormattingContext;
  class InlineSoftLineBreakItem;
  enum class IntrinsicWidthMode : uint8;

  /// @brief The line being built: the runs placed on it so far, with their positions, trimmable and hanging
  /// trailing content and the line's content width.
  class Line
  {
  public:
    enum class ShapingBoundary : uint8
    {
      Start,
      Inside,
      End
    };

    struct Run
    {
      enum class Type : uint8
      {
        Text,
        NonBreakingSpace,
        WordSeparator,
        HardLineBreak,
        SoftLineBreak,
        WordBreakOpportunity,
        AtomicInlineBox,
        ListMarkerInside,
        ListMarkerOutside,
        InlineBoxStart,
        InlineBoxEnd,
        LineSpanningInlineBoxStart,
        Opaque,
        Block
      };

      struct Text
      {
        size_t Start {0};
        size_t Length {0};
        bool NeedsHyphen {false};
      };

      KRYS_NODISCARD bool IsText() const noexcept
      {
        return _type == Type::Text || IsWordSeparator() || IsNonBreakingSpace();
      }
      KRYS_NODISCARD bool IsNonBreakingSpace() const noexcept
      {
        return _type == Type::NonBreakingSpace;
      }
      KRYS_NODISCARD bool IsWordSeparator() const noexcept
      {
        return _type == Type::WordSeparator;
      }
      KRYS_NODISCARD bool IsAtomicInlineBox() const noexcept
      {
        return _type == Type::AtomicInlineBox;
      }
      KRYS_NODISCARD bool IsListMarker() const noexcept
      {
        return IsListMarkerInside() || IsListMarkerOutside();
      }
      KRYS_NODISCARD bool IsListMarkerInside() const noexcept
      {
        return _type == Type::ListMarkerInside;
      }
      KRYS_NODISCARD bool IsListMarkerOutside() const noexcept
      {
        return _type == Type::ListMarkerOutside;
      }
      KRYS_NODISCARD bool IsLineBreak() const noexcept
      {
        return IsHardLineBreak() || IsSoftLineBreak();
      }
      KRYS_NODISCARD bool IsSoftLineBreak() const noexcept
      {
        return _type == Type::SoftLineBreak;
      }
      KRYS_NODISCARD bool IsHardLineBreak() const noexcept
      {
        return _type == Type::HardLineBreak;
      }
      KRYS_NODISCARD bool IsWordBreakOpportunity() const noexcept
      {
        return _type == Type::WordBreakOpportunity;
      }
      KRYS_NODISCARD bool IsInlineBox() const noexcept
      {
        return IsInlineBoxStart() || IsLineSpanningInlineBoxStart() || IsInlineBoxEnd();
      }
      KRYS_NODISCARD bool IsInlineBoxStart() const noexcept
      {
        return _type == Type::InlineBoxStart;
      }
      KRYS_NODISCARD bool IsLineSpanningInlineBoxStart() const noexcept
      {
        return _type == Type::LineSpanningInlineBoxStart;
      }
      KRYS_NODISCARD bool IsInlineBoxEnd() const noexcept
      {
        return _type == Type::InlineBoxEnd;
      }
      KRYS_NODISCARD bool IsOpaque() const noexcept
      {
        return _type == Type::Opaque;
      }
      KRYS_NODISCARD bool IsBlock() const noexcept
      {
        return _type == Type::Block;
      }

      KRYS_NODISCARD bool IsContentful() const noexcept
      {
        return (IsText() && TextContent()->Length) || IsAtomicInlineBox() || IsLineBreak() || IsListMarker()
               || IsBlock();
      }
      KRYS_NODISCARD bool IsGenerated() const noexcept
      {
        return IsListMarker();
      }
      KRYS_NODISCARD static bool
        IsContentfulOrHasDecoration(const Run &run,
                                    const InlineFormattingContext &formattingContext) noexcept;

      KRYS_NODISCARD const Box &LayoutBox() const noexcept
      {
        return *_layoutBox;
      }
      KRYS_NODISCARD const Maybe<Text> &TextContent() const noexcept
      {
        return _textContent;
      }

      KRYS_NODISCARD InlineLayoutUnit LogicalWidth() const noexcept
      {
        return _logicalWidth;
      }
      KRYS_NODISCARD InlineLayoutUnit LogicalLeft() const noexcept
      {
        return _logicalLeft;
      }
      KRYS_NODISCARD InlineLayoutUnit LogicalRight() const noexcept
      {
        return LogicalLeft() + LogicalWidth();
      }

      KRYS_NODISCARD const inline_display::Box::Expansion &Expansion() const noexcept
      {
        return _expansion;
      }

      KRYS_NODISCARD bool HasTrailingWhitespace() const noexcept
      {
        return _trailingWhitespace.has_value();
      }
      KRYS_NODISCARD InlineLayoutUnit TrailingWhitespaceWidth() const noexcept
      {
        return _trailingWhitespace ? _trailingWhitespace->Width : 0.f;
      }
      KRYS_NODISCARD bool IsWhitespaceOnly() const noexcept
      {
        return HasTrailingWhitespace() && _trailingWhitespace->Length == _textContent->Length;
      }

      KRYS_NODISCARD TextDirection InlineDirection() const noexcept;
      KRYS_NODISCARD InlineLayoutUnit LetterSpacing() const noexcept;
      KRYS_NODISCARD bool HasTextCombine() const noexcept;
      KRYS_NODISCARD InlineLayoutUnit TextSpacingAdjustment() const noexcept
      {
        return _textSpacingAdjustment;
      }

      KRYS_NODISCARD UBiDiLevel BidiLevel() const noexcept
      {
        return _bidiLevel;
      }

      KRYS_NODISCARD bool IsShapingBoundaryStart() const noexcept
      {
        return IsShapingBoundary() && *_shapingBoundary == Line::ShapingBoundary::Start;
      }
      KRYS_NODISCARD bool IsShapingBoundaryEnd() const noexcept
      {
        return IsShapingBoundary() && *_shapingBoundary == Line::ShapingBoundary::End;
      }
      KRYS_NODISCARD bool IsInsideShapingBoundary() const noexcept
      {
        return IsShapingBoundary() && *_shapingBoundary == Line::ShapingBoundary::Inside;
      }
      KRYS_NODISCARD bool IsShapingBoundary() const noexcept
      {
        return _shapingBoundary.has_value();
      }

      // FIXME: Maybe add create functions intead?
      Run(const InlineItem &zeroWidthInlineItem, const RenderStyle &style,
          InlineLayoutUnit logicalLeft) noexcept;
      Run(const InlineItem &lineSpanningInlineBoxItem, InlineLayoutUnit logicalLeft,
          InlineLayoutUnit logicalWidth, InlineLayoutUnit textSpacingAdjustment = 0.f) noexcept;
      Run(const InlineTextItem &inlineTextItem, const RenderStyle &style, InlineLayoutUnit logicalLeft,
          InlineLayoutUnit logicalWidth, InlineLayoutUnit textSpacingAdjustment = 0.f,
          Maybe<Line::ShapingBoundary> shapingBoundary = null) noexcept;

      KRYS_NODISCARD const RenderStyle &Style() const noexcept
      {
        return *_style;
      }

    private:
      friend class Line;
      friend class InlineContentAligner;
      friend class RubyFormattingContext;

      struct TrailingWhitespace
      {
        enum class Type : uint8
        {
          NotCollapsible,
          Collapsible,
          Collapsed
        };
        Type WhitespaceType {Type::NotCollapsible};
        InlineLayoutUnit Width {0};
        size_t Length {0};
      };

      Type _type {Type::Text};
      const Box *_layoutBox {nullptr};
      const RenderStyle *_style;
      InlineLayoutUnit _logicalLeft {0};
      InlineLayoutUnit _logicalWidth {0};
      inline_display::Box::Expansion _expansion;
      UBiDiLevel _bidiLevel {UBIDI_DEFAULT_LTR};
      Maybe<TrailingWhitespace> _trailingWhitespace {};
      Maybe<size_t> _lastNonWhitespaceContentStart {};
      Maybe<Text> _textContent;
      Maybe<Line::ShapingBoundary> _shapingBoundary;
      InlineLayoutUnit _textSpacingAdjustment {0};

      Run(const InlineSoftLineBreakItem &softLineBreakItem, const RenderStyle &style,
          InlineLayoutUnit logicalLeft) noexcept;
      Run(const InlineItem &inlineItem, const RenderStyle &style, InlineLayoutUnit logicalLeft,
          InlineLayoutUnit logicalWidth, InlineLayoutUnit textSpacingAdjustment = 0.f) noexcept;

      void Expand(const InlineTextItem &inlineTextItem, InlineLayoutUnit logicalWidth) noexcept;
      void MoveHorizontally(InlineLayoutUnit offset) noexcept
      {
        _logicalLeft += offset;
      }
      void ShrinkHorizontally(InlineLayoutUnit width) noexcept
      {
        _logicalWidth -= width;
      }
      void SetExpansion(inline_display::Box::Expansion expansion) noexcept
      {
        _expansion = expansion;
      }
      void SetNeedsHyphen(InlineLayoutUnit hyphenLogicalWidth) noexcept
      {
        krys_debug_assert(_textContent);
        _textContent->NeedsHyphen = true;
        _logicalWidth += hyphenLogicalWidth;
      }
      void SetBidiLevel(UBiDiLevel bidiLevel) noexcept
      {
        _bidiLevel = bidiLevel;
      }

      KRYS_NODISCARD bool HasCollapsibleTrailingWhitespace() const noexcept
      {
        return _trailingWhitespace
               && (_trailingWhitespace->WhitespaceType == TrailingWhitespace::Type::Collapsible
                   || HasCollapsedTrailingWhitespace());
      }
      KRYS_NODISCARD bool HasCollapsedTrailingWhitespace() const noexcept
      {
        return _trailingWhitespace
               && _trailingWhitespace->WhitespaceType == TrailingWhitespace::Type::Collapsed;
      }
      KRYS_NODISCARD static Maybe<TrailingWhitespace::Type>
        TrailingWhitespaceType(const InlineTextItem &inlineTextItem) noexcept;
      InlineLayoutUnit RemoveTrailingWhitespace() noexcept;

      KRYS_NODISCARD Maybe<Run> DetachTrailingWhitespace() noexcept;

      KRYS_NODISCARD bool HasTrailingLetterSpacing() const noexcept;
      KRYS_NODISCARD InlineLayoutUnit TrailingLetterSpacing() const noexcept;
      InlineLayoutUnit RemoveTrailingLetterSpacing() noexcept;
    };
    using RunList = SmallList<Run, 10>;

    using InlineBoxListWithClonedDecorationEnd = List<const Box *>;

    struct Result
    {
      RunList Runs;
      InlineLayoutUnit ContentLogicalWidth {0.f};
      InlineLayoutUnit ContentLogicalRight {0.f};
      bool IsContentful {false};
      bool IsHangingTrailingContentWhitespace {false};
      InlineLayoutUnit HangingTrailingContentWidth {0.f};
      InlineLayoutUnit HangablePunctuationStartWidth {0.f};
      bool ContentNeedsBidiReordering {false};
      size_t NonSpanningInlineLevelBoxCount {0};
    };

    enum class TrailingContentAction : uint8
    {
      Remove,
      Preserve
    };

  private:
    struct TrimmableTrailingContent
    {
      explicit TrimmableTrailingContent(RunList &runs) noexcept : _runs(runs)
      {
      }

      void AddFullyTrimmableContent(size_t runIndex, InlineLayoutUnit trimmableContentOffset,
                                    InlineLayoutUnit trimmableWidth) noexcept;
      void AddPartiallyTrimmableContent(size_t runIndex, InlineLayoutUnit trimmableWidth) noexcept;
      InlineLayoutUnit Remove() noexcept;
      InlineLayoutUnit RemovePartiallyTrimmableContent() noexcept;

      KRYS_NODISCARD InlineLayoutUnit Width() const noexcept
      {
        return _fullyTrimmableWidth + _partiallyTrimmableWidth;
      }
      KRYS_NODISCARD bool IsEmpty() const noexcept
      {
        return !_firstTrimmableRunIndex.has_value();
      }
      KRYS_NODISCARD bool IsTrailingRunFullyTrimmable() const noexcept
      {
        return _hasFullyTrimmableContent;
      }
      KRYS_NODISCARD bool IsTrailingRunPartiallyTrimmable() const noexcept
      {
        return _partiallyTrimmableWidth != 0.f;
      }

      void Reset() noexcept
      {
        _hasFullyTrimmableContent = false;
        _firstTrimmableRunIndex = {};
        _fullyTrimmableWidth = {};
        _partiallyTrimmableWidth = {};
        _trimmableContentOffset = {};
      }

    private:
      RunList &_runs;
      Maybe<size_t> _firstTrimmableRunIndex;
      bool _hasFullyTrimmableContent {false};
      InlineLayoutUnit _trimmableContentOffset {0};
      InlineLayoutUnit _fullyTrimmableWidth {0};
      InlineLayoutUnit _partiallyTrimmableWidth {0};
    };

    struct HangingContent
    {
      void SetLeadingPunctuation(InlineLayoutUnit logicalWidth) noexcept
      {
        _leadingPunctuationWidth = logicalWidth;
      }
      void SetTrailingPunctuation(InlineLayoutUnit logicalWidth) noexcept
      {
        _trailingContent = {TrailingContent::Type::Punctuation, TrailingContent::IsConditional::No, 1,
                            logicalWidth};
      }
      void SetTrailingStopOrComma(InlineLayoutUnit logicalWidth, bool isConditional) noexcept
      {
        _trailingContent = {TrailingContent::Type::StopOrComma,
                            isConditional ? TrailingContent::IsConditional::Yes
                                          : TrailingContent::IsConditional::No,
                            1, logicalWidth};
      }
      void SetTrailingWhitespace(size_t length, InlineLayoutUnit logicalWidth) noexcept
      {
        // If white-space is set to pre-wrap, the UA must (unconditionally) hang this sequence, unless the
        // sequence is followed by a forced line break, in which case it must conditionally hang the sequence
        // is instead. Note that end of last line in a paragraph is considered a forced break.
        _trailingContent = {TrailingContent::Type::Whitespace,
                            TrailingContent::IsConditional::WhenFollowedByForcedLineBreak, length,
                            logicalWidth};
      }

      void ResetTrailingContent() noexcept
      {
        _trailingContent = {};
      }
      KRYS_NODISCARD InlineLayoutUnit TrailingWidth() const noexcept
      {
        return _trailingContent ? _trailingContent->Width : 0.f;
      }
      KRYS_NODISCARD InlineLayoutUnit TrailingWhitespaceWidth() const noexcept
      {
        return _trailingContent && _trailingContent->ContentType == TrailingContent::Type::Whitespace
                 ? _trailingContent->Width
                 : 0.f;
      }

      KRYS_NODISCARD InlineLayoutUnit LeadingPunctuationWidth() const noexcept
      {
        return _leadingPunctuationWidth;
      }
      KRYS_NODISCARD InlineLayoutUnit Width() const noexcept
      {
        return _leadingPunctuationWidth + TrailingWidth();
      }

      KRYS_NODISCARD size_t TrailingWhitespaceLength() const noexcept
      {
        return _trailingContent && _trailingContent->ContentType == TrailingContent::Type::Whitespace
                 ? _trailingContent->Length
                 : 0;
      }

      KRYS_NODISCARD bool IsTrailingContentPunctuation() const noexcept
      {
        return _trailingContent && _trailingContent->ContentType == TrailingContent::Type::Punctuation;
      }
      KRYS_NODISCARD bool IsTrailingContentConditional() const noexcept
      {
        return _trailingContent && _trailingContent->Conditional == TrailingContent::IsConditional::Yes;
      }
      KRYS_NODISCARD bool IsTrailingContentConditionalWhenFollowedByForcedLineBreak() const noexcept
      {
        return _trailingContent
               && _trailingContent->Conditional
                    == TrailingContent::IsConditional::WhenFollowedByForcedLineBreak;
      }

    private:
      InlineLayoutUnit _leadingPunctuationWidth {0.f};
      // There's either a whitespace or punctuation trailing content.
      struct TrailingContent
      {
        enum class Type : uint8
        {
          Whitespace,
          StopOrComma,
          Punctuation
        };
        Type ContentType {Type::Whitespace};

        enum class IsConditional : uint8
        {
          Yes,
          No,
          WhenFollowedByForcedLineBreak
        };
        IsConditional Conditional {IsConditional::No};
        size_t Length {0};
        InlineLayoutUnit Width {0.f};
      };
      Maybe<TrailingContent> _trailingContent {};
    };

    const InlineFormattingContext &_inlineFormattingContext;
    RunList _runs;
    TrimmableTrailingContent _trimmableTrailingContent;
    HangingContent _hangingContent;
    InlineLayoutUnit _contentLogicalWidth {0};
    size_t _nonSpanningInlineLevelBoxCount {0};
    Maybe<InlineLayoutUnit> _trailingSoftHyphenWidth {};
    InlineBoxListWithClonedDecorationEnd _inlineBoxListWithClonedDecorationEnd;
    bool _hasNonDefaultBidiLevelRun {false};
    bool _isFirstFormattedLine {false};
    bool _hasRubyContent {false};
    InlineLayoutUnit _rubyAlignContentRightOffset {0.f};
    List<InlineLayoutUnit> _inlineBoxLogicalLeftStack;

  public:
    explicit Line(const InlineFormattingContext &inlineFormattingContext) noexcept;
    KRYS_NON_COPY_MOVABLE(Line);

    void Initialize(const SmallList<InlineItem, 1> &lineSpanningInlineBoxes,
                    bool isFirstFormattedLine) noexcept;

    void AppendText(const InlineTextItem &inlineTextItem, const RenderStyle &style,
                    InlineLayoutUnit logicalWidth, Maybe<ShapingBoundary> shapingBoundary) noexcept;
    // Reserved for TextOnlySimpleLineBuilder
    void AppendTextFast(const InlineTextItem &inlineTextItem, const RenderStyle &style,
                        InlineLayoutUnit logicalWidth) noexcept;
    void AppendAtomicInlineBox(const InlineItem &inlineItem, const RenderStyle &style,
                               InlineLayoutUnit marginBoxLogicalWidth) noexcept;
    void AppendInlineBoxStart(const InlineItem &inlineItem, const RenderStyle &style,
                              InlineLayoutUnit logicalWidth, InlineLayoutUnit textSpacingAdjustment) noexcept;
    void AppendInlineBoxEnd(const InlineItem &inlineItem, const RenderStyle &style,
                            InlineLayoutUnit logicalWidth) noexcept;
    void AppendLineBreak(const InlineItem &inlineItem, const RenderStyle &style) noexcept;
    void AppendWordBreakOpportunity(const InlineItem &inlineItem, const RenderStyle &style) noexcept;
    void AppendOpaqueBox(const InlineItem &inlineItem, const RenderStyle &style) noexcept;
    void AppendBlock(const InlineItem &blockItem, InlineLayoutUnit marginBoxLogicalWidth) noexcept;

    void SetContentNeedsBidiReordering() noexcept
    {
      _hasNonDefaultBidiLevelRun = true;
    }

    KRYS_NODISCARD bool HasContent() const noexcept;
    KRYS_NODISCARD bool HasContentOrListMarker() const noexcept;
    KRYS_NODISCARD bool HasRubyContent() const noexcept
    {
      return _hasRubyContent;
    }

    KRYS_NODISCARD InlineLayoutUnit ContentLogicalWidth() const noexcept
    {
      return _contentLogicalWidth;
    }
    KRYS_NODISCARD InlineLayoutUnit ContentLogicalRight() const noexcept
    {
      return LastRunLogicalRight();
    }

    KRYS_NODISCARD bool ContentNeedsBidiReordering() const noexcept
    {
      return _hasNonDefaultBidiLevelRun;
    }
    KRYS_NODISCARD size_t NonSpanningInlineLevelBoxCount() const noexcept
    {
      return _nonSpanningInlineLevelBoxCount;
    }
    KRYS_NODISCARD InlineLayoutUnit HangingTrailingContentWidth() const noexcept
    {
      return _hangingContent.TrailingWidth();
    }
    KRYS_NODISCARD size_t HangingTrailingWhitespaceLength() const noexcept
    {
      return _hangingContent.TrailingWhitespaceLength();
    }
    KRYS_NODISCARD bool IsHangingTrailingContentWhitespace() const noexcept
    {
      return _hangingContent.TrailingWhitespaceLength() != 0;
    }

    KRYS_NODISCARD InlineLayoutUnit TrimmableTrailingWidth() const noexcept
    {
      return _trimmableTrailingContent.Width();
    }
    KRYS_NODISCARD bool IsTrailingRunFullyTrimmable() const noexcept
    {
      return _trimmableTrailingContent.IsTrailingRunFullyTrimmable();
    }

    KRYS_NODISCARD Maybe<InlineLayoutUnit> TrailingSoftHyphenWidth() const noexcept
    {
      return _trailingSoftHyphenWidth;
    }
    void AddTrailingHyphen(InlineLayoutUnit hyphenLogicalWidth) noexcept;

    InlineLayoutUnit
      HandleTrailingTrimmableContent(TrailingContentAction trailingTrimmableContentAction) noexcept;
    void HandleTrailingHangingContent(Maybe<IntrinsicWidthMode> intrinsicWidthMode,
                                      InlineLayoutUnit horizontalAvailableSpaceForContent,
                                      bool isLastFormattedLine) noexcept;
    void HandleOverflowingNonBreakingSpace(TrailingContentAction trailingContentAction,
                                           InlineLayoutUnit overflowingWidth) noexcept;
    const Box *RemoveOverflowingOutOfFlowContent() noexcept;
    void ResetBidiLevelForTrailingWhitespace(UBiDiLevel rootBidiLevel) noexcept;

    KRYS_NODISCARD const RunList &Runs() const noexcept
    {
      return _runs;
    }
    KRYS_NODISCARD RunList &Runs() noexcept
    {
      return _runs;
    }
    void InflateContentLogicalWidth(InlineLayoutUnit delta) noexcept
    {
      _contentLogicalWidth += delta;
    }
    // FIXME: This is temporary and should be removed when annotation transitions to inline box structure.
    void AdjustContentRightWithRubyAlign(InlineLayoutUnit offset) noexcept
    {
      _rubyAlignContentRightOffset = offset;
    }

    KRYS_NODISCARD const InlineBoxListWithClonedDecorationEnd &
      GetInlineBoxListWithClonedDecorationEnd() const noexcept
    {
      return _inlineBoxListWithClonedDecorationEnd;
    }

    KRYS_NODISCARD Result Close() noexcept;

    static bool RestoreTrimmedTrailingWhitespace(InlineLayoutUnit trimmedTrailingWhitespaceWidth,
                                                 RunList &runs, InlineItemRange inlineItemRange,
                                                 const InlineItemList &inlineItems) noexcept;
    KRYS_NODISCARD static bool HasTrailingForcedLineBreak(const RunList &runs) noexcept;

  private:
    KRYS_NODISCARD InlineLayoutUnit LastRunLogicalRight() const noexcept
    {
      return _runs.empty() ? 0.0f : _runs.back().LogicalRight();
    }

    void ResetTrailingContent() noexcept;

    KRYS_NODISCARD bool LineHasVisuallyNonEmptyContent() const noexcept;

    KRYS_NODISCARD bool IsFirstFormattedLine() const noexcept
    {
      return _isFirstFormattedLine;
    }
    KRYS_NODISCARD const InlineFormattingContext &GetFormattingContext() const noexcept
    {
      return _inlineFormattingContext;
    }

    static bool AppendTrailingInlineItemAsTrailingRun(RunList &runs,
                                                      InlineLayoutUnit trimmedTrailingWhitespaceWidth,
                                                      InlineItemRange inlineItemRange,
                                                      const InlineItemList &inlineItems) noexcept;
  };
}
