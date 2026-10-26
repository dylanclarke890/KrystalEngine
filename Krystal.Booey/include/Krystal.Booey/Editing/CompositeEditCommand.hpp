#pragma once

#include "Krystal.Booey/DOM/Position.hpp"
#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/Editing/EditCommand.hpp"
#include "Krystal.Booey/Editing/UndoStep.hpp"
#include "Krystal.Booey/Editing/VisiblePosition.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/CanMakeWeakPtr.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::dom
{
  class ContainerNode;
  class Element;
  class StaticRange;
  class Text;
}

namespace krys::boo
{
  class EditingStyle;

  /// @brief A whitespace run rebalancing replaced in a text node. A no-break space is two bytes and a space
  /// one, so the run's length can change while its characters stay as many.
  struct WhitespaceRebalance
  {
    RefPtr<dom::Text> Node;
    uint32 Offset {0};
    utf8_string OldText;
    utf8_string NewText;

    /// @brief @p position moved across the replacement: after the run by the change in length, within it to
    /// the same character.
    KRYS_NODISCARD dom::Position Adjust(const dom::Position &position) const noexcept;
  };

  enum class ApplyStylePropertyLevel : bool
  {
    Default,
    ForceBlock
  };

  /// @brief The simple commands a top-level command made, which undo and redo it, and the selections before
  /// and after it.
  // FIXME: Accessibility's record of the text an undo or redo replaces (AccessibilityUndoReplacedText), with
  // accessibility.
  class EditCommandComposition final : public UndoStep
  {
  public:
    enum class AddToUndoStack : bool
    {
      No,
      Yes
    };

  private:
    const Ref<dom::Document> _document;
    VisibleSelection _startingSelection;
    VisibleSelection _endingSelection;
    List<RefPtr<SimpleEditCommand>> _commands;
    RefPtr<dom::Element> _startingRootEditableElement;
    RefPtr<dom::Element> _endingRootEditableElement;
    EditAction _editAction;

  public:
    EditCommandComposition(dom::Document &document, const VisibleSelection &startingSelection,
                           const VisibleSelection &endingSelection, EditAction editAction) noexcept;

    KRYS_NODISCARD static Ref<EditCommandComposition> Create(dom::Document &document,
                                                             const VisibleSelection &startingSelection,
                                                             const VisibleSelection &endingSelection,
                                                             EditAction editAction) noexcept;

    void Unapply() noexcept override;
    void Unapply(AddToUndoStack addToUndoStack) noexcept;
    void Reapply() noexcept override;
    KRYS_NODISCARD EditAction EditingAction() const noexcept override
    {
      return _editAction;
    }
    void Append(SimpleEditCommand *command) noexcept;
    KRYS_NODISCARD bool WasCreateLinkCommand() const noexcept
    {
      return _editAction == EditAction::CreateLink;
    }

    KRYS_NODISCARD const VisibleSelection &StartingSelection() const noexcept
    {
      return _startingSelection;
    }
    KRYS_NODISCARD const VisibleSelection &EndingSelection() const noexcept
    {
      return _endingSelection;
    }
    void SetStartingSelection(const VisibleSelection &selection) noexcept;
    void SetEndingSelection(const VisibleSelection &selection) noexcept;
    KRYS_NODISCARD dom::Element *StartingRootEditableElement() const noexcept
    {
      return _startingRootEditableElement.get();
    }
    KRYS_NODISCARD dom::Element *EndingRootEditableElement() const noexcept
    {
      return _endingRootEditableElement.get();
    }

  private:
    KRYS_NODISCARD utf8_string Label() const noexcept final;
    void DidRemoveFromUndoManager() noexcept final
    {
    }

    KRYS_NODISCARD bool AreRootEditabledElementsConnected() noexcept;
  };

