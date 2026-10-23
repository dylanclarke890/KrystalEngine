#include "Krystal.Booey/Editing/CompositeEditCommand.hpp"
#include "Krystal.Booey/DOM/Algorithms/NodeAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/DocumentFragment.hpp"
#include "Krystal.Booey/DOM/SimpleRange.hpp"
#include "Krystal.Booey/DOM/StaticRange.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/Editing/AppendNodeCommand.hpp"
#include "Krystal.Booey/Editing/ApplyStyleCommand.hpp"
#include "Krystal.Booey/Editing/BreakBlockquoteCommand.hpp"
#include "Krystal.Booey/Editing/DeleteFromTextNodeCommand.hpp"
#include "Krystal.Booey/Editing/DeleteSelectionCommand.hpp"
#include "Krystal.Booey/Editing/Editing.hpp"
#include "Krystal.Booey/Editing/EditingStyle.hpp"
#include "Krystal.Booey/Editing/Editor.hpp"
#include "Krystal.Booey/Editing/FrameSelection.hpp"
#include "Krystal.Booey/Editing/InsertIntoTextNodeCommand.hpp"
#include "Krystal.Booey/Editing/InsertLineBreakCommand.hpp"
#include "Krystal.Booey/Editing/InsertNodeBeforeCommand.hpp"
#include "Krystal.Booey/Editing/InsertParagraphSeparatorCommand.hpp"
#include "Krystal.Booey/Editing/InsertTextCommand.hpp"
#include "Krystal.Booey/Editing/Markup.hpp"
#include "Krystal.Booey/Editing/MergeIdenticalElementsCommand.hpp"
#include "Krystal.Booey/Editing/RemoveNodeCommand.hpp"
#include "Krystal.Booey/Editing/RemoveNodePreservingChildrenCommand.hpp"
#include "Krystal.Booey/Editing/ReplaceNodeWithSpanCommand.hpp"
#include "Krystal.Booey/Editing/ReplaceSelectionCommand.hpp"
#include "Krystal.Booey/Editing/SetNodeAttributeCommand.hpp"
#include "Krystal.Booey/Editing/SplitElementCommand.hpp"
#include "Krystal.Booey/Editing/SplitTextNodeCommand.hpp"
#include "Krystal.Booey/Editing/SplitTextNodeContainingElementCommand.hpp"
#include "Krystal.Booey/Editing/TextIterator.hpp"
#include "Krystal.Booey/Editing/VisibleUnits.hpp"
#include "Krystal.Booey/Editing/WrapContentsInDummySpanCommand.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementReactionQueue.hpp"
#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/InlineIteratorLogicalOrderTraversal.hpp"
#include "Krystal.Booey/Rendering/RenderBlockFlow.hpp"
#include "Krystal.Booey/Rendering/RenderText.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle.hpp"
#include <algorithm>
#include <utility>

namespace krys::boo
{
  using dom::TreeQueries;

  namespace
  {
    KRYS_NODISCARD Ref<dom::Element> CreateBreakElement(dom::Document &document) noexcept
    {
      return CreateHTMLElement(document, u8"br");
    }

    KRYS_NODISCARD bool HasOneChild(const dom::Node &node) noexcept
    {
      auto *firstChild = node.FirstChild();
      return firstChild && !firstChild->NextSibling();
    }
  }

  Ref<EditCommandComposition> EditCommandComposition::Create(dom::Document &document,
                                                             const VisibleSelection &startingSelection,
                                                             const VisibleSelection &endingSelection,
                                                             EditAction editAction) noexcept
  {
    return CreateRef<EditCommandComposition>(document, startingSelection, endingSelection, editAction);
  }

  EditCommandComposition::EditCommandComposition(dom::Document &document,
                                                 const VisibleSelection &startingSelection,
                                                 const VisibleSelection &endingSelection,
                                                 EditAction editAction) noexcept
      : _document(ShareRef(document)), _startingSelection(startingSelection),
        _endingSelection(endingSelection),
        _startingRootEditableElement(ShareRefPtr(startingSelection.RootEditableElement())),
        _endingRootEditableElement(ShareRefPtr(endingSelection.RootEditableElement())),
        _editAction(editAction)
  {
  }

  bool EditCommandComposition::AreRootEditabledElementsConnected() noexcept
  {
    if (_startingRootEditableElement && !_startingRootEditableElement->IsConnected())
    {
      return false;
    }

    if (_endingRootEditableElement && !_endingRootEditableElement->IsConnected())
    {
      return false;
    }

    return true;
  }

  // FIXME: Stopping scrolling as content size changes during the undo
  // (prohibitScrollingWhenChangingContentSize), with scrolling.
  void EditCommandComposition::Unapply(AddToUndoStack addToUndoStack) noexcept
  {
    if (!_document->Frame())
    {
      return;
    }

    if (!AreRootEditabledElementsConnected())
    {
      return;
    }

    // Changes to the document may have been made since the last editing operation that require a layout, as
    // in <rdar://problem/5658603>. Low level operations, like RemoveNodeCommand, don't require a layout
    // because the high level operations that use them perform one if one is necessary (like for the creation
    // of VisiblePositions).
    _document->UpdateLayoutIgnorePendingStylesheets();

    if (addToUndoStack == AddToUndoStack::Yes && !_document->GetEditor().WillUnapplyEditing(*this))
    {
      return;
    }

    {
      // The custom element reactions of the commands' changes run once they're all made.
      html::CEReactionsScope ceReactions;
      for (size_t i = _commands.size(); i; --i)
      {
        _commands[i - 1]->DoUnapply();
      }
    }

    if (addToUndoStack == AddToUndoStack::No)
    {
      return;
    }

    _document->GetEditor().UnappliedEditing(*this);
  }

  void EditCommandComposition::Unapply() noexcept
  {
    Unapply(AddToUndoStack::Yes);
  }

  void EditCommandComposition::Reapply() noexcept
  {
    if (!_document->Frame())
    {
      return;
    }

    if (!AreRootEditabledElementsConnected())
    {
      return;
    }

    // Changes to the document may have been made since the last editing operation that require a layout, as
    // in <rdar://problem/5658603>. Low level operations, like RemoveNodeCommand, don't require a layout
    // because the high level operations that use them perform one if one is necessary (like for the creation
    // of VisiblePositions).
    _document->UpdateLayoutIgnorePendingStylesheets();

    if (!_document->GetEditor().WillReapplyEditing(*this))
    {
      return;
    }

    {
      html::CEReactionsScope ceReactions;
      for (size_t i = 0; i < _commands.size(); ++i)
      {
        RefPtr<SimpleEditCommand> command = _commands[i];
        command->DoReapply();
      }
    }

    _document->GetEditor().ReappliedEditing(*this);
  }

  void EditCommandComposition::Append(SimpleEditCommand *command) noexcept
  {
    _commands.push_back(ShareRefPtr(command));
  }

  void EditCommandComposition::SetStartingSelection(const VisibleSelection &selection) noexcept
  {
    _startingSelection = selection;
    _startingRootEditableElement = ShareRefPtr(selection.RootEditableElement());
  }

  void EditCommandComposition::SetEndingSelection(const VisibleSelection &selection) noexcept
  {
    _endingSelection = selection;
    _endingRootEditableElement = ShareRefPtr(selection.RootEditableElement());
  }

  utf8_string EditCommandComposition::Label() const noexcept
  {
    return utf8_string(UndoRedoLabel(_editAction));
  }

  CompositeEditCommand::CompositeEditCommand(dom::Document &document, EditAction editingAction) noexcept
      : EditCommand(document, editingAction)
  {
  }

  CompositeEditCommand::~CompositeEditCommand() noexcept
  {
    krys_debug_assert(IsTopLevelCommand() || !_composition);
  }

  bool CompositeEditCommand::WillApplyCommand() noexcept
  {
    return GetDocument().GetEditor().WillApplyEditing(*this, TargetRangesForBindings());
  }

  // FIXME: Stopping scrolling as content size changes during the command
  // (prohibitScrollingWhenChangingContentSize), with scrolling; and queueing the events the command fires
  // until it's done (EventQueueScope), with a scoped event queue.
  void CompositeEditCommand::Apply() noexcept
  {
    if (!EndingSelection().IsContentRichlyEditable())
    {
      switch (EditingAction())
      {
        case EditAction::TypingDeleteSelection:
        case EditAction::TypingDeleteBackward:
        case EditAction::TypingDeleteForward:
        case EditAction::TypingDeleteWordBackward:
        case EditAction::TypingDeleteWordForward:
        case EditAction::TypingDeleteLineBackward:
        case EditAction::TypingDeleteLineForward:
        case EditAction::TypingDeletePendingComposition:
        case EditAction::TypingDeleteFinalComposition:
        case EditAction::TypingInsertText:
        case EditAction::TypingInsertLineBreak:
        case EditAction::TypingInsertParagraph:
        case EditAction::TypingInsertPendingComposition:
        case EditAction::TypingInsertFinalComposition:
        case EditAction::Paste:
        case EditAction::DeleteByDrag:
        case EditAction::SetInlineWritingDirection:
        case EditAction::SetBlockWritingDirection:
        case EditAction::Cut:
        case EditAction::Unspecified:
        case EditAction::Insert:
        case EditAction::InsertReplacement:
        case EditAction::InsertFromDrop:
        case EditAction::Delete:
        case EditAction::Dictation:                      break;
        default:                                         krys_debug_assert(false); return;
      }
    }
    (void)EnsureComposition();

    // Changes to the document may have been made since the last editing operation that require a layout, as
    // in <rdar://problem/5658603>. Low level operations, like RemoveNodeCommand, don't require a layout
    // because the high level operations that use them perform one if one is necessary (like for the creation
    // of VisiblePositions).
    Ref<dom::Document> document = ShareRef(GetDocument());
    document->UpdateLayoutIgnorePendingStylesheets();

    if (!WillApplyCommand())
    {
      return;
    }

    {
      // The custom element reactions of the command's changes run once they're all made, rather than after
      // each of the DOM methods it calls.
      html::CEReactionsScope ceReactions;
      DoApply();
    }

    DidApplyCommand();
    SetShouldRetainAutocorrectionIndicator(false);
  }

  void CompositeEditCommand::DidApplyCommand() noexcept
  {
    GetDocument().GetEditor().AppliedEditing(*this);
  }

