/*
 * Copyright (C) 2004 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/DOM/Ranges/Position.hpp"
#include "Krystal.Booey/Editing/Selection/EditingBoundary.hpp"
#include "Krystal.Booey/Editing/Selection/VisiblePosition.hpp"
#include "Krystal.Booey/Graphics/Geometry/LayoutUnit.hpp"
#include "Krystal.Booey/Text/WritingMode.hpp"
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
  // ranges they make, with modifying the selection.

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