  /// @brief An edit made of other edits, which it applies to the composition its top-level command makes.
  // FIXME: Document markers (replaceTextInNodePreservingMarkers keeping spelling and grammar markers), with
  // document markers; the input events' data transfer, with DataTransfer.
  class CompositeEditCommand : public EditCommand, public CanMakeWeakPtr<CompositeEditCommand>
  {
  protected:
    List<RefPtr<EditCommand>> _commands;

  private:
    RefPtr<EditCommandComposition> _composition;

  public:
    ~CompositeEditCommand() noexcept override;

    void Apply() noexcept;
    KRYS_NODISCARD bool IsFirstCommand(EditCommand *command) const noexcept
    {
      return !_commands.empty() && _commands.front().get() == command;
    }
    KRYS_NODISCARD EditCommandComposition *Composition() const noexcept;
    Ref<EditCommandComposition> EnsureComposition() noexcept;

    KRYS_NODISCARD virtual bool IsTypingCommand() const noexcept;
    KRYS_NODISCARD virtual bool IsDictationCommand() const noexcept
    {
      return false;
    }
    KRYS_NODISCARD virtual bool PreservesTypingStyle() const noexcept;
    KRYS_NODISCARD virtual bool ShouldRetainAutocorrectionIndicator() const noexcept;
    virtual void SetShouldRetainAutocorrectionIndicator(bool) noexcept;
    KRYS_NODISCARD virtual bool ShouldStopCaretBlinking() const noexcept
    {
      return false;
    }
    KRYS_NODISCARD virtual dom::DOMString InputEventTypeName() const noexcept;
    KRYS_NODISCARD virtual bool IsInputMethodComposing() const noexcept;
    KRYS_NODISCARD virtual dom::DOMString InputEventData() const noexcept
    {
      return {};
    }
    KRYS_NODISCARD virtual bool IsBeforeInputEventCancelable() const noexcept
    {
      return true;
    }
    KRYS_NODISCARD virtual bool ShouldDispatchInputEvents() const noexcept
    {
      return true;
    }
    KRYS_NODISCARD List<RefPtr<dom::StaticRange>> TargetRangesForBindings() const noexcept;

  protected:
    explicit CompositeEditCommand(dom::Document &document,
                                  EditAction editingAction = EditAction::Unspecified) noexcept;

    // If WillApplyCommand returns false, we won't proceed with applying the command.
    virtual bool WillApplyCommand() noexcept;
    virtual void DidApplyCommand() noexcept;

    KRYS_NODISCARD virtual List<RefPtr<dom::StaticRange>> TargetRanges() const noexcept;