  List<RefPtr<dom::StaticRange>> CompositeEditCommand::TargetRanges() const noexcept
  {
    krys_debug_assert(!IsEditingTextAreaOrTextInput());
    auto firstRange = GetDocument().Selection().Selection().FirstRange();
    if (!firstRange)
    {
      return {};
    }

    auto init = dom::StaticRangeInit::Create(firstRange->Start, firstRange->End);
    if (init.HasException())
    {
      return {};
    }
    return {CreateRef<dom::StaticRange>(init.ReleaseValue())};
  }

  List<RefPtr<dom::StaticRange>> CompositeEditCommand::TargetRangesForBindings() const noexcept
  {
    if (IsEditingTextAreaOrTextInput())
    {
      return {};
    }

    return TargetRanges();
  }

  EditCommandComposition *CompositeEditCommand::Composition() const noexcept
  {
    for (auto *command = this; command; command = command->Parent())
    {
      if (auto *composition = command->_composition.get())
      {
        krys_debug_assert(!command->Parent());
        return composition;
      }
    }
    return nullptr;
  }

  Ref<EditCommandComposition> CompositeEditCommand::EnsureComposition() noexcept
  {
    auto *command = this;
    while (auto *parent = command->Parent())
    {
      command = parent;
    }
    if (!command->_composition)
    {
      command->_composition = EditCommandComposition::Create(GetDocument(), StartingSelection(),
                                                             EndingSelection(), EditingAction());
    }
    return ShareRef(*command->_composition);
  }

  bool CompositeEditCommand::PreservesTypingStyle() const noexcept
  {
    return false;
  }

  bool CompositeEditCommand::IsTypingCommand() const noexcept
  {
    return false;
  }

  bool CompositeEditCommand::ShouldRetainAutocorrectionIndicator() const noexcept
  {
    return false;
  }

  void CompositeEditCommand::SetShouldRetainAutocorrectionIndicator(bool) noexcept
  {
  }

  dom::DOMString CompositeEditCommand::InputEventTypeName() const noexcept
  {
    return dom::DOMString(InputTypeNameForEditingAction(EditingAction()));
  }

  bool CompositeEditCommand::IsInputMethodComposing() const noexcept
  {
    return IsInputMethodComposingForEditingAction(EditingAction());
  }

  //
  // sugary-sweet convenience functions to help create and apply edit commands in composite commands
  //
  void CompositeEditCommand::ApplyCommandToComposite(Ref<EditCommand> &&command) noexcept
  {
    command->SetParent(this);
    command->DoApply();
    if (auto *simpleCommand = DynamicDowncast<SimpleEditCommand>(*command))
    {
      command->SetParent(nullptr);
      EnsureComposition()->Append(simpleCommand);
    }
    _commands.push_back(krys::move(command));
  }

  void CompositeEditCommand::ApplyCommandToComposite(Ref<CompositeEditCommand> &&command,
                                                     const VisibleSelection &selection) noexcept
  {
    command->SetParent(this);
    if (selection != command->EndingSelection())
    {
      command->SetStartingSelection(selection);
      command->SetEndingSelection(selection);
    }
    command->DoApply();
    _commands.push_back(krys::move(command));
  }

  void CompositeEditCommand::ApplyStyle(const EditingStyle *style, EditAction editingAction) noexcept
  {
    ApplyCommandToComposite(ApplyStyleCommand::Create(GetDocument(), style, editingAction));
  }

  void CompositeEditCommand::ApplyStyle(const EditingStyle *style, const dom::Position &start,
                                        const dom::Position &end, EditAction editingAction,
                                        ApplyStylePropertyLevel level) noexcept
  {
    ApplyCommandToComposite(
      ApplyStyleCommand::Create(GetDocument(), style, start, end, editingAction, level));
  }

  void CompositeEditCommand::ApplyStyledElement(dom::Element &element) noexcept
  {
    ApplyCommandToComposite(ApplyStyleCommand::Create(element, false));
  }

  void CompositeEditCommand::RemoveStyledElement(dom::Element &element) noexcept
  {
    ApplyCommandToComposite(ApplyStyleCommand::Create(element, true));
  }

  void CompositeEditCommand::InsertParagraphSeparator(bool useDefaultParagraphElement,
                                                      bool pasteBlockquoteIntoUnquotedArea) noexcept
  {
    ApplyCommandToComposite(InsertParagraphSeparatorCommand::Create(
      GetDocument(), useDefaultParagraphElement, pasteBlockquoteIntoUnquotedArea, EditingAction()));
  }

  void CompositeEditCommand::InsertLineBreak() noexcept
  {
    ApplyCommandToComposite(InsertLineBreakCommand::Create(GetDocument()));
  }

  bool CompositeEditCommand::IsRemovableBlock(const dom::Node *node) noexcept
  {
    krys_debug_assert(node);
    // FIXME: We should support other elements that can be removed.
    if (!IsHTMLElementOfType(node, html::TagName::div))
    {
      return false;
    }

    auto *parentNode = node->ParentNode();
    if (!parentNode || !HasOneChild(*parentNode))
    {
      return false;
    }

    if (!Downcast<dom::Element>(*node).HasAttributes())
    {
      return true;
    }

    return false;
  }

  bool CompositeEditCommand::InsertNodeBefore(
    dom::Node &insertChild, dom::Node &refChild,
    ShouldAssumeContentIsAlwaysEditable shouldAssumeContentIsAlwaysEditable) noexcept
  {
    RefPtr<dom::ContainerNode> parent = ShareRefPtr(refChild.ParentNode());
    if (!parent || (!parent->HasEditableStyle() && parent->Renderer()))
    {
      return false;
    }
    ApplyCommandToComposite(InsertNodeBeforeCommand::Create(
      insertChild, refChild, shouldAssumeContentIsAlwaysEditable, EditingAction()));
    return true;
  }

  void CompositeEditCommand::InsertNodeAfter(dom::Node &insertChild, dom::Node &refChild) noexcept
  {
    RefPtr<dom::ContainerNode> parent = ShareRefPtr(refChild.ParentNode());
    if (!parent)
    {
      return;
    }

    if (parent->LastChild() == &refChild)
    {
      AppendNode(insertChild, *parent);
    }
    else
    {
      krys_debug_assert(refChild.NextSibling());
      (void)InsertNodeBefore(insertChild, *refChild.NextSibling());
    }
  }

  void CompositeEditCommand::InsertNodeAt(dom::Node &insertChild,
                                          const dom::Position &editingPosition) noexcept
  {
    krys_debug_assert(IsEditablePosition(editingPosition));
    // For editing positions like [table, 0], insert before the table,
    // likewise for replaced elements, brs, etc.
    dom::Position p = editingPosition.ParentAnchoredEquivalent();
    RefPtr<dom::Node> refChild = ShareRefPtr(p.DeprecatedNode());
    int32 offset = p.DeprecatedEditingOffset();

    if (CanHaveChildrenForEditing(*refChild))
    {
      RefPtr<dom::Node> child = ShareRefPtr(refChild->FirstChild());
      for (int32 i = 0; child && i < offset; i++)
      {
        child = ShareRefPtr(child->NextSibling());
      }
      if (child)
      {
        (void)InsertNodeBefore(insertChild, *child);
      }
      else
      {
        AppendNode(insertChild, Downcast<dom::ContainerNode>(*refChild));
      }
    }
    else if (CaretMinOffset(*refChild) >= offset)
    {
      (void)InsertNodeBefore(insertChild, *refChild);
    }
    else if (auto *text = DynamicDowncast<dom::Text>(*refChild); text && CaretMaxOffset(*refChild) > offset)
    {
      SplitTextNode(*text, static_cast<uint32>(offset));

      // Mutation events (bug 22634) from the text node insertion may have removed the refChild
      if (!refChild->IsConnected())
      {
        return;
      }
      (void)InsertNodeBefore(insertChild, *refChild);
    }
    else
    {
      InsertNodeAfter(insertChild, *refChild);
    }
  }

  void CompositeEditCommand::AppendNode(dom::Node &node, dom::ContainerNode &parent) noexcept
  {
    ApplyCommandToComposite(AppendNodeCommand::Create(parent, node, EditingAction()));
  }

  void CompositeEditCommand::RemoveChildrenInRange(dom::Node &node, uint32 from, uint32 to) noexcept
  {
    List<Ref<dom::Node>> children;
    auto *child = node.FirstChild();
    for (uint32 i = 0; child && i < from; ++i)
    {
      child = child->NextSibling();
    }
    for (uint32 i = from; child && i < to; i++, child = child->NextSibling())
    {
      children.push_back(ShareRef(*child));
    }

    for (auto &childToRemove : children)
    {
      RemoveNode(*childToRemove);
    }
  }

  void CompositeEditCommand::RemoveNode(
    dom::Node &node, ShouldAssumeContentIsAlwaysEditable shouldAssumeContentIsAlwaysEditable) noexcept
  {
    if (!node.NonShadowBoundaryParentNode())
    {
      return;
    }
    ApplyCommandToComposite(
      RemoveNodeCommand::Create(node, shouldAssumeContentIsAlwaysEditable, EditingAction()));
  }

  void CompositeEditCommand::RemoveNodePreservingChildren(
    dom::Node &node, ShouldAssumeContentIsAlwaysEditable shouldAssumeContentIsAlwaysEditable) noexcept
  {
    ApplyCommandToComposite(RemoveNodePreservingChildrenCommand::Create(
      node, shouldAssumeContentIsAlwaysEditable, EditingAction()));
  }

  void CompositeEditCommand::RemoveNodeAndPruneAncestors(dom::Node &node) noexcept
  {
    RefPtr<dom::ContainerNode> parent = ShareRefPtr(node.ParentNode());
    RemoveNode(node);
    Prune(parent.get());
  }

  void CompositeEditCommand::MoveRemainingSiblingsToNewParent(dom::Node *node, dom::Node *pastLastNodeToMove,
                                                              dom::Element &newParent) noexcept
  {
    List<Ref<dom::Node>> nodesToRemove;
    Ref<dom::Element> protectedNewParent = ShareRef(newParent);

    for (auto *currentNode = node; currentNode && currentNode != pastLastNodeToMove;
         currentNode = currentNode->NextSibling())
    {
      nodesToRemove.push_back(ShareRef(*currentNode));
    }

    for (auto &nodeToRemove : nodesToRemove)
    {
      RemoveNode(*nodeToRemove);
      AppendNode(*nodeToRemove, newParent);
    }
  }

