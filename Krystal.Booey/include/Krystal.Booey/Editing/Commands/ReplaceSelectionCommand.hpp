/*
 * Copyright (C) 2005-2025 Apple Inc. All rights reserved.
 * Copyright (C) 2014 Google Inc. All rights reserved.
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

#include "Krystal.Booey/DOM/Nodes/DocumentFragment.hpp"
#include "Krystal.Booey/DOM/Ranges/SimpleRange.hpp"
#include "Krystal.Booey/Editing/Commands/CompositeEditCommand.hpp"
#include "Krystal.Core/Containers/EnumSet.hpp"
#include "Krystal.Core/Memory/UniquePtr.hpp"

namespace krys::boo
{
  class ReplacementFragment;

  enum class ReplaceSelectionCommandOption : uint8
  {
    SelectReplacement,
    SmartReplace,
    MatchStyle,
    PreventNesting,
    MovingParagraph,
    SanitizeFragment,
  };

  consteval ReplaceSelectionCommandOption MaxEnumValue(ReplaceSelectionCommandOption) noexcept
  {
    return ReplaceSelectionCommandOption::SanitizeFragment;
  }

  /// @brief Replaces the selection with a fragment (pasted, dropped or moved content), merging its first and
  /// last paragraphs with the paragraphs around it and removing the styles it doesn't need.
  class ReplaceSelectionCommand : public CompositeEditCommand
  {
  public:
    using CommandOption = ReplaceSelectionCommandOption;

  private:
    class InsertedNodes
    {
    private:
      RefPtr<boo::Node> _firstNodeInserted;
      RefPtr<boo::Node> _lastNodeInserted;

    public:
      void RespondToNodeInsertion(boo::Node *node) noexcept;

      void WillRemoveNodePreservingChildren(boo::Node *node) noexcept;

      void WillRemovePossibleAncestorNode(boo::Node *node) noexcept;

      void WillRemoveNode(boo::Node *node) noexcept;

      void DidReplaceNode(boo::Node *node, boo::Node *newNode) noexcept;

      KRYS_NODISCARD bool IsEmpty() const noexcept
      {
        return !_firstNodeInserted;
      }

      KRYS_NODISCARD boo::Node *FirstNodeInserted() const noexcept
      {
        return _firstNodeInserted.get();
      }

      KRYS_NODISCARD boo::Node *LastLeafInserted() const noexcept;

      KRYS_NODISCARD boo::Node *PastLastLeaf() const noexcept;
    };

    VisibleSelection _visibleSelectionForInsertedText;
    Position _startOfInsertedContent;
    Position _endOfInsertedContent;
    RefPtr<EditingStyle> _insertionStyle;
    bool _selectReplacement;
    bool _smartReplace;
    bool _matchStyle;
    const RefPtr<DocumentFragment> _documentFragment;
    UniquePtr<ReplacementFragment> _replacementFragment;
    utf8_string _documentFragmentHTMLMarkup;
    utf8_string _documentFragmentPlainText;
    bool _preventNesting;
    bool _movingParagraph;
    bool _sanitizeFragment;
    bool _shouldMergeEnd {false};

  public:
    ReplaceSelectionCommand(Document &document, RefPtr<DocumentFragment> &&fragment,
                            EnumSet<CommandOption> options, EditAction editAction) noexcept;

    ~ReplaceSelectionCommand() noexcept override;

    KRYS_NODISCARD static Ref<ReplaceSelectionCommand>
      Create(Document &document, RefPtr<DocumentFragment> &&fragment, EnumSet<CommandOption> options,
             EditAction editingAction = EditAction::Insert) noexcept;

    KRYS_NODISCARD const VisibleSelection &VisibleSelectionForInsertedText() const noexcept
    {
      return _visibleSelectionForInsertedText;
    }

    KRYS_NODISCARD const utf8_string &DocumentFragmentPlainText() const noexcept
    {
      return _documentFragmentPlainText;
    }

    KRYS_NODISCARD Maybe<dom::SimpleRange> InsertedContentRange() const noexcept;

  private:
    KRYS_NODISCARD DOMString InputEventData() const noexcept final;

    KRYS_NODISCARD RefPtr<DataTransfer> InputEventDataTransfer() const noexcept final;

    bool WillApplyCommand() noexcept final;

    void DoApply() noexcept override;

    RefPtr<boo::Node> InsertAsListItems(Element &listElement, boo::Node *insertionBlock,
                                        const Position &position, InsertedNodes &insertedNodes) noexcept;

    void UpdateNodesInserted(boo::Node *node) noexcept;

    KRYS_NODISCARD bool ShouldRemoveEndBR(boo::Node *endBR,
                                          const VisiblePosition &originalVisPosBeforeEndBR) noexcept;

    KRYS_NODISCARD bool ShouldMergeStart(bool selectionStartWasStartOfParagraph,
                                         bool fragmentHasInterchangeNewlineAtStart) noexcept;

    KRYS_NODISCARD bool ShouldMergeEnd(bool selectionEndWasEndOfParagraph) noexcept;

    KRYS_NODISCARD static bool ShouldMerge(const VisiblePosition &source,
                                           const VisiblePosition &destination) noexcept;

    void MergeEndIfNeeded() noexcept;

    void RemoveUnrenderedTextNodesAtEnds(InsertedNodes &insertedNodes) noexcept;

    void RemoveRedundantStylesAndKeepStyleSpanInline(InsertedNodes &insertedNodes) noexcept;

    void MakeInsertedContentRoundTrippableWithHTMLTreeBuilder(InsertedNodes &insertedNodes) noexcept;

    void MoveNodeOutOfAncestor(boo::Node &node, boo::Node &ancestor, InsertedNodes &insertedNodes) noexcept;

    KRYS_NODISCARD VisiblePosition PositionAtStartOfInsertedContent() const noexcept;

    KRYS_NODISCARD VisiblePosition PositionAtEndOfInsertedContent() const noexcept;

    KRYS_NODISCARD bool ShouldPerformSmartReplace() const noexcept;

    KRYS_NODISCARD bool ShouldPerformSmartParagraphReplace() const noexcept;

    void AddSpacesForSmartReplace() noexcept;

    void AddNewLinesForSmartReplace() noexcept;

    void CompleteHTMLReplacement(const Position &lastPositionToSelect) noexcept;

    void MergeTextNodesAroundPosition(Position &position, Position &positionOnlyToBeUpdated) noexcept;

    ReplacementFragment *EnsureReplacementFragment() noexcept;

    bool PerformTrivialReplace(const ReplacementFragment &fragment) noexcept;
  };
}
