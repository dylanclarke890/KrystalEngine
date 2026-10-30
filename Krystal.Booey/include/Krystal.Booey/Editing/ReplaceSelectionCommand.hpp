#pragma once

#include "Krystal.Booey/DOM/DocumentFragment.hpp"
#include "Krystal.Booey/DOM/SimpleRange.hpp"
#include "Krystal.Booey/Editing/CompositeEditCommand.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

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
    IgnoreMailBlockquote,
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::ReplaceSelectionCommandOption, 7uz);

namespace krys::boo
{
  /// @brief Replaces the selection with a fragment (pasted, dropped or moved content), merging its first and
  /// last paragraphs with the paragraphs around it and removing the styles it doesn't need.
  // FIXME: The accessibility selection of the inserted text (visibleSelectionForInsertedText), with
  // accessibility.
  class ReplaceSelectionCommand : public CompositeEditCommand
  {
  public:
    using CommandOption = ReplaceSelectionCommandOption;

  private:
    class InsertedNodes
    {
    private:
      RefPtr<dom::Node> _firstNodeInserted;
      RefPtr<dom::Node> _lastNodeInserted;

    public:
      void RespondToNodeInsertion(dom::Node *node) noexcept;
      void WillRemoveNodePreservingChildren(dom::Node *node) noexcept;
      void WillRemovePossibleAncestorNode(dom::Node *node) noexcept;
      void WillRemoveNode(dom::Node *node) noexcept;
      void DidReplaceNode(dom::Node *node, dom::Node *newNode) noexcept;

      KRYS_NODISCARD bool IsEmpty() const noexcept
      {
        return !_firstNodeInserted;
      }
      KRYS_NODISCARD dom::Node *FirstNodeInserted() const noexcept
      {
        return _firstNodeInserted.get();
      }
      KRYS_NODISCARD dom::Node *LastLeafInserted() const noexcept;
      KRYS_NODISCARD dom::Node *PastLastLeaf() const noexcept;
    };

    VisibleSelection _visibleSelectionForInsertedText;
    dom::Position _startOfInsertedContent;
    dom::Position _endOfInsertedContent;
    RefPtr<EditingStyle> _insertionStyle;
    bool _selectReplacement;
    bool _smartReplace;
    bool _matchStyle;
    const RefPtr<dom::DocumentFragment> _documentFragment;
    UniquePtr<ReplacementFragment> _replacementFragment;
    utf8_string _documentFragmentHTMLMarkup;
    utf8_string _documentFragmentPlainText;
    bool _preventNesting;
    bool _movingParagraph;
    bool _sanitizeFragment;
    bool _shouldMergeEnd {false};
    bool _ignoreMailBlockquote;

  public:
    ReplaceSelectionCommand(dom::Document &document, RefPtr<dom::DocumentFragment> &&fragment,
                            EnumSet<CommandOption> options, EditAction editingAction) noexcept;
    ~ReplaceSelectionCommand() noexcept override;

    KRYS_NODISCARD static Ref<ReplaceSelectionCommand>
      Create(dom::Document &document, RefPtr<dom::DocumentFragment> &&fragment,
             EnumSet<CommandOption> options, EditAction editingAction = EditAction::Insert) noexcept;

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
    KRYS_NODISCARD dom::DOMString InputEventData() const noexcept final;
    KRYS_NODISCARD RefPtr<dom::DataTransfer> InputEventDataTransfer() const noexcept final;
    bool WillApplyCommand() noexcept final;
    void DoApply() noexcept override;

    RefPtr<dom::Node> InsertAsListItems(dom::Element &listElement, dom::Node *insertionNode,
                                        const dom::Position &position, InsertedNodes &insertedNodes) noexcept;
    void UpdateNodesInserted(dom::Node *node) noexcept;
    KRYS_NODISCARD bool ShouldRemoveEndBR(dom::Node *endBR,
                                          const VisiblePosition &originalVisPosBeforeEndBR) noexcept;

    KRYS_NODISCARD bool ShouldMergeStart(bool selectionStartWasStartOfParagraph,
                                         bool fragmentHasInterchangeNewlineAtStart,
                                         bool selectionStartWasInsideMailBlockquote) noexcept;
    KRYS_NODISCARD bool ShouldMergeEnd(bool selectionEndWasEndOfParagraph) noexcept;
    KRYS_NODISCARD bool ShouldMerge(const VisiblePosition &source,
                                    const VisiblePosition &destination) noexcept;

    void MergeEndIfNeeded() noexcept;

    void RemoveUnrenderedTextNodesAtEnds(InsertedNodes &insertedNodes) noexcept;

    void RemoveRedundantStylesAndKeepStyleSpanInline(InsertedNodes &insertedNodes) noexcept;
    void MakeInsertedContentRoundTrippableWithHTMLTreeBuilder(InsertedNodes &insertedNodes) noexcept;
    void MoveNodeOutOfAncestor(dom::Node &node, dom::Node &ancestor, InsertedNodes &insertedNodes) noexcept;
    void HandleStyleSpans(InsertedNodes &insertedNodes) noexcept;

    KRYS_NODISCARD VisiblePosition PositionAtStartOfInsertedContent() const noexcept;
    KRYS_NODISCARD VisiblePosition PositionAtEndOfInsertedContent() const noexcept;
    KRYS_NODISCARD bool ShouldPerformSmartReplace() const noexcept;
    KRYS_NODISCARD bool ShouldPerformSmartParagraphReplace() const noexcept;
    void AddSpacesForSmartReplace() noexcept;
    void AddNewLinesForSmartReplace() noexcept;
    void CompleteHTMLReplacement(const dom::Position &lastPositionToSelect) noexcept;
    void MergeTextNodesAroundPosition(dom::Position &position,
                                      dom::Position &positionOnlyToBeUpdated) noexcept;
    ReplacementFragment *EnsureReplacementFragment() noexcept;
    bool PerformTrivialReplace(const ReplacementFragment &fragment) noexcept;
  };
}