  void CompositeEditCommand::UpdatePositionForNodeRemovalPreservingChildren(dom::Position &position,
                                                                            dom::Node &node) noexcept
  {
    int32 offset = (position.GetAnchorType() == dom::Position::AnchorType::PositionIsOffsetInAnchor)
                     ? position.OffsetInContainerNode()
                     : 0;
    UpdatePositionForNodeRemoval(position, node);
    if (offset)
    {
      position.MoveToOffset(static_cast<uint32>(offset));
    }
  }

  RefPtr<dom::Element> CompositeEditCommand::ReplaceElementWithSpanPreservingChildrenAndAttributes(
    dom::Element &element) noexcept
  {
    // It would also be possible to implement all of ReplaceNodeWithSpanCommand
    // as a series of existing smaller edit commands.  Someone who wanted to
    // reduce the number of edit commands could do so here.
    auto command = ReplaceNodeWithSpanCommand::Create(element);
    ApplyCommandToComposite(Ref<EditCommand>(command));
    // Returning a raw pointer here is OK because the command is retained by
    // applyCommandToComposite (thus retaining the span), and the span is also
    // in the DOM tree, and thus alive whie it has a parent.
    krys_debug_assert(command->SpanElement()->IsConnected());
    return ShareRefPtr(command->SpanElement());
  }

  void CompositeEditCommand::Prune(dom::Node *node) noexcept
  {
    if (auto highestNodeToRemove = HighestNodeToRemoveInPruning(node))
    {
      RemoveNode(*highestNodeToRemove);
    }
  }

  void CompositeEditCommand::SplitTextNode(dom::Text &node, uint32 offset) noexcept
  {
    ApplyCommandToComposite(SplitTextNodeCommand::Create(node, offset));
  }

  void CompositeEditCommand::SplitElement(dom::Element &element, dom::Node &atChild) noexcept
  {
    ApplyCommandToComposite(SplitElementCommand::Create(element, atChild));
  }

  void CompositeEditCommand::MergeIdenticalElements(dom::Element &first, dom::Element &second) noexcept
  {
    Ref<dom::Element> protectedFirst = ShareRef(first);
    Ref<dom::Element> protectedSecond = ShareRef(second);
    krys_debug_assert(!first.IsDescendantOf(&second) && &second != &first);
    if (first.NextSibling() != &second)
    {
      RemoveNode(second);
      InsertNodeAfter(second, first);
    }
    ApplyCommandToComposite(MergeIdenticalElementsCommand::Create(first, second));
  }

  void CompositeEditCommand::WrapContentsInDummySpan(dom::Element &element) noexcept
  {
    ApplyCommandToComposite(WrapContentsInDummySpanCommand::Create(element));
  }

  void CompositeEditCommand::SplitTextNodeContainingElement(dom::Text &text, uint32 offset) noexcept
  {
    ApplyCommandToComposite(SplitTextNodeContainingElementCommand::Create(text, offset));
  }

  void CompositeEditCommand::InputText(utf8_stringview text, bool selectInsertedText) noexcept
  {
    size_t offset = 0;
    size_t length = text.size();

    RefPtr<dom::ContainerNode> scope;
    int32 startIndex = IndexForVisiblePosition(EndingSelection().VisibleStart(), scope);

    size_t newline;
    do
    {
      newline = text.find(u8'\n', offset);
      if (newline != offset)
      {
        size_t substringLength = newline == utf8_stringview::npos ? length - offset : newline - offset;
        ApplyCommandToComposite(InsertTextCommand::Create(GetDocument(), text.substr(offset, substringLength),
                                                          AllowPasswordEcho::Yes, false));
      }
      if (newline != utf8_stringview::npos)
      {
        VisiblePosition caret(EndingSelection().VisibleStart());
        if (EnclosingNodeOfType(caret.DeepEquivalent(), &IsMailBlockquote))
        {
          // FIXME: Breaking a blockquote when the caret is just after a space will collapse the
          // space. Modify startIndex or length to compensate for this so that the ending selection
          // will be positioned correctly.
          // <rdar://problem/9914462> breaking a Mail blockquote just after a space collapses the space
          if (caret.Previous().CharacterAfter() == U' ')
          {
            if (!offset && !startIndex)
            {
              startIndex--;
            }
            else if (!length)
            {
              length--;
            }
          }
          ApplyCommandToComposite(BreakBlockquoteCommand::Create(GetDocument()));
        }
        else
        {
          InsertLineBreak();
        }
      }

      offset = newline + 1;
    } while (newline != utf8_stringview::npos && offset != length);

    if (selectInsertedText)
    {
      SetEndingSelection(
        VisibleSelection(VisiblePositionForIndex(startIndex, scope.get()),
                         VisiblePositionForIndex(startIndex + static_cast<int32>(length), scope.get())));
    }
  }

  void CompositeEditCommand::InsertTextIntoNode(dom::Text &node, uint32 offset, utf8_stringview text,
                                                AllowPasswordEcho allowPasswordEcho) noexcept
  {
    if (!text.empty())
    {
      ApplyCommandToComposite(
        InsertIntoTextNodeCommand::Create(node, offset, text, allowPasswordEcho, EditingAction()));
    }
  }

  void CompositeEditCommand::DeleteTextFromNode(dom::Text &node, uint32 offset, uint32 count) noexcept
  {
    ApplyCommandToComposite(DeleteFromTextNodeCommand::Create(node, offset, count, EditingAction()));
  }

  void CompositeEditCommand::ReplaceTextInNode(dom::Text &node, uint32 offset, uint32 count,
                                               utf8_stringview replacementText,
                                               AllowPasswordEcho allowPasswordEcho) noexcept
  {
    ApplyCommandToComposite(DeleteFromTextNodeCommand::Create(node, offset, count, EditAction::Unspecified));
    if (!replacementText.empty())
    {
      ApplyCommandToComposite(
        InsertIntoTextNodeCommand::Create(node, offset, replacementText, allowPasswordEcho, EditingAction()));
    }
  }

  dom::Position CompositeEditCommand::ReplaceSelectedTextInNode(utf8_stringview text) noexcept
  {
    dom::Position start = EndingSelection().Start();
    dom::Position end = EndingSelection().End();
    if (start.GetContainerNode() != end.GetContainerNode() || !Is<dom::Text>(start.GetContainerNode())
        || ParentTabSpanNode(start.GetContainerNode()))
    {
      return dom::Position();
    }

    RefPtr<dom::Text> textNode = ShareRefPtr(start.ContainerText());
    auto textLength = static_cast<int32>(textNode->Length());
    if (end.OffsetInContainerNode() < start.OffsetInContainerNode()
        || start.OffsetInContainerNode() > textLength || end.OffsetInContainerNode() > textLength)
    {
      return dom::Position();
    }
    ReplaceTextInNode(*textNode, static_cast<uint32>(start.OffsetInContainerNode()),
                      static_cast<uint32>(end.OffsetInContainerNode() - start.OffsetInContainerNode()), text);

    return dom::Position(RefPtr<dom::Text>(textNode), static_cast<uint32>(start.OffsetInContainerNode())
                                                        + static_cast<uint32>(text.size()));
  }

  void CompositeEditCommand::ReplaceTextInNodePreservingMarkers(dom::Text &node, uint32 offset, uint32 count,
                                                                utf8_stringview replacementText) noexcept
  {
    ReplaceTextInNode(node, offset, count, replacementText);
  }

  dom::Position CompositeEditCommand::PositionOutsideTabSpan(const dom::Position &position) noexcept
  {
    if (!ParentTabSpanNode(position.AnchorNode()))
    {
      return position;
    }

    switch (position.GetAnchorType())
    {
      case dom::Position::AnchorType::PositionIsBeforeChildren:
      case dom::Position::AnchorType::PositionIsAfterChildren:  krys_debug_assert(false); return position;
      case dom::Position::AnchorType::PositionIsOffsetInAnchor: break;
      case dom::Position::AnchorType::PositionIsBeforeAnchor:
        return dom::PositionInParentBeforeNode(position.AnchorNode());
      case dom::Position::AnchorType::PositionIsAfterAnchor:
        return dom::PositionInParentAfterNode(position.AnchorNode());
    }

    RefPtr<dom::Element> tabSpan = ShareRefPtr(ParentTabSpanNode(position.GetContainerNode()));

    if (position.OffsetInContainerNode() <= CaretMinOffset(*position.GetContainerNode()))
    {
      return dom::PositionInParentBeforeNode(tabSpan.get());
    }

    if (position.OffsetInContainerNode() >= CaretMaxOffset(*position.GetContainerNode()))
    {
      return dom::PositionInParentAfterNode(tabSpan.get());
    }

    SplitTextNodeContainingElement(Downcast<dom::Text>(*position.GetContainerNode()),
                                   static_cast<uint32>(position.OffsetInContainerNode()));
    return dom::PositionInParentBeforeNode(tabSpan.get());
  }

  void CompositeEditCommand::InsertNodeAtTabSpanPosition(dom::Node &node,
                                                         const dom::Position &position) noexcept
  {
    // insert node before, after, or at split of tab span
    InsertNodeAt(node, PositionOutsideTabSpan(position));
  }

  namespace
  {
    KRYS_NODISCARD EditAction DeleteSelectionEditingActionForEditingAction(EditAction editingAction) noexcept
    {
      switch (editingAction)
      {
        case EditAction::Cut: return EditAction::Cut;
        default:              return EditAction::Delete;
      }
    }
  }

  void CompositeEditCommand::DeleteSelection(bool smartDelete, bool mergeBlocksAfterDelete, bool replace,
                                             bool expandForSpecialElements, bool sanitizeMarkup) noexcept
  {
    if (EndingSelection().IsRange())
    {
      ApplyCommandToComposite(DeleteSelectionCommand::Create(
        GetDocument(), smartDelete, mergeBlocksAfterDelete, replace, expandForSpecialElements, sanitizeMarkup,
        DeleteSelectionEditingActionForEditingAction(EditingAction())));
    }
  }

