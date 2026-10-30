#include "Krystal.Booey/Editing/TextInsertionBaseCommand.hpp"
#include "Krystal.Booey/DOM/Algorithms/EventDispatcher.hpp"
#include "Krystal.Booey/DOM/BeforeTextInsertedEvent.hpp"
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

  utf8_string DispatchBeforeTextInsertedEvent(utf8_stringview text,
                                              const VisibleSelection &selectionForInsertion,
                                              bool insertionIsForUpdatingComposition) noexcept
  {
    if (insertionIsForUpdatingComposition)
    {
      return utf8_string(text);
    }

    utf8_string newText(text);
    if (RefPtr<dom::Node> startNode = ShareRefPtr(selectionForInsertion.Start().GetContainerNode()))
    {
      if (RefPtr<dom::Element> rootEditableElement = ShareRefPtr(startNode->RootEditableElement()))
      {
        // Send BeforeTextInsertedEvent. The event handler will update text if necessary.
        auto event = dom::BeforeTextInsertedEvent::Create(text);
        (void)dom::EventDispatcher::DispatchToTarget(*event, rootEditableElement.get());
        newText = event->Text();
      }
    }
    return newText;
  }

  bool CanAppendNewLineFeedToSelection(const VisibleSelection &selection) noexcept
  {
    RefPtr<dom::Element> node = ShareRefPtr(selection.RootEditableElement());
    if (!node)
    {
      return false;
    }

    auto event = dom::BeforeTextInsertedEvent::Create(u8"\n");
    (void)dom::EventDispatcher::DispatchToTarget(*event, node.get());
    return !event->Text().empty();
  }
}
