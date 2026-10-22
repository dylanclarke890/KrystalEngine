#include "Krystal.Booey/Editing/TextInsertionBaseCommand.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/Editing/FrameSelection.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"

namespace krys::boo
{
  TextInsertionBaseCommand::TextInsertionBaseCommand(dom::Document &document,
                                                     EditAction editingAction) noexcept
      : CompositeEditCommand(document, editingAction)
  {
  }

  void TextInsertionBaseCommand::ApplyTextInsertionCommand(LocalFrame *frame,
                                                           TextInsertionBaseCommand &command,
                                                           const VisibleSelection &selectionForInsertion,
                                                           const VisibleSelection &endingSelection) noexcept
  {
    bool changeSelection = selectionForInsertion != endingSelection;
    if (changeSelection)
    {
      command.SetStartingSelection(selectionForInsertion);
      command.SetEndingSelection(selectionForInsertion);
    }
    command.Apply();
    if (changeSelection)
    {
      command.SetEndingSelection(endingSelection);
      frame->Document()->Selection().SetSelection(endingSelection);
    }
  }

  // FIXME: The event text controls handle to filter the text they're given (BeforeTextInsertedEvent), with
  // form controls; until then the text is inserted as it is.
  utf8_string DispatchBeforeTextInsertedEvent(utf8_stringview text, const VisibleSelection &, bool) noexcept
  {
    return utf8_string(text);
  }

  // FIXME: Asking text controls whether they'd take a newline (BeforeTextInsertedEvent), with form controls;
  // until then any editable root takes one.
  bool CanAppendNewLineFeedToSelection(const VisibleSelection &selection) noexcept
  {
    return selection.RootEditableElement();
  }
}