  void CompositeEditCommand::DeleteSelection(const VisibleSelection &selection, bool smartDelete,
                                             bool mergeBlocksAfterDelete, bool replace,
                                             bool expandForSpecialElements, bool sanitizeMarkup) noexcept
  {
    if (selection.IsRange())
    {
      ApplyCommandToComposite(DeleteSelectionCommand::Create(
        selection, smartDelete, mergeBlocksAfterDelete, replace, expandForSpecialElements, sanitizeMarkup));
    }
  }

  void CompositeEditCommand::RemoveNodeAttribute(dom::Element &element, dom::DOMStringAtom attribute) noexcept
  {
    SetNodeAttribute(element, attribute, null);
  }

  void CompositeEditCommand::SetNodeAttribute(dom::Element &element, dom::DOMStringAtom attribute,
                                              Maybe<dom::DOMString> value) noexcept
  {
    ApplyCommandToComposite(SetNodeAttributeCommand::Create(element, attribute, krys::move(value)));
  }

  namespace
  {
    // Whether the byte at @p index is editing whitespace, or either byte of a no-break space.
    KRYS_NODISCARD bool IsEditingWhitespaceByte(utf8_stringview text, size_t index) noexcept
    {
      if (index >= text.size())
      {
        return false;
      }
      auto byte = static_cast<uint8>(text[index]);
      if (byte == 0xC2)
      {
        return index + 1 < text.size() && static_cast<uint8>(text[index + 1]) == 0xA0;
      }
      if (byte == 0xA0)
      {
        return index >= 1 && static_cast<uint8>(text[index - 1]) == 0xC2;
      }
      return DeprecatedIsEditingWhitespace(text[index]);
    }

    KRYS_NODISCARD bool ContainsOnlyDeprecatedEditingWhitespace(utf8_stringview text) noexcept
    {
      for (size_t i = 0; i < text.size(); ++i)
      {
        if (!IsEditingWhitespaceByte(text, i))
        {
          return false;
        }
      }
      return true;
    }
  }

  bool CompositeEditCommand::ShouldRebalanceLeadingWhitespaceFor(utf8_stringview text) const noexcept
  {
    return ContainsOnlyDeprecatedEditingWhitespace(text);
  }

  RefPtr<dom::Text> CompositeEditCommand::TextNodeForRebalance(const dom::Position &position) const noexcept
  {
    if (position.GetAnchorType() != dom::Position::AnchorType::PositionIsOffsetInAnchor)
    {
      return nullptr;
    }

    RefPtr<dom::Text> textNode = ShareRefPtr(DynamicDowncast<dom::Text>(position.GetContainerNode()));
    if (!textNode || !textNode->Length())
    {
      return nullptr;
    }

    (void)textNode->NodeDocument().UpdateStyleIfNeeded();

    auto *renderer = textNode->Renderer();
    if (renderer && !renderer->Style().CollapseWhiteSpace())
    {
      return nullptr;
    }

    return textNode;
  }

  // FIXME: Doesn't go into text nodes that contribute adjacent text (siblings, cousins, etc).
  void CompositeEditCommand::RebalanceWhitespaceAt(const dom::Position &position) noexcept
  {
    auto textNode = TextNodeForRebalance(position);
    if (!textNode)
    {
      return;
    }

    // If the rebalance is for the single offset, and neither text[offset] nor text[offset - 1] are some form
    // of whitespace, do nothing.
    int32 offset = position.DeprecatedEditingOffset();
    utf8_stringview text = textNode->Data();
    if (offset < 0 || !IsEditingWhitespaceByte(text, static_cast<size_t>(offset)))
    {
      offset--;
      if (offset < 0 || !IsEditingWhitespaceByte(text, static_cast<size_t>(offset)))
      {
        return;
      }
    }

    RebalanceWhitespaceOnTextSubstring(*textNode, position.OffsetInContainerNode(),
                                       position.OffsetInContainerNode());
  }

  namespace
  {
    // Whether the single-byte @p character is whitespace; the no-break space's bytes are handled whole by the
    // callers.
    KRYS_NODISCARD bool IsWhitespaceForRebalance(dom::Text &textNode, char8_t character) noexcept
    {
      return character < 0x80 && DeprecatedIsEditingWhitespace(character)
             && (character != u8'\n' || !textNode.Renderer()
                 || !textNode.Renderer()->Style().PreserveNewline());
    }

    KRYS_NODISCARD bool IsContinuationByte(const dom::DOMString &text, size_t index) noexcept
    {
      return index < text.size() && (static_cast<uint8>(text[index]) & 0xC0) == 0x80;
    }

    // The no-break space is two bytes in UTF-8; these step over it whole so that whitespace runs end on
    // character boundaries.
    KRYS_NODISCARD size_t WhitespaceLengthBefore(dom::Text &textNode, const dom::DOMString &text,
                                                 size_t index) noexcept
    {
      if (index >= 2 && static_cast<uint8>(text[index - 2]) == 0xC2
          && static_cast<uint8>(text[index - 1]) == 0xA0)
      {
        return 2;
      }
      return index >= 1 && IsWhitespaceForRebalance(textNode, text[index - 1]) ? 1 : 0;
    }

    KRYS_NODISCARD size_t WhitespaceLengthAt(dom::Text &textNode, const dom::DOMString &text,
                                             size_t index) noexcept
    {
      if (index + 1 < text.size() && static_cast<uint8>(text[index]) == 0xC2
          && static_cast<uint8>(text[index + 1]) == 0xA0)
      {
        return 2;
      }
      return index < text.size() && IsWhitespaceForRebalance(textNode, text[index]) ? 1 : 0;
    }
  }

  void CompositeEditCommand::RebalanceWhitespaceOnTextSubstring(dom::Text &textNode, int32 startOffset,
                                                                int32 endOffset) noexcept
  {
    dom::DOMString text = textNode.Data();
    krys_debug_assert(!text.empty());

    // Set upstream and downstream to define the extent of the whitespace surrounding text[offset].
    // Offsets inside a character move out to its edges.
    auto upstream = static_cast<size_t>(std::max(0, startOffset));
    while (upstream && IsContinuationByte(text, upstream))
    {
      --upstream;
    }
    while (auto whitespaceLength = WhitespaceLengthBefore(textNode, text, upstream))
    {
      upstream -= whitespaceLength;
    }

    auto downstream = std::max(upstream, static_cast<size_t>(std::max(0, endOffset)));
    while (IsContinuationByte(text, downstream))
    {
      ++downstream;
    }
    while (auto whitespaceLength = WhitespaceLengthAt(textNode, text, downstream))
    {
      downstream += whitespaceLength;
    }

    size_t length = downstream - upstream;
    if (!length)
    {
      return;
    }

    VisiblePosition visibleUpstreamPos(dom::Position(ShareRefPtr(&textNode), static_cast<uint32>(upstream)));
    VisiblePosition visibleDownstreamPos(
      dom::Position(ShareRefPtr(&textNode), static_cast<uint32>(downstream)));

    utf8_stringview string = utf8_stringview(text).substr(upstream, length);
    // FIXME: Because of the problem mentioned at the top of this function, we must also use nbsps at the
    // start/end of the string because this function doesn't get all surrounding whitespace, just the
    // whitespace in the current text node.
    auto *nextSiblingTextNode = DynamicDowncast<dom::Text>(textNode.NextSibling());
    const bool nextSiblingIsTextNodeWithoutLeadingSpace =
      nextSiblingTextNode && !nextSiblingTextNode->Data().empty()
      && !IsEditingWhitespaceByte(nextSiblingTextNode->Data(), 0);
    auto rebalancedString =
      StringWithRebalancedWhitespace(string, IsStartOfParagraph(visibleUpstreamPos) || !upstream,
                                     (IsEndOfParagraph(visibleDownstreamPos) || downstream == text.size())
                                       && !nextSiblingIsTextNodeWithoutLeadingSpace);

    if (string != rebalancedString)
    {
      ReplaceTextInNodePreservingMarkers(textNode, static_cast<uint32>(upstream), static_cast<uint32>(length),
                                         rebalancedString);
    }
  }

  void CompositeEditCommand::PrepareWhitespaceAtPositionForSplit(dom::Position &position) noexcept
  {
    if (!IsRichlyEditablePosition(position))
    {
      return;
    }

    auto *textNode = DynamicDowncast<dom::Text>(position.DeprecatedNode());
    if (!textNode || !textNode->Length())
    {
      return;
    }

    if (auto *renderer = textNode->Renderer(); renderer && !renderer->Style().CollapseWhiteSpace())
    {
      return;
    }

    // Delete collapsed whitespace so that inserting nbsps doesn't uncollapse it.
    dom::Position upstreamPos = position.Upstream();
    DeleteInsignificantText(upstreamPos, position.Downstream());
    position = upstreamPos.Downstream();

    VisiblePosition visiblePos(position);
    VisiblePosition previousVisiblePos(visiblePos.Previous());
    ReplaceCollapsibleWhitespaceWithNonBreakingSpaceIfNeeded(previousVisiblePos);
    ReplaceCollapsibleWhitespaceWithNonBreakingSpaceIfNeeded(visiblePos);
  }

  void CompositeEditCommand::ReplaceCollapsibleWhitespaceWithNonBreakingSpaceIfNeeded(
    const VisiblePosition &visiblePosition) noexcept
  {
    if (!DeprecatedIsCollapsibleWhitespace(visiblePosition.CharacterAfter()))
    {
      return;
    }

    dom::Position pos = visiblePosition.DeepEquivalent().Downstream();
    if (!pos.GetContainerNode() || !Is<dom::Text>(*pos.GetContainerNode())
        || IsHTMLElementOfType(pos.DeprecatedNode(), html::TagName::br))
    {
      return;
    }
    ReplaceTextInNodePreservingMarkers(*pos.ContainerText(), static_cast<uint32>(pos.OffsetInContainerNode()),
                                       1, NonBreakingSpaceString());
  }

  void CompositeEditCommand::RebalanceWhitespace() noexcept
  {
    VisibleSelection selection = EndingSelection();
    if (selection.IsNone())
    {
      return;
    }

    RebalanceWhitespaceAt(selection.Start());
    if (selection.IsRange())
    {
      RebalanceWhitespaceAt(selection.End());
    }
  }

