#pragma once

#include "Krystal.Booey/Core/LayoutUnit.hpp"
#include "Krystal.Booey/Core/WritingMode.hpp"
#include "Krystal.Booey/DOM/Position.hpp"
#include "Krystal.Booey/Editing/EditingBoundary.hpp"
#include "Krystal.Booey/Editing/VisiblePosition.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/String.hpp"

namespace krys::boo
{
  class SimplifiedBackwardsTextIterator;
  class TextIterator;

  // Text units a visible position moves by. Text offsets in the boundary searches are bytes of UTF-8 text.
  // FIXME: The units of a granularity in a selection direction (atBoundaryOfGranularity,
  // withinTextUnitOfGranularity, positionOfNextBoundaryOfGranularity, enclosingTextUnitOfGranularity) and the
  // ranges they make, once there's a selection.

  enum class WordSide : bool
  {
    RightWordIfOnBoundary,
    LeftWordIfOnBoundary
  };

  // words
  KRYS_NODISCARD VisiblePosition StartOfWord(const VisiblePosition &position,
                                             WordSide side = WordSide::RightWordIfOnBoundary) noexcept;
  KRYS_NODISCARD VisiblePosition EndOfWord(const VisiblePosition &position,
                                           WordSide side = WordSide::RightWordIfOnBoundary) noexcept;
  KRYS_NODISCARD VisiblePosition PreviousWordPosition(const VisiblePosition &position) noexcept;
  KRYS_NODISCARD VisiblePosition NextWordPosition(const VisiblePosition &position) noexcept;
  KRYS_NODISCARD VisiblePosition RightWordPosition(const VisiblePosition &position,
                                                   bool skipsSpaceWhenMovingRight) noexcept;
  KRYS_NODISCARD VisiblePosition LeftWordPosition(const VisiblePosition &position,
                                                  bool skipsSpaceWhenMovingRight) noexcept;
  KRYS_NODISCARD bool IsStartOfWord(const VisiblePosition &position) noexcept;

  // sentences
  KRYS_NODISCARD VisiblePosition StartOfSentence(const VisiblePosition &position) noexcept;
  KRYS_NODISCARD VisiblePosition EndOfSentence(const VisiblePosition &position) noexcept;
  KRYS_NODISCARD VisiblePosition PreviousSentencePosition(const VisiblePosition &position) noexcept;
  KRYS_NODISCARD VisiblePosition NextSentencePosition(const VisiblePosition &position) noexcept;

  // lines
  KRYS_NODISCARD VisiblePosition StartOfLine(const VisiblePosition &position) noexcept;
  KRYS_NODISCARD VisiblePosition EndOfLine(const VisiblePosition &position) noexcept;
  KRYS_NODISCARD VisiblePosition
    PreviousLinePosition(const VisiblePosition &position, LayoutUnit lineDirectionPoint,
                         EditableType editableType = EditableType::ContentIsEditable) noexcept;
  KRYS_NODISCARD VisiblePosition
    NextLinePosition(const VisiblePosition &position, LayoutUnit lineDirectionPoint,
                     EditableType editableType = EditableType::ContentIsEditable) noexcept;
  KRYS_NODISCARD bool InSameLine(const VisiblePosition &a, const VisiblePosition &b) noexcept;
  KRYS_NODISCARD bool IsStartOfLine(const VisiblePosition &position) noexcept;
  KRYS_NODISCARD bool IsEndOfLine(const VisiblePosition &position) noexcept;
  KRYS_NODISCARD VisiblePosition LogicalStartOfLine(const VisiblePosition &position,
                                                    bool *reachedBoundary = nullptr) noexcept;
  KRYS_NODISCARD VisiblePosition LogicalEndOfLine(const VisiblePosition &position,
                                                  bool *reachedBoundary = nullptr) noexcept;
  KRYS_NODISCARD bool IsLogicalEndOfLine(const VisiblePosition &position) noexcept;
  KRYS_NODISCARD VisiblePosition LeftBoundaryOfLine(const VisiblePosition &position, TextDirection direction,
                                                    bool *reachedBoundary) noexcept;
  KRYS_NODISCARD VisiblePosition RightBoundaryOfLine(const VisiblePosition &position, TextDirection direction,
                                                     bool *reachedBoundary) noexcept;

  // paragraphs (perhaps a misnomer, can be divided by line break elements)
  KRYS_NODISCARD VisiblePosition StartOfParagraph(
    const VisiblePosition &position,
    EditingBoundaryCrossingRule rule = EditingBoundaryCrossingRule::CannotCrossEditingBoundary) noexcept;
  KRYS_NODISCARD VisiblePosition EndOfParagraph(
    const VisiblePosition &position,
    EditingBoundaryCrossingRule rule = EditingBoundaryCrossingRule::CannotCrossEditingBoundary) noexcept;
  KRYS_NODISCARD VisiblePosition StartOfNextParagraph(const VisiblePosition &position) noexcept;
  KRYS_NODISCARD VisiblePosition PreviousParagraphPosition(const VisiblePosition &position,
                                                           LayoutUnit x) noexcept;
  KRYS_NODISCARD VisiblePosition NextParagraphPosition(const VisiblePosition &position,
                                                       LayoutUnit x) noexcept;
  KRYS_NODISCARD bool IsStartOfParagraph(
    const VisiblePosition &position,
    EditingBoundaryCrossingRule rule = EditingBoundaryCrossingRule::CannotCrossEditingBoundary) noexcept;
  KRYS_NODISCARD bool IsEndOfParagraph(
    const VisiblePosition &position,
    EditingBoundaryCrossingRule rule = EditingBoundaryCrossingRule::CannotCrossEditingBoundary) noexcept;
  KRYS_NODISCARD bool InSameParagraph(
    const VisiblePosition &a, const VisiblePosition &b,
    EditingBoundaryCrossingRule rule = EditingBoundaryCrossingRule::CannotCrossEditingBoundary) noexcept;
  KRYS_NODISCARD bool IsBlankParagraph(const VisiblePosition &position) noexcept;