    //
    // sugary-sweet convenience functions to help create and apply edit commands in composite commands
    //
    void AppendNode(dom::Node &node, dom::ContainerNode &parent) noexcept;
    void ApplyCommandToComposite(Ref<EditCommand> &&command) noexcept;
    void ApplyCommandToComposite(Ref<CompositeEditCommand> &&command,
                                 const VisibleSelection &selection) noexcept;
    void ApplyStyle(const EditingStyle *style,
                    EditAction editingAction = EditAction::ChangeAttributes) noexcept;
    void ApplyStyle(const EditingStyle *style, const dom::Position &start, const dom::Position &end,
                    EditAction editingAction = EditAction::ChangeAttributes,
                    ApplyStylePropertyLevel level = ApplyStylePropertyLevel::Default) noexcept;
    void ApplyStyledElement(dom::Element &element) noexcept;
    void RemoveStyledElement(dom::Element &element) noexcept;
    void DeleteSelection(bool smartDelete = false, bool mergeBlocksAfterDelete = true, bool replace = false,
                         bool expandForSpecialElements = true, bool sanitizeMarkup = true) noexcept;
    void DeleteSelection(const VisibleSelection &selection, bool smartDelete = false,
                         bool mergeBlocksAfterDelete = true, bool replace = false,
                         bool expandForSpecialElements = true, bool sanitizeMarkup = true) noexcept;
    virtual void DeleteTextFromNode(dom::Text &node, uint32 offset, uint32 count) noexcept;
    void InputText(utf8_stringview text, bool selectInsertedText = false) noexcept;
    KRYS_NODISCARD bool IsRemovableBlock(const dom::Node *node) noexcept;
    void InsertNodeAfter(dom::Node &insertChild, dom::Node &refChild) noexcept;
    void InsertNodeAt(dom::Node &insertChild, const dom::Position &editingPosition) noexcept;
    void InsertNodeAtTabSpanPosition(dom::Node &node, const dom::Position &position) noexcept;
    bool InsertNodeBefore(dom::Node &insertChild, dom::Node &refChild,
                          ShouldAssumeContentIsAlwaysEditable shouldAssumeContentIsAlwaysEditable =
                            DoNotAssumeContentIsAlwaysEditable) noexcept;
    void InsertParagraphSeparatorAtPosition(const dom::Position &position,
                                            bool useDefaultParagraphElement = false,
                                            bool pasteBlockquoteIntoUnquotedArea = false) noexcept;
    void InsertParagraphSeparator(bool useDefaultParagraphElement = false,
                                  bool pasteBlockquoteIntoUnquotedArea = false) noexcept;
    void InsertLineBreak() noexcept;
    void InsertTextIntoNode(dom::Text &node, uint32 offset, utf8_stringview text,
                            AllowPasswordEcho allowPasswordEcho = AllowPasswordEcho::Yes) noexcept;
    void MergeIdenticalElements(dom::Element &first, dom::Element &second) noexcept;
    void RebalanceWhitespace() noexcept;
    /// @return The run it replaced, if any, to move positions in its text node across.
    Maybe<WhitespaceRebalance> RebalanceWhitespaceAt(const dom::Position &position) noexcept;
    /// @return The run it replaced, if any, to move positions in its text node across.
    Maybe<WhitespaceRebalance> RebalanceWhitespaceOnTextSubstring(dom::Text &textNode, int32 startOffset,
                                                                  int32 endOffset) noexcept;
    void PrepareWhitespaceAtPositionForSplit(dom::Position &position) noexcept;
    void ReplaceCollapsibleWhitespaceWithNonBreakingSpaceIfNeeded(
      const VisiblePosition &visiblePosition) noexcept;
    KRYS_NODISCARD RefPtr<dom::Text> TextNodeForRebalance(const dom::Position &position) const noexcept;
    KRYS_NODISCARD bool ShouldRebalanceLeadingWhitespaceFor(utf8_stringview text) const noexcept;
    void RemoveNodeAttribute(dom::Element &element, dom::DOMStringAtom attribute) noexcept;
    void RemoveChildrenInRange(dom::Node &node, uint32 from, uint32 to) noexcept;
    virtual void RemoveNode(dom::Node &node,
                            ShouldAssumeContentIsAlwaysEditable shouldAssumeContentIsAlwaysEditable =
                              DoNotAssumeContentIsAlwaysEditable) noexcept;
    RefPtr<dom::Element>
      ReplaceElementWithSpanPreservingChildrenAndAttributes(dom::Element &element) noexcept;
    void
      RemoveNodePreservingChildren(dom::Node &node,
                                   ShouldAssumeContentIsAlwaysEditable shouldAssumeContentIsAlwaysEditable =
                                     DoNotAssumeContentIsAlwaysEditable) noexcept;
    void RemoveNodeAndPruneAncestors(dom::Node &node) noexcept;
    void MoveRemainingSiblingsToNewParent(dom::Node *node, dom::Node *pastLastNodeToMove,
                                          dom::Element &newParent) noexcept;
    void UpdatePositionForNodeRemovalPreservingChildren(dom::Position &position, dom::Node &node) noexcept;
    void Prune(dom::Node *node) noexcept;
    void ReplaceTextInNode(dom::Text &node, uint32 offset, uint32 count, utf8_stringview replacementText,
                           AllowPasswordEcho allowPasswordEcho = AllowPasswordEcho::Yes) noexcept;
    dom::Position ReplaceSelectedTextInNode(utf8_stringview text) noexcept;
    void ReplaceTextInNodePreservingMarkers(dom::Text &node, uint32 offset, uint32 count,
                                            utf8_stringview replacementText) noexcept;
    dom::Position PositionOutsideTabSpan(const dom::Position &position) noexcept;
    void SetNodeAttribute(dom::Element &element, dom::DOMStringAtom attribute,
                          Maybe<dom::DOMString> value) noexcept;
    void SplitElement(dom::Element &element, dom::Node &atChild) noexcept;
    void SplitTextNode(dom::Text &node, uint32 offset) noexcept;
    void SplitTextNodeContainingElement(dom::Text &text, uint32 offset) noexcept;
    void WrapContentsInDummySpan(dom::Element &element) noexcept;