  void CompositeEditCommand::DeleteInsignificantText(dom::Text &textNode, uint32 start, uint32 end) noexcept
  {
    if (start >= end)
    {
      return;
    }

    (void)GetDocument().UpdateLayout();

    bool wholeTextNodeIsEmpty = false;
    Maybe<dom::DOMString> string;
    auto determineRemovalMode = [&]
    {
      auto *textRenderer = textNode.Renderer();
      if (!textRenderer)
      {
        return;
      }

      auto [run, orderCache] = inline_iterator::FirstTextBoxInLogicalOrderFor(*textRenderer);
      if (!run)
      {
        wholeTextNodeIsEmpty = true;
        return;
      }

      auto length = static_cast<uint32>(textNode.Length());
      if (start >= length || end > length)
      {
        return;
      }

      uint32 removed = 0;
      inline_iterator::TextBoxIterator previousRun;

      // This loop structure works to process all gaps preceding a box,
      // and also will look at the gap after the last box.
      while (previousRun || run)
      {
        uint32 gapStart = previousRun ? previousRun->End() : 0;
        if (end < gapStart)
        {
          break; // No more chance for any intersections
        }

        uint32 gapEnd = run ? run->Start() : length;
        bool indicesIntersect = start <= gapEnd && end >= gapStart;
        int32 gapLength = static_cast<int32>(gapEnd) - static_cast<int32>(gapStart);
        if (indicesIntersect && gapLength > 0)
        {
          gapStart = std::max(gapStart, start);
          gapEnd = std::min(gapEnd, end);
          if (!string)
          {
            string = dom::DOMString(utf8_stringview(textNode.Data()).substr(start, end - start));
          }
          // Remove text in the gap.
          string->erase(gapStart - start - removed, static_cast<size_t>(gapLength));
          removed += static_cast<uint32>(gapLength);
        }

        previousRun = run;
        if (run)
        {
          run = inline_iterator::NextTextBoxInLogicalOrder(run, orderCache);
        }
      }
    };
    determineRemovalMode();

    if (wholeTextNodeIsEmpty)
    {
      RemoveNode(textNode);
      return;
    }

    if (string)
    {
      // Replace the text between start and end with our pruned version.
      if (!string->empty())
      {
        ReplaceTextInNode(textNode, start, end - start, *string);
      }
      else
      {
        // Assert that we are not going to delete all of the text in the node.
        // If we were, that should have been done above with the call to
        // removeNode and return.
        krys_debug_assert(start > 0 || end - start < textNode.Length());
        DeleteTextFromNode(textNode, start, end - start);
      }
    }
  }

  void CompositeEditCommand::DeleteInsignificantText(const dom::Position &start,
                                                     const dom::Position &end) noexcept
  {
    if (!(start < end))
    {
      return;
    }

    List<Ref<dom::Text>> nodes;
    for (auto *node = start.DeprecatedNode(); node; node = TreeQueries::Next(*node))
    {
      if (auto *textNode = DynamicDowncast<dom::Text>(*node))
      {
        nodes.push_back(ShareRef(*textNode));
      }
      if (node == end.DeprecatedNode())
      {
        break;
      }
    }

    for (auto &textNode : nodes)
    {
      int32 startOffset = textNode.get() == start.DeprecatedNode() ? start.DeprecatedEditingOffset() : 0;
      int32 endOffset = textNode.get() == end.DeprecatedNode() ? end.DeprecatedEditingOffset()
                                                               : static_cast<int32>(textNode->Length());
      DeleteInsignificantText(*textNode, static_cast<uint32>(startOffset), static_cast<uint32>(endOffset));
    }
    if (!nodes.empty())
    {
      // Callers expect render tree to be in sync.
      (void)GetDocument().UpdateLayoutIgnorePendingStylesheets();
    }
  }

  void CompositeEditCommand::DeleteInsignificantTextDownstream(const dom::Position &position) noexcept
  {
    DeleteInsignificantText(position, VisiblePosition(position).Next().DeepEquivalent().Downstream());
  }

  RefPtr<dom::Element> CompositeEditCommand::AppendBlockPlaceholder(dom::Element &container) noexcept
  {
    (void)GetDocument().UpdateLayoutIgnorePendingStylesheets();

    // Should check isBlockFlow || isInlineFlow when deletion improves. See 4244964.
    if (!container.Renderer())
    {
      return nullptr;
    }

    auto placeholder = CreateBlockPlaceholderElement(GetDocument());
    AppendNode(*placeholder, container);
    return placeholder;
  }

  RefPtr<dom::Node> CompositeEditCommand::InsertBlockPlaceholder(const dom::Position &position) noexcept
  {
    if (position.IsNull())
    {
      return nullptr;
    }

    // Should assert isBlockFlow || isInlineFlow when deletion improves.  See 4244964.
    krys_debug_assert(position.DeprecatedNode()->Renderer());

    auto placeholder = CreateBlockPlaceholderElement(GetDocument());
    InsertNodeAt(*placeholder, position);
    return placeholder;
  }

  RefPtr<dom::Node> CompositeEditCommand::AddBlockPlaceholderIfNeeded(dom::Element *container) noexcept
  {
    if (!container)
    {
      return nullptr;
    }

    (void)GetDocument().UpdateLayoutIgnorePendingStylesheets();

    auto *blockFlow = DynamicDowncast<RenderBlockFlow>(container->Renderer());
    if (!blockFlow)
    {
      return nullptr;
    }

    // Append the placeholder to make sure it follows any unrendered blocks.
    if (blockFlow->Height() && (!blockFlow->IsRenderListItem() || blockFlow->FirstChild()))
    {
      return nullptr;
    }

    return AppendBlockPlaceholder(*container);
  }

  // Assumes that the position is at a placeholder and does the removal without much checking.
  void CompositeEditCommand::RemovePlaceholderAt(const dom::Position &position) noexcept
  {
    krys_debug_assert(LineBreakExistsAtPosition(position));

    // We are certain that the position is at a line break, but it may be a br or a preserved newline.
    RefPtr<dom::Node> anchorNode = ShareRefPtr(position.AnchorNode());
    if (IsHTMLElementOfType(anchorNode.get(), html::TagName::br))
    {
      RemoveNode(*anchorNode);
      return;
    }

    DeleteTextFromNode(Downcast<dom::Text>(*anchorNode),
                       static_cast<uint32>(position.OffsetInContainerNode()), 1);
  }

  Ref<dom::Element>
    CompositeEditCommand::InsertNewDefaultParagraphElementAt(const dom::Position &position) noexcept
  {
    auto &document = GetDocument();
    auto paragraphElement = CreateDefaultParagraphElement(document);
    (void)paragraphElement->AppendChild(*CreateBreakElement(document));
    InsertNodeAt(*paragraphElement, position);
    return paragraphElement;
  }

  // If the paragraph is not entirely within it's own block, create one and move the paragraph into
  // it, and return that block.  Otherwise return 0.
  RefPtr<dom::Node>
    CompositeEditCommand::MoveParagraphContentsToNewBlockIfNecessary(const dom::Position &position) noexcept
  {
    if (position.IsNull())
    {
      return nullptr;
    }

    (void)GetDocument().UpdateLayoutIgnorePendingStylesheets();

    // It's strange that this function is responsible for verifying that pos has not been invalidated
    // by an earlier call to this function.  The caller, applyBlockStyle, should do this.
    VisiblePosition visiblePos(position);
    VisiblePosition visibleParagraphStart(StartOfParagraph(visiblePos));
    VisiblePosition visibleParagraphEnd = EndOfParagraph(visiblePos);
    if (visibleParagraphStart.IsNull() || visibleParagraphEnd.IsNull())
    {
      return nullptr;
    }

    VisiblePosition next = visibleParagraphEnd.Next();
    VisiblePosition visibleEnd = next.IsNotNull() ? next : visibleParagraphEnd;

    dom::Position upstreamStart = visibleParagraphStart.DeepEquivalent().Upstream();
    dom::Position upstreamEnd = visibleEnd.DeepEquivalent().Upstream();

    // If there are no VisiblePositions in the same block as pos then
    // upstreamStart will be outside the paragraph
    if (position < upstreamStart)
    {
      return nullptr;
    }

    // Perform some checks to see if we need to perform work in this function.
    if (upstreamStart.DeprecatedNode() && IsBlock(*upstreamStart.DeprecatedNode()))
    {
      // If the block is the root editable element, always move content to a new block,
      // since it is illegal to modify attributes on the root editable element for editing.
      if (upstreamStart.DeprecatedNode() == EditableRootForPosition(upstreamStart))
      {
        // If the block is the root editable element and it contains no visible content, create a new
        // block but don't try and move content into it, since there's nothing for moveParagraphs to move.
        if (!dom::Position::HasRenderedNonAnonymousDescendantsWithHeight(
              Downcast<RenderElement>(*upstreamStart.DeprecatedNode()->Renderer())))
        {
          return InsertNewDefaultParagraphElementAt(upstreamStart);
        }
      }
      else if (upstreamEnd.DeprecatedNode() && IsBlock(*upstreamEnd.DeprecatedNode()))
      {
        if (!upstreamEnd.DeprecatedNode()->IsDescendantOf(upstreamStart.DeprecatedNode()))
        {
          // If the paragraph end is a descendant of paragraph start, then we need to run
          // the rest of this function. If not, we can bail here.
          return nullptr;
        }
      }
      else if (upstreamStart.DeprecatedNode() != EnclosingBlock(ShareRefPtr(upstreamEnd.DeprecatedNode())))
      {
        // The visibleEnd. If it is an ancestor of the paragraph start, then
        // we can bail as we have a full block to work with.
        if (upstreamStart.DeprecatedNode()->IsDescendantOf(
              EnclosingBlock(ShareRefPtr(upstreamEnd.DeprecatedNode())).get()))
        {
          return nullptr;
        }
      }
      else if (IsEndOfEditableOrNonEditableContent(visibleEnd))
      {
        // At the end of the editable region. We can bail here as well.
        return nullptr;
      }
    }

    // If upstreamStart is not editable, then we can bail here.
    if (!IsEditablePosition(upstreamStart))
    {
      return nullptr;
    }
    auto newBlock = InsertNewDefaultParagraphElementAt(upstreamStart);

    bool endWasBr =
      IsHTMLElementOfType(visibleParagraphEnd.DeepEquivalent().DeprecatedNode(), html::TagName::br);

    MoveParagraphs(visibleParagraphStart, visibleParagraphEnd,
                   VisiblePosition(dom::FirstPositionInNode(newBlock.get())));

    if (newBlock->LastChild() && IsHTMLElementOfType(newBlock->LastChild(), html::TagName::br) && !endWasBr)
    {
      RemoveNode(*newBlock->LastChild());
    }

    return newBlock;
  }