  // blocks (true paragraphs; line break elements don't break blocks)
  KRYS_NODISCARD VisiblePosition StartOfBlock(
    const VisiblePosition &position,
    EditingBoundaryCrossingRule rule = EditingBoundaryCrossingRule::CannotCrossEditingBoundary) noexcept;
  KRYS_NODISCARD VisiblePosition EndOfBlock(
    const VisiblePosition &position,
    EditingBoundaryCrossingRule rule = EditingBoundaryCrossingRule::CannotCrossEditingBoundary) noexcept;
  KRYS_NODISCARD bool InSameBlock(const VisiblePosition &a, const VisiblePosition &b) noexcept;
  KRYS_NODISCARD bool IsStartOfBlock(const VisiblePosition &position) noexcept;
  KRYS_NODISCARD bool IsEndOfBlock(const VisiblePosition &position) noexcept;

  // document
  KRYS_NODISCARD VisiblePosition StartOfDocument(const dom::Node *node) noexcept;
  KRYS_NODISCARD VisiblePosition EndOfDocument(const dom::Node *node) noexcept;
  KRYS_NODISCARD VisiblePosition StartOfDocument(const VisiblePosition &position) noexcept;
  KRYS_NODISCARD VisiblePosition EndOfDocument(const VisiblePosition &position) noexcept;
  KRYS_NODISCARD bool IsStartOfDocument(const VisiblePosition &position) noexcept;
  KRYS_NODISCARD bool IsEndOfDocument(const VisiblePosition &position) noexcept;

  // editable content
  KRYS_NODISCARD VisiblePosition StartOfEditableContent(const VisiblePosition &position) noexcept;
  KRYS_NODISCARD VisiblePosition EndOfEditableContent(const VisiblePosition &position) noexcept;
  KRYS_NODISCARD bool IsEndOfEditableOrNonEditableContent(const VisiblePosition &position) noexcept;

  // helper function
  enum class BoundarySearchContextAvailability : bool
  {
    DontHaveMoreContext,
    MayHaveMoreContext
  };

  using BoundarySearchFunction = uint32 (*)(utf8_stringview, uint32 offset, BoundarySearchContextAvailability,
                                            bool &needMoreContext);

  KRYS_NODISCARD uint32 StartWordBoundary(utf8_stringview text, uint32 offset,
                                          BoundarySearchContextAvailability mayHaveMoreContext,
                                          bool &needMoreContext) noexcept;
  KRYS_NODISCARD uint32 EndWordBoundary(utf8_stringview text, uint32 offset,
                                        BoundarySearchContextAvailability mayHaveMoreContext,
                                        bool &needMoreContext) noexcept;
  KRYS_NODISCARD uint32 StartSentenceBoundary(utf8_stringview text, uint32 offset,
                                              BoundarySearchContextAvailability, bool &) noexcept;
  KRYS_NODISCARD uint32 EndSentenceBoundary(utf8_stringview text, uint32 offset,
                                            BoundarySearchContextAvailability, bool &) noexcept;
  KRYS_NODISCARD uint32 SuffixLengthForRange(const dom::SimpleRange &forwardsScanRange,
                                             utf8_string &string) noexcept;
  KRYS_NODISCARD uint32 PrefixLengthForRange(const dom::SimpleRange &backwardsScanRange,
                                             utf8_string &string) noexcept;
  KRYS_NODISCARD uint32 BackwardSearchForBoundaryWithTextIterator(
    SimplifiedBackwardsTextIterator &it, utf8_string &string, uint32 suffixLength,
    BoundarySearchFunction searchFunction) noexcept;
  KRYS_NODISCARD uint32
    ForwardSearchForBoundaryWithTextIterator(TextIterator &it, utf8_string &string, uint32 prefixLength,
                                             BoundarySearchFunction searchFunction) noexcept;
  KRYS_NODISCARD RefPtr<dom::Node>
    FindStartOfParagraph(dom::Node *startNode, dom::Node *highestRoot, dom::Node *startBlock, int32 &offset,
                         dom::Position::AnchorType &type,
                         EditingBoundaryCrossingRule boundaryCrossingRule) noexcept;
  KRYS_NODISCARD RefPtr<dom::Node>
    FindEndOfParagraph(dom::Node *startNode, dom::Node *highestRoot, dom::Node *stayInsideBlock,
                       int32 &offset, dom::Position::AnchorType &type,
                       EditingBoundaryCrossingRule boundaryCrossingRule) noexcept;
}
