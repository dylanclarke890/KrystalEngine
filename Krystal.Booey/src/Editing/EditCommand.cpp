#include "Krystal.Booey/Editing/EditCommand.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Editing/CompositeEditCommand.hpp"
#include "Krystal.Booey/Editing/Editing.hpp"
#include "Krystal.Booey/Editing/FrameSelection.hpp"
#include "Krystal.Booey/HTML/HTMLTextFormControlElement.hpp"

namespace krys::boo
{
  utf8_stringview InputTypeNameForEditingAction(EditAction action) noexcept
  {
    switch (action)
    {
      case EditAction::Justify:                        return u8"formatJustifyFull";
      case EditAction::AlignLeft:                      return u8"formatJustifyLeft";
      case EditAction::AlignRight:                     return u8"formatJustifyRight";
      case EditAction::Center:                         return u8"formatJustifyCenter";
      case EditAction::Subscript:                      return u8"formatSubscript";
      case EditAction::Superscript:                    return u8"formatSuperscript";
      case EditAction::Underline:                      return u8"formatUnderline";
      case EditAction::StrikeThrough:                  return u8"formatStrikeThrough";
      case EditAction::SetColor:                       return u8"formatFontColor";
      case EditAction::DeleteByDrag:                   return u8"deleteByDrag";
      case EditAction::Cut:                            return u8"deleteByCut";
      case EditAction::Bold:                           return u8"formatBold";
      case EditAction::Italics:                        return u8"formatItalic";
      case EditAction::Paste:                          return u8"insertFromPaste";
      case EditAction::Delete:
      case EditAction::TypingDeleteSelection:          return u8"deleteContent";
      case EditAction::TypingDeleteBackward:           return u8"deleteContentBackward";
      case EditAction::TypingDeleteForward:            return u8"deleteContentForward";
      case EditAction::TypingDeleteWordBackward:       return u8"deleteWordBackward";
      case EditAction::TypingDeleteWordForward:        return u8"deleteWordForward";
      case EditAction::TypingDeleteLineBackward:       return u8"deleteHardLineBackward";
      case EditAction::TypingDeleteLineForward:        return u8"deleteHardLineForward";
      case EditAction::TypingDeletePendingComposition: return u8"deleteCompositionText";
      case EditAction::TypingDeleteFinalComposition:   return u8"deleteByComposition";
      case EditAction::Insert:
      case EditAction::TypingInsertText:               return u8"insertText";
      case EditAction::InsertReplacement:              return u8"insertReplacementText";
      case EditAction::InsertFromDrop:                 return u8"insertFromDrop";
      case EditAction::TypingInsertLineBreak:          return u8"insertLineBreak";
      case EditAction::TypingInsertParagraph:          return u8"insertParagraph";
      case EditAction::InsertOrderedList:              return u8"insertOrderedList";
      case EditAction::InsertUnorderedList:            return u8"insertUnorderedList";
      case EditAction::TypingInsertPendingComposition: return u8"insertCompositionText";
      case EditAction::TypingInsertFinalComposition:   return u8"insertFromComposition";
      case EditAction::Indent:                         return u8"formatIndent";
      case EditAction::Outdent:                        return u8"formatOutdent";
      case EditAction::SetInlineWritingDirection:      return u8"formatSetInlineTextDirection";
      case EditAction::SetBlockWritingDirection:       return u8"formatSetBlockTextDirection";
      case EditAction::CreateLink:                     return u8"insertLink";
      default:                                         return {};
    }
  }

  bool IsInputMethodComposingForEditingAction(EditAction action) noexcept
  {
    switch (action)
    {
      case EditAction::TypingDeletePendingComposition:
      case EditAction::TypingDeleteFinalComposition:
      case EditAction::TypingInsertPendingComposition:
      case EditAction::TypingInsertFinalComposition:   return true;
      default:                                         break;
    }
    return false;
  }

  EditCommand::EditCommand(dom::Document &document, EditAction editingAction) noexcept
      : _document(ShareRef(document)), _startingSelection(document.Selection().Selection()),
        _endingSelection(_startingSelection), _editingAction(editingAction)
  {
  }

  EditCommand::EditCommand(dom::Document &document, const VisibleSelection &startingSelection,
                           const VisibleSelection &endingSelection) noexcept
      : _document(ShareRef(document)), _startingSelection(startingSelection),
        _endingSelection(endingSelection)
  {
  }

  EditCommand::~EditCommand() noexcept = default;

  EditAction EditCommand::EditingAction() const noexcept
  {
    return _editingAction;
  }

  namespace
  {
    KRYS_NODISCARD EditCommandComposition *CompositionIfPossible(EditCommand &command) noexcept
    {
      auto *compositeCommand = DynamicDowncast<CompositeEditCommand>(command);
      return compositeCommand ? compositeCommand->Composition() : nullptr;
    }
  }

  bool EditCommand::IsEditingTextAreaOrTextInput() const noexcept
  {
    return html::EnclosingTextFormControl(_document->Selection().Selection().Start());
  }

  void EditCommand::SetStartingSelection(const VisibleSelection &selection) noexcept
  {
    for (RefPtr<EditCommand> command = ShareRefPtr(this);;
         command = ShareRefPtr<EditCommand>(command->_parent.get()))
    {
      if (auto *composition = CompositionIfPossible(*command))
      {
        composition->SetStartingSelection(selection);
      }
      command->_startingSelection = selection;
      if (!command->_parent || command->_parent->IsFirstCommand(command.get()))
      {
        break;
      }
    }
  }

  void EditCommand::SetEndingSelection(const VisibleSelection &selection) noexcept
  {
    for (RefPtr<EditCommand> command = ShareRefPtr(this); command;
         command = ShareRefPtr<EditCommand>(command->_parent.get()))
    {
      if (auto *composition = CompositionIfPossible(*command))
      {
        composition->SetEndingSelection(selection);
      }
      command->_endingSelection = selection;
    }
  }

  void EditCommand::SetParent(CompositeEditCommand *parent) noexcept
  {
    krys_debug_assert((parent && !_parent) || (!parent && _parent));
    _parent = parent ? WeakPtr<CompositeEditCommand>::WithRef(*parent) : WeakPtr<CompositeEditCommand>();
    if (_parent)
    {
      _startingSelection = _parent->_endingSelection;
      _endingSelection = _parent->_endingSelection;
    }
  }

  SimpleEditCommand::SimpleEditCommand(dom::Document &document, EditAction editingAction) noexcept
      : EditCommand(document, editingAction)
  {
  }

  void SimpleEditCommand::DoReapply() noexcept
  {
    DoApply();
  }
}