  void CompositeEditCommand::PushAnchorElementDown(dom::Element &anchorElement) noexcept
  {
    SetEndingSelection(VisibleSelection::SelectionFromContentsOfNode(&anchorElement));
    ApplyStyledElement(anchorElement);
    // Clones of anchorElement have been pushed down, now remove it.
    if (anchorElement.IsConnected())
    {
      RemoveNodePreservingChildren(anchorElement);
    }
  }

  // Clone the paragraph between start and end under blockElement,
  // preserving the hierarchy up to outerNode.

  void CompositeEditCommand::CloneParagraphUnderNewElement(const dom::Position &start,
                                                           const dom::Position &end,
                                                           dom::Node *passedOuterNode,
                                                           dom::Element *blockElement) noexcept
  {
    krys_debug_assert(start <= end);

    auto cloneNode = [&](dom::Node &node, bool deep) -> Ref<dom::Node>
    {
      auto clone = dom::NodeAlgorithms::CloneNode(node, &GetDocument(), deep);
      krys_debug_assert(!clone.HasException());
      return clone.ReleaseValue();
    };

    // First we clone the outerNode
    RefPtr<dom::Node> lastNode;
    RefPtr<dom::Node> outerNode = ShareRefPtr(passedOuterNode);

    if (outerNode->IsRootEditableElement())
    {
      lastNode = ShareRefPtr(blockElement);
    }
    else
    {
      lastNode = cloneNode(*outerNode, IsRenderedTable(outerNode.get()));
      AppendNode(*lastNode, *blockElement);
    }

    if (start.DeprecatedNode() != outerNode && Is<dom::Element>(*lastNode)
        && start.AnchorNode()->IsDescendantOf(outerNode.get()))
    {
      List<RefPtr<dom::Node>> ancestors;

      // Insert each node from innerNode to outerNode (excluded) in a list.
      for (auto *n = start.DeprecatedNode(); n && n != outerNode; n = n->ParentNode())
      {
        ancestors.push_back(ShareRefPtr(n));
      }

      // Clone every node between start.deprecatedNode() and outerBlock.

      for (size_t i = ancestors.size(); i != 0; --i)
      {
        auto item = std::exchange(ancestors[i - 1], nullptr);
        auto child = cloneNode(*item, IsRenderedTable(item.get()));
        AppendNode(*child, Downcast<dom::Element>(*lastNode));
        lastNode = krys::move(child);
      }
    }

    if (!start.DeprecatedNode()->IsConnected() || !end.DeprecatedNode()->IsConnected())
    {
      return;
    }

    // Handle the case of paragraphs with more than one node,
    // cloning all the siblings until end.deprecatedNode() is reached.

    if (start.DeprecatedNode() != end.DeprecatedNode()
        && !start.DeprecatedNode()->IsDescendantOf(end.DeprecatedNode()))
    {
      // If end is not a descendant of outerNode we need to
      // find the first common ancestor to increase the scope
      // of our nextSibling traversal.
      while (!end.DeprecatedNode()->IsDescendantOf(outerNode.get()) && outerNode->ParentNode())
      {
        outerNode = ShareRefPtr(outerNode->ParentNode());
      }

      RefPtr<dom::Node> startNode = ShareRefPtr(start.DeprecatedNode());
      for (RefPtr<dom::Node> node =
             ShareRefPtr(TreeQueries::NextSkippingChildren(*startNode, outerNode.get()));
           node; node = ShareRefPtr(TreeQueries::NextSkippingChildren(*node, outerNode.get())))
      {
        // Move lastNode up in the tree as much as node was moved up in the
        // tree by NodeTraversal::nextSkippingChildren, so that the relative depth between
        // node and the original start node is maintained in the clone.
        while (startNode->ParentNode() && lastNode->ParentNode()
               && startNode->ParentNode() != node->ParentNode())
        {
          startNode = ShareRefPtr(startNode->ParentNode());
          lastNode = ShareRefPtr(lastNode->ParentNode());
        }

        auto clonedNode = cloneNode(*node, true);
        InsertNodeAfter(*clonedNode, *lastNode);
        lastNode = krys::move(clonedNode);
        if (node == end.DeprecatedNode() || end.DeprecatedNode()->IsDescendantOf(*node))
        {
          break;
        }
      }
    }
  }

  // There are bugs in deletion when it removes a fully selected table/list.
  // It expands and removes the entire table/list, but will let content
  // before and after the table/list collapse onto one line.
  // Deleting a paragraph will leave a placeholder. Remove it (and prune
  // empty or unrendered parents).

  void CompositeEditCommand::CleanupAfterDeletion(VisiblePosition destination) noexcept
  {
    VisiblePosition caretAfterDelete = EndingSelection().VisibleStart();
    if (caretAfterDelete != destination && IsStartOfParagraph(caretAfterDelete)
        && IsEndOfParagraph(caretAfterDelete))
    {
      // Note: We want the rightmost candidate.
      dom::Position position = caretAfterDelete.DeepEquivalent().Downstream();
      RefPtr<dom::Node> node = ShareRefPtr(position.DeprecatedNode());
      krys_debug_assert(node);
      // Normally deletion will leave a br as a placeholder.
      if (IsHTMLElementOfType(node.get(), html::TagName::br))
      {
        RemoveNodeAndPruneAncestors(*node);
      }
      // If the selection to move was empty and in an empty block that
      // doesn't require a placeholder to prop itself open (like a bordered
      // div or an li), remove it during the move (the list removal code
      // expects this behavior).
      else if (IsBlock(*node))
      {
        // If caret position after deletion and destination position coincides,
        // node should not be removed.
        if (!position.RendersInDifferentPosition(destination.DeepEquivalent()))
        {
          Prune(node.get());
          return;
        }
        RemoveNodeAndPruneAncestors(*node);
      }
      else if (LineBreakExistsAtPosition(position))
      {
        // There is a preserved '\n' at caretAfterDelete.
        // We can safely assume this is a text node.
        auto &textNode = Downcast<dom::Text>(*node);
        if (textNode.Length() == 1)
        {
          RemoveNodeAndPruneAncestors(textNode);
        }
        else
        {
          DeleteTextFromNode(textNode, static_cast<uint32>(position.DeprecatedEditingOffset()), 1);
        }
      }
    }
  }

  // This is a version of moveParagraph that preserves style by keeping the original markup
  // It is currently used only by IndentOutdentCommand but it is meant to be used in the
  // future by several other commands such as InsertList and the align commands.
  // The blockElement parameter is the element to move the paragraph to,
  // outerNode is the top element of the paragraph hierarchy.

  void CompositeEditCommand::MoveParagraphWithClones(const VisiblePosition &startOfParagraphToMove,
                                                     const VisiblePosition &endOfParagraphToMove,
                                                     dom::Element *blockElement,
                                                     dom::Node *outerNode) noexcept
  {
    if (startOfParagraphToMove.IsNull() || endOfParagraphToMove.IsNull())
    {
      return;
    }

    krys_debug_assert(outerNode);
    krys_debug_assert(blockElement);

    VisiblePosition beforeParagraph = startOfParagraphToMove.Previous();
    VisiblePosition afterParagraph(endOfParagraphToMove.Next());

    // We upstream() the end and downstream() the start so that we don't include collapsed whitespace in the
    // move. When we paste a fragment, spaces after the end and before the start are treated as though they
    // were rendered.
    dom::Position start = startOfParagraphToMove.DeepEquivalent().Downstream();
    dom::Position end = startOfParagraphToMove == endOfParagraphToMove
                          ? start
                          : endOfParagraphToMove.DeepEquivalent().Upstream();

    CloneParagraphUnderNewElement(start, end, outerNode, blockElement);

    SetEndingSelection(VisibleSelection(start, end));
    DeleteSelection(false, false, false, false);

    // There are bugs in deletion when it removes a fully selected table/list.
    // It expands and removes the entire table/list, but will let content
    // before and after the table/list collapse onto one line.

    CleanupAfterDeletion();

    // Add a br if pruning an empty block level element caused a collapse.  For example:
    // foo^
    // <div>bar</div>
    // baz
    // Imagine moving 'bar' to ^.  'bar' will be deleted and its div pruned.  That would
    // cause 'baz' to collapse onto the line with 'foobar' unless we insert a br.
    // Must recononicalize these two VisiblePositions after the pruning above.
    beforeParagraph = VisiblePosition(beforeParagraph.DeepEquivalent());
    afterParagraph = VisiblePosition(afterParagraph.DeepEquivalent());

    if (beforeParagraph.IsNotNull() && !IsRenderedTable(beforeParagraph.DeepEquivalent().DeprecatedNode())
        && ((!IsEndOfParagraph(beforeParagraph) && !IsStartOfParagraph(beforeParagraph))
            || beforeParagraph == afterParagraph)
        && IsEditablePosition(beforeParagraph.DeepEquivalent()))
    {
      // FIXME: Trim text between beforeParagraph and afterParagraph if they aren't equal.
      InsertNodeAt(*CreateBreakElement(GetDocument()), beforeParagraph.DeepEquivalent());
    }
  }

  // This moves a paragraph preserving its style.
  void CompositeEditCommand::MoveParagraph(const VisiblePosition &startOfParagraphToMove,
                                           const VisiblePosition &endOfParagraphToMove,
                                           const VisiblePosition &destination, bool preserveSelection,
                                           bool preserveStyle) noexcept
  {
    krys_debug_assert(IsStartOfParagraph(startOfParagraphToMove));
    krys_debug_assert(IsEndOfParagraph(endOfParagraphToMove));
    MoveParagraphs(startOfParagraphToMove, endOfParagraphToMove, destination, preserveSelection,
                   preserveStyle);
  }

