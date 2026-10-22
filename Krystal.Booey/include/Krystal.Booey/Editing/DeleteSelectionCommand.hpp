#pragma once

#include "Krystal.Booey/Editing/CompositeEditCommand.hpp"

namespace krys::boo
{
  /// @brief Deletes the selection, merging the paragraphs at its ends and fixing up the whitespace and the
  /// typing style left behind.
  // FIXME: Autocorrection's record of the text it replaced
  // (originalStringForAutocorrectionAtBeginningOfSelection, deletedAutocorrectionAtPosition), with document
  // markers and text checking.
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
    RefPtr<EditingStyle> _deleteIntoBlockquoteStyle;
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