    void DeleteInsignificantText(dom::Text &textNode, uint32 start, uint32 end) noexcept;
    void DeleteInsignificantText(const dom::Position &start, const dom::Position &end) noexcept;
    void DeleteInsignificantTextDownstream(const dom::Position &position) noexcept;

    RefPtr<dom::Element> AppendBlockPlaceholder(dom::Element &container) noexcept;
    RefPtr<dom::Node> InsertBlockPlaceholder(const dom::Position &position) noexcept;
    RefPtr<dom::Node> AddBlockPlaceholderIfNeeded(dom::Element *container) noexcept;
    void RemovePlaceholderAt(const dom::Position &position) noexcept;

    Ref<dom::Element> InsertNewDefaultParagraphElementAt(const dom::Position &position) noexcept;

    RefPtr<dom::Node> MoveParagraphContentsToNewBlockIfNecessary(const dom::Position &position) noexcept;

    void PushAnchorElementDown(dom::Element &anchorElement) noexcept;

    void MoveParagraph(const VisiblePosition &startOfParagraphToMove,
                       const VisiblePosition &endOfParagraphToMove, const VisiblePosition &destination,
                       bool preserveSelection = false, bool preserveStyle = true) noexcept;
    void MoveParagraphs(const VisiblePosition &startOfParagraphToMove,
                        const VisiblePosition &endOfParagraphToMove, const VisiblePosition &destination,
                        bool preserveSelection = false, bool preserveStyle = true) noexcept;
    void MoveParagraphWithClones(const VisiblePosition &startOfParagraphToMove,
                                 const VisiblePosition &endOfParagraphToMove, dom::Element *blockElement,
                                 dom::Node *outerNode) noexcept;
    void CloneParagraphUnderNewElement(const dom::Position &start, const dom::Position &end,
                                       dom::Node *outerNode, dom::Element *blockElement) noexcept;
    void CleanupAfterDeletion(VisiblePosition destination = VisiblePosition()) noexcept;

    KRYS_NODISCARD VisibleSelection ShouldBreakOutOfEmptyListItem() const noexcept;
    bool BreakOutOfEmptyListItem() noexcept;
    bool BreakOutOfEmptyMailBlockquotedParagraph() noexcept;

    dom::Position PositionAvoidingSpecialElementBoundary(const dom::Position &original) noexcept;

    RefPtr<dom::Node> SplitTreeToNode(dom::Node &start, dom::Node &end, bool splitAncestor = false) noexcept;

  private:
    KRYS_NODISCARD bool IsCompositeEditCommand() const noexcept override
    {
      return true;
    }
  };

  KRYS_NODISCARD Ref<dom::Element> CreateBlockPlaceholderElement(dom::Document &document) noexcept;
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::CompositeEditCommand)
  KRYS_NODISCARD static bool IsType(const krys::boo::EditCommand &command) noexcept
  {
    return command.IsCompositeEditCommand();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