  // FIXME: Clearing and marking the spelling and grammar of the moved paragraph
  // (clearMisspellingsAndBadGrammar, markMisspellingsAndBadGrammar), with text checking.
  void CompositeEditCommand::MoveParagraphs(const VisiblePosition &startOfParagraphToMove,
                                            const VisiblePosition &endOfParagraphToMove,
                                            const VisiblePosition &passedDestination, bool preserveSelection,
                                            bool preserveStyle) noexcept
  {
    if (passedDestination.IsNull() || startOfParagraphToMove == passedDestination)
    {
      return;
    }

    krys_debug_assert((startOfParagraphToMove.IsNull() && endOfParagraphToMove.IsNull())
                      || !endOfParagraphToMove.IsNull());

    VisiblePosition destination = passedDestination;

    Maybe<uint64> startIndex;
    Maybe<uint64> endIndex;
    auto originalDirectionality = EndingSelection().GetDirectionality();
    constexpr TextIteratorBehaviors behaviors {
      TextIteratorBehavior::EmitsCharactersBetweenAllVisiblePositions};
    if (preserveSelection && !EndingSelection().IsNone())
    {
      auto visibleStart = EndingSelection().VisibleStart();
      auto visibleEnd = EndingSelection().VisibleEnd();
      if (visibleStart <= endOfParagraphToMove && visibleEnd >= startOfParagraphToMove)
      {
        startIndex = 0;
        if (visibleStart >= startOfParagraphToMove)
        {
          if (auto rangeToSelectionStart =
                MakeSimpleRange(VisiblePositionRange {startOfParagraphToMove, visibleStart}))
          {
            startIndex = CharacterCount(*rangeToSelectionStart, behaviors);
          }
        }
        endIndex = 0;
        if (visibleEnd <= endOfParagraphToMove)
        {
          if (auto rangeToSelectionEnd =
                MakeSimpleRange(VisiblePositionRange {startOfParagraphToMove, visibleEnd}))
          {
            endIndex = CharacterCount(*rangeToSelectionEnd, behaviors);
          }
        }
      }
    }

    auto beforeParagraph =
      startOfParagraphToMove.Previous(EditingBoundaryCrossingRule::CannotCrossEditingBoundary);
    auto afterParagraph = endOfParagraphToMove.Next(EditingBoundaryCrossingRule::CannotCrossEditingBoundary);

    // We upstream() the end and downstream() the start so that we don't include collapsed whitespace in the
    // move. When we paste a fragment, spaces after the end and before the start are treated as though they
    // were rendered.
    VisiblePosition start = startOfParagraphToMove.DeepEquivalent().Downstream();
    VisiblePosition end = endOfParagraphToMove.DeepEquivalent().Upstream();

    if (start.IsNull() || end.IsNull())
    {
      return;
    }

    Ref<dom::Document> document = ShareRef(GetDocument());
    // FIXME: Serializing and re-parsing is an inefficient way to preserve style.
    RefPtr<dom::DocumentFragment> fragment;
    if (startOfParagraphToMove != endOfParagraphToMove)
    {
      fragment = CreateFragmentFromMarkup(
        *document,
        SerializePreservingVisualAppearance(*MakeSimpleRange(VisiblePositionRange {start, end}), nullptr,
                                            AnnotateForInterchange::No, ConvertBlocksToInlines::Yes),
        {});
    }

    // A non-empty paragraph's style is moved when we copy and move it.  We don't move
    // anything if we're given an empty paragraph, but an empty paragraph can have style
    // too, <div><b><br></b></div> for example.  Save it so that we can preserve it later.
    RefPtr<EditingStyle> styleInEmptyParagraph;
    if (startOfParagraphToMove == endOfParagraphToMove && preserveStyle)
    {
      styleInEmptyParagraph = EditingStyle::Create(startOfParagraphToMove.DeepEquivalent());
      styleInEmptyParagraph->MergeTypingStyle(*document);
      // The moved paragraph should assume the block style of the destination.
      styleInEmptyParagraph->RemoveBlockProperties();
    }

    // FIXME (5098931): We should add a new insert action "WebViewInsertActionMoved" and call
    // shouldInsertFragment here.

    SetEndingSelection(VisibleSelection(start, end));

    auto downstreamDestination = destination.DeepEquivalent().Downstream();

    DeleteSelection(false, false, false, false);

    if (destination.IsOrphan())
    {
      destination = VisiblePosition {downstreamDestination};
    }

    krys_debug_assert(!destination.DeepEquivalent().IsOrphan());
    CleanupAfterDeletion(destination);

    // FIXME (Bug 211793): We should redesign cleanupAfterDeletion or find another destination when it is
    // removed.
    if (destination.DeepEquivalent().IsOrphan()
        || VisibleSelection(destination, originalDirectionality).IsNone())
    {
      return;
    }

    // Add a br if pruning an empty block level element caused a collapse. For example:
    // foo^
    // <div>bar</div>
    // baz
    // Imagine moving 'bar' to ^. 'bar' will be deleted and its div pruned. That would
    // cause 'baz' to collapse onto the line with 'foobar' unless we insert a br.
    // Must recononicalize these two VisiblePositions after the pruning above.
    beforeParagraph = VisiblePosition(beforeParagraph.DeepEquivalent());
    afterParagraph = VisiblePosition(afterParagraph.DeepEquivalent());
    if (beforeParagraph.IsNotNull()
        && ((!IsStartOfParagraph(beforeParagraph) && !IsEndOfParagraph(beforeParagraph))
            || beforeParagraph == afterParagraph))
    {
      // FIXME: Trim text between beforeParagraph and afterParagraph if they aren't equal.
      InsertNodeAt(*CreateBreakElement(*document), beforeParagraph.DeepEquivalent());
      // Need an updateLayout here in case inserting the br has split a text node.
      (void)document->UpdateLayoutIgnorePendingStylesheets();
    }

    RefPtr<dom::ContainerNode> editableRoot = ShareRefPtr(destination.RootEditableElement());
    if (!editableRoot)
    {
      editableRoot = document;
    }

    auto destinationIndex = CharacterCount(
      {dom::BoundaryPoint(ShareRef(*editableRoot), 0), *MakeBoundaryPoint(destination)}, behaviors);

    SetEndingSelection(VisibleSelection(destination, originalDirectionality));
    krys_debug_assert(EndingSelection().IsCaretOrRange());
    EnumSet<ReplaceSelectionCommand::CommandOption> options {
      ReplaceSelectionCommand::CommandOption::SelectReplacement,
      ReplaceSelectionCommand::CommandOption::MovingParagraph};
    if (!preserveStyle)
    {
      options.Add(ReplaceSelectionCommand::CommandOption::MatchStyle);
    }
    ApplyCommandToComposite(ReplaceSelectionCommand::Create(*document, krys::move(fragment), options));

    // If the selection is in an empty paragraph, restore styles from the old empty paragraph to the new empty
    // paragraph.
    bool selectionIsEmptyParagraph = EndingSelection().IsCaret()
                                     && IsStartOfParagraph(EndingSelection().VisibleStart())
                                     && IsEndOfParagraph(EndingSelection().VisibleStart());
    if (styleInEmptyParagraph && selectionIsEmptyParagraph)
    {
      ApplyStyle(styleInEmptyParagraph.get());
    }

    if (preserveSelection && startIndex && endIndex)
    {
      // Fragment creation (using createMarkup) incorrectly uses regular
      // spaces instead of nbsps for some spaces that were rendered (11475), which
      // causes spaces to be collapsed during the move operation.  This results
      // in a call to rangeFromLocationAndLength with a location past the end
      // of the document (which will return null).
      auto scope = dom::MakeRangeSelectingNodeContents(*editableRoot);
      auto startPosition = dom::MakeDeprecatedLegacyPosition(
        ResolveCharacterLocation(scope, destinationIndex + *startIndex, behaviors));
      auto endPosition = dom::MakeDeprecatedLegacyPosition(
        ResolveCharacterLocation(scope, destinationIndex + *endIndex, behaviors));
      SetEndingSelection(
        VisibleSelection(startPosition, endPosition, Affinity::Downstream, originalDirectionality));
    }
  }

  VisibleSelection CompositeEditCommand::ShouldBreakOutOfEmptyListItem() const noexcept
  {
    RefPtr<dom::Node> emptyListItem = EnclosingEmptyListItem(EndingSelection().VisibleStart());
    if (!emptyListItem)
    {
      return {};
    }

    RefPtr<dom::ContainerNode> listNode = ShareRefPtr(emptyListItem->ParentNode());
    // FIXME: Can't we do something better when the immediate parent wasn't a list node?
    if (!listNode
        || (!IsHTMLElementOfType(listNode.get(), html::TagName::ul)
            && !IsHTMLElementOfType(listNode.get(), html::TagName::ol))
        || !listNode->HasEditableStyle() || listNode == emptyListItem->RootEditableElement())
    {
      return {};
    }

    return VisibleSelection(EndingSelection().Start().Previous(dom::PositionMoveType::BackwardDeletion),
                            EndingSelection().End());
  }

