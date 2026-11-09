/*
 * Copyright (C) 2005, 2006, 2008 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Editing/Commands/CompositeEditCommand.hpp"

namespace krys::boo
{
  /// @brief Deletes the selection, merging the paragraphs at its ends and fixing up the whitespace and the
  /// typing style left behind.
  // (There's no autocorrection, whose record of the text it replaced would be kept; see DIFFERENCES.md.)
  class DeleteSelectionCommand : public CompositeEditCommand
  {
  private:
    bool _hasSelectionToDelete;
    bool _smartDelete;
    bool _mergeBlocksAfterDelete;
    bool _needPlaceholder {false};
    bool _replace;
    bool _expandForSpecialElements;
    bool _pruneStartBlockIfNecessary {false};
    bool _startsAtEmptyLine {false};
    bool _sanitizeMarkup;

    // This data is transient and should be cleared at the end of the doApply function.
    VisibleSelection _selectionToDelete;
    dom::Position _upstreamStart;
    dom::Position _downstreamStart;
    dom::Position _upstreamEnd;
    dom::Position _downstreamEnd;
    dom::Position _endingPosition;
    dom::Position _leadingWhitespace;
    dom::Position _trailingWhitespace;
    RefPtr<dom::Node> _startBlock;
    RefPtr<dom::Node> _endBlock;
    RefPtr<EditingStyle> _typingStyle;
    RefPtr<dom::Node> _startRoot;
    RefPtr<dom::Node> _endRoot;
    RefPtr<dom::Node> _startTableRow;
    RefPtr<dom::Node> _endTableRow;
    RefPtr<dom::Node> _temporaryPlaceholder;

  public:
    DeleteSelectionCommand(dom::Document &document, bool smartDelete, bool mergeBlocksAfterDelete,
                           bool replace, bool expandForSpecialElements, bool sanitizeMarkup,
                           EditAction editingAction = EditAction::Delete) noexcept;
    DeleteSelectionCommand(const VisibleSelection &selection, bool smartDelete, bool mergeBlocksAfterDelete,
                           bool replace, bool expandForSpecialElements, bool sanitizeMarkup,
                           EditAction editingAction) noexcept;
    ~DeleteSelectionCommand() noexcept override;

    KRYS_NODISCARD static Ref<DeleteSelectionCommand>
      Create(dom::Document &document, bool smartDelete = false, bool mergeBlocksAfterDelete = true,
             bool replace = false, bool expandForSpecialElements = false, bool sanitizeMarkup = true,
             EditAction editingAction = EditAction::Delete) noexcept;
    KRYS_NODISCARD static Ref<DeleteSelectionCommand>
      Create(const VisibleSelection &selection, bool smartDelete = false, bool mergeBlocksAfterDelete = true,
             bool replace = false, bool expandForSpecialElements = false, bool sanitizeMarkup = true,
             EditAction editingAction = EditAction::Delete) noexcept;

  private:
    void DoApply() noexcept override;

    KRYS_NODISCARD bool PreservesTypingStyle() const noexcept override;
    void InitializeStartEnd(dom::Position &start, dom::Position &end) noexcept;
    void SetStartingSelectionOnSmartDelete(const dom::Position &start, const dom::Position &end) noexcept;
    bool InitializePositionData() noexcept;
    void SaveTypingStyleState() noexcept;
    bool HandleSpecialCaseBRDelete() noexcept;
    void HandleGeneralDelete() noexcept;
    void FixupWhitespace() noexcept;
    void MergeParagraphs() noexcept;
    void RemovePreviouslySelectedEmptyTableRows() noexcept;
    void CalculateTypingStyleAfterDelete() noexcept;
    void ClearTransientState() noexcept;
    void MakeStylingElementsDirectChildrenOfEditableRootToPreventStyleLoss() noexcept;
    void RemoveNode(dom::Node &node, ShouldAssumeContentIsAlwaysEditable shouldAssumeContentIsAlwaysEditable =
                                       DoNotAssumeContentIsAlwaysEditable) noexcept override;
    void DeleteTextFromNode(dom::Text &node, uint32 offset, uint32 count) noexcept override;
    void RemoveRedundantBlocks() noexcept;
    KRYS_NODISCARD bool ShouldSmartDeleteParagraphSpacers() noexcept;
    void SmartDeleteParagraphSpacers() noexcept;
    void RemoveNodeUpdatingStates(
      dom::Node &node, ShouldAssumeContentIsAlwaysEditable shouldAssumeContentIsAlwaysEditable) noexcept;
    void InsertBlockPlaceholderForTableCellIfNeeded(dom::Element &element) noexcept;
  };
}