  // FIXME: Send an appropriate shouldDeleteRange call.
  bool CompositeEditCommand::BreakOutOfEmptyListItem() noexcept
  {
    if (ShouldBreakOutOfEmptyListItem().IsNone())
    {
      return false;
    }

    RefPtr<dom::Node> emptyListItem = EnclosingEmptyListItem(EndingSelection().VisibleStart());
    RefPtr<dom::ContainerNode> listNode = ShareRefPtr(emptyListItem->ParentNode());
    auto style = EditingStyle::Create(EndingSelection().Start());
    Ref<dom::Document> document = ShareRef(GetDocument());
    style->MergeTypingStyle(*document);

    RefPtr<dom::Element> newBlock;
    if (RefPtr<dom::ContainerNode> blockEnclosingList = ShareRefPtr(listNode->ParentNode()))
    {
      if (IsHTMLElementOfType(blockEnclosingList.get(), html::TagName::li))
      {
        // listNode is inside another list item
        auto &liElement = Downcast<dom::Element>(*blockEnclosingList);
        if (VisiblePositionAfterNode(*blockEnclosingList) == VisiblePositionAfterNode(*listNode))
        {
          // If listNode appears at the end of the outer list item, then move listNode outside of this list
          // item e.g. <ul><li>hello <ul><li><br></li></ul> </li></ul> should become <ul><li>hello</li>
          // <ul><li><br></li></ul> </ul> after this section If listNode does NOT appear at the end, then we
          // should consider it as a regular paragraph. e.g. <ul><li> <ul><li><br></li></ul> hello</li></ul>
          // should become <ul><li> <div><br></div> hello</li></ul> at the end
          SplitElement(liElement, *listNode);
          RemoveNodePreservingChildren(*listNode->ParentNode());
          newBlock = CreateHTMLElement(*document, u8"li");
        }
        // If listNode does NOT appear at the end of the outer list item, then behave as if in a regular
        // paragraph.
      }
      else if (IsHTMLElementOfType(blockEnclosingList.get(), html::TagName::ol)
               || IsHTMLElementOfType(blockEnclosingList.get(), html::TagName::ul))
      {
        newBlock = CreateHTMLElement(*document, u8"li");
      }
    }
    if (!newBlock)
    {
      newBlock = CreateDefaultParagraphElement(*document);
    }

    RefPtr<dom::Node> previousListNode =
      Is<dom::Element>(*emptyListItem)
        ? ShareRefPtr<dom::Node>(TreeQueries::PreviousElementSibling(*emptyListItem))
        : ShareRefPtr(emptyListItem->PreviousSibling());
    RefPtr<dom::Node> nextListNode =
      Is<dom::Element>(*emptyListItem)
        ? ShareRefPtr<dom::Node>(TreeQueries::NextElementSibling(*emptyListItem))
        : ShareRefPtr(emptyListItem->NextSibling());
    if ((nextListNode && IsListItem(*nextListNode)) || IsListHTMLElement(nextListNode.get()))
    {
      // If emptyListItem follows another list item or nested list, split the list node.
      if (previousListNode && (IsListItem(*previousListNode) || IsListHTMLElement(previousListNode.get())))
      {
        SplitElement(Downcast<dom::Element>(*listNode), *emptyListItem);
      }

      // If emptyListItem is followed by other list item or nested list, then insert newBlock before the list
      // node. Because we have splitted the element, emptyListItem is the first element in the list node. i.e.
      // insert newBlock before ul or ol whose first element is emptyListItem
      (void)InsertNodeBefore(*newBlock, *listNode);
      RemoveNode(*emptyListItem);
    }
    else
    {
      // When emptyListItem does not follow any list item or nested list, insert newBlock after the enclosing
      // list node. Remove the enclosing node if emptyListItem is the only child; otherwise just remove
      // emptyListItem.
      InsertNodeAfter(*newBlock, *listNode);
      RemoveNode(
        (previousListNode && (IsListItem(*previousListNode) || IsListHTMLElement(previousListNode.get())))
          ? *emptyListItem
          : static_cast<dom::Node &>(*listNode));
    }

    (void)AppendBlockPlaceholder(*newBlock);
    SetEndingSelection(VisibleSelection(dom::FirstPositionInNode(newBlock.get()), Affinity::Downstream,
                                        EndingSelection().GetDirectionality()));

    style->PrepareToApplyAt(EndingSelection().Start());
    if (!style->IsEmpty())
    {
      ApplyStyle(style.get());
    }

    return true;
  }

  // If the caret is in an empty quoted paragraph, and either there is nothing before that
  // paragraph, or what is before is unquoted, and the user presses delete, unquote that paragraph.
  bool CompositeEditCommand::BreakOutOfEmptyMailBlockquotedParagraph() noexcept
  {
    if (!EndingSelection().IsCaret())
    {
      return false;
    }

    VisiblePosition caret(EndingSelection().VisibleStart());
    RefPtr<dom::Node> highestBlockquote =
      HighestEnclosingNodeOfType(caret.DeepEquivalent(), &IsMailBlockquote);
    if (!highestBlockquote)
    {
      return false;
    }

    if (!IsStartOfParagraph(caret) || !IsEndOfParagraph(caret))
    {
      return false;
    }

    VisiblePosition previous(caret.Previous(EditingBoundaryCrossingRule::CannotCrossEditingBoundary));
    // Only move forward if there's nothing before the caret, or if there's unquoted content before it.
    if (EnclosingNodeOfType(previous.DeepEquivalent(), &IsMailBlockquote))
    {
      return false;
    }

    Ref<dom::Document> document = ShareRef(GetDocument());
    auto br = CreateBreakElement(*document);
    // We want to replace this quoted paragraph with an unquoted one, so insert a br
    // to hold the caret before the highest blockquote.
    (void)InsertNodeBefore(*br, *highestBlockquote);
    VisiblePosition atBR = dom::PositionBeforeNode(br.get());
    // If the br we inserted collapsed, for example foo<br><blockquote>...</blockquote>, insert
    // a second one.
    if (!IsStartOfParagraph(atBR))
    {
      (void)InsertNodeBefore(*CreateBreakElement(*document), *br);
    }
    SetEndingSelection(VisibleSelection(atBR, EndingSelection().GetDirectionality()));

    // If this is an empty paragraph there must be a line break here.
    if (!LineBreakExistsAtVisiblePosition(caret))
    {
      return false;
    }

    dom::Position caretPos(caret.DeepEquivalent().Downstream());
    // A line break is either a br or a preserved newline.
    krys_debug_assert(IsHTMLElementOfType(caretPos.DeprecatedNode(), html::TagName::br)
                      || (Is<dom::Text>(*caretPos.DeprecatedNode())
                          && caretPos.DeprecatedNode()->Renderer()->Style().PreserveNewline()));

    if (IsHTMLElementOfType(caretPos.DeprecatedNode(), html::TagName::br))
    {
      RemoveNodeAndPruneAncestors(*caretPos.DeprecatedNode());
    }
    else if (RefPtr<dom::Text> textNode = ShareRefPtr(DynamicDowncast<dom::Text>(*caretPos.DeprecatedNode())))
    {
      krys_debug_assert(caretPos.DeprecatedEditingOffset() == 0);
      RefPtr<dom::ContainerNode> parentNode = ShareRefPtr(textNode->ParentNode());
      // The preserved newline must be the first thing in the node, since otherwise the previous
      // paragraph would be quoted, and we verified that it wasn't above.
      DeleteTextFromNode(*textNode, 0, 1);
      Prune(parentNode.get());
    }

    return true;
  }

  // Operations use this function to avoid inserting content into an anchor when at the start or the end of
  // that anchor, as in NSTextView.
  // FIXME: This is only an approximation of NSTextViews insertion behavior, which varies depending on how
  // the caret was made.
  dom::Position
    CompositeEditCommand::PositionAvoidingSpecialElementBoundary(const dom::Position &original) noexcept
  {
    if (original.IsNull())
    {
      return original;
    }

    VisiblePosition visiblePos(original);
    RefPtr<dom::Element> enclosingAnchor = EnclosingAnchorElement(original);
    dom::Position result = original;

    if (!enclosingAnchor)
    {
      return result;
    }

    // Don't avoid block level anchors, because that would insert content into the wrong paragraph.
    if (enclosingAnchor && !IsBlock(*enclosingAnchor))
    {
      VisiblePosition firstInAnchor(dom::FirstPositionInNode(enclosingAnchor.get()));
      VisiblePosition lastInAnchor(dom::LastPositionInNode(enclosingAnchor.get()));
      // If visually just after the anchor, insert *inside* the anchor unless it's the last
      // VisiblePosition in the document, to match NSTextView.
      if (visiblePos == lastInAnchor)
      {
        // Make sure anchors are pushed down before avoiding them so that we don't
        // also avoid structural elements like lists and blocks (5142012).
        if (original.DeprecatedNode() != enclosingAnchor
            && original.DeprecatedNode()->ParentNode() != enclosingAnchor)
        {
          PushAnchorElementDown(*enclosingAnchor);
          enclosingAnchor = EnclosingAnchorElement(original);
          if (!enclosingAnchor)
          {
            return original;
          }
        }
        // Don't insert outside an anchor if doing so would skip over a line break.  It would
        // probably be safe to move the line break so that we could still avoid the anchor here.
        dom::Position downstream(visiblePos.DeepEquivalent().Downstream());
        if (LineBreakExistsAtVisiblePosition(visiblePos)
            && downstream.DeprecatedNode()->IsDescendantOf(enclosingAnchor.get()))
        {
          return original;
        }

        result = dom::PositionInParentAfterNode(enclosingAnchor.get());
      }
      // If visually just before an anchor, insert *outside* the anchor unless it's the first
      // VisiblePosition in a paragraph, to match NSTextView.
      if (visiblePos == firstInAnchor)
      {
        // Make sure anchors are pushed down before avoiding them so that we don't
        // also avoid structural elements like lists and blocks (5142012).
        if (original.DeprecatedNode() != enclosingAnchor
            && original.DeprecatedNode()->ParentNode() != enclosingAnchor)
        {
          PushAnchorElementDown(*enclosingAnchor);
          enclosingAnchor = EnclosingAnchorElement(original);
        }
        if (!enclosingAnchor)
        {
          return original;
        }

        result = dom::PositionInParentBeforeNode(enclosingAnchor.get());
      }
    }

    if (result.IsNull() || !EditableRootForPosition(result))
    {
      result = original;
    }

    return result;
  }

  // Splits the tree parent by parent until we reach the specified ancestor. We use VisiblePositions
  // to determine if the split is necessary. Returns the last split node.
  RefPtr<dom::Node> CompositeEditCommand::SplitTreeToNode(dom::Node &start, dom::Node &end,
                                                          bool shouldSplitAncestor) noexcept
  {
    krys_debug_assert(&start != &end);

    RefPtr<dom::Node> adjustedEnd = ShareRefPtr(&end);
    if (shouldSplitAncestor && adjustedEnd->ParentNode())
    {
      adjustedEnd = ShareRefPtr(adjustedEnd->ParentNode());
    }

    krys_debug_assert(adjustedEnd);
    RefPtr<dom::Node> node;
    for (node = ShareRefPtr(&start); node && adjustedEnd != node->ParentNode();)
    {
      RefPtr<dom::ContainerNode> parentNode = ShareRefPtr(node->ParentNode());
      auto *parentElement = DynamicDowncast<dom::Element>(parentNode.get());
      if (!parentElement || EditingIgnoresContent(*parentNode))
      {
        break;
      }
      // Do not split a node when doing so introduces an empty node.
      VisiblePosition positionInParent = dom::FirstPositionInNode(parentNode.get());
      VisiblePosition positionInNode = FirstPositionInOrBeforeNode(node.get());
      if (positionInParent != positionInNode)
      {
        SplitElement(*parentElement, *node);
      }
      node = parentNode;
    }

    return node;
  }

  Ref<dom::Element> CreateBlockPlaceholderElement(dom::Document &document) noexcept
  {
    return CreateBreakElement(document);
  }
}
