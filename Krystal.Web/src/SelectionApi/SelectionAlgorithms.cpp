#include "Krystal.Web/SelectionApi/SelectionAlgorithms.hpp"
#include "Krystal.Web/Dom/Events/Event.hpp"
#include "Krystal.Web/Dom/Events/EventDispatcher.hpp"
#include "Krystal.Web/Dom/Events/EventNames.hpp"
#include "Krystal.Web/Dom/Nodes/Document.hpp"
#include "Krystal.Web/Editing/Selection/FrameSelection.hpp"
#include "Krystal.Web/Html/Elements/HTMLTextFormControlElement.hpp"
#include "Krystal.Web/Html/EventLoop/EventLoop.hpp"

namespace krys::web
{
  void SelectionAlgorithms::ScheduleSelectionChangeEvent(Document &document) noexcept
  {
    auto &selection = document.Selection();
    if (selection._hasScheduledSelectionChangeEventOnDocument)
    {
      return;
    }

    selection._hasScheduledSelectionChangeEventOnDocument = true;
    html::EventLoop::Current().QueueTask(
      html::TaskSource::UserInteraction, document,
      [weakDocument = CreateWeakPtr(&document)]
      {
        if (RefPtr<Document> document = ShareRefPtr(weakDocument.get()))
        {
          document->Selection()._hasScheduledSelectionChangeEventOnDocument = false;
          (void)dom::EventDispatcher::FireEvent(event_names::SelectionChange, *document);
        }
      });
  }

  void SelectionAlgorithms::ScheduleSelectionChangeEvent(HTMLTextFormControlElement &textControl) noexcept
  {
    if (textControl._hasScheduledSelectionChangeEvent)
    {
      return;
    }

    textControl._hasScheduledSelectionChangeEvent = true;
    html::EventLoop::Current().QueueTask(
      html::TaskSource::UserInteraction, textControl.NodeDocument(),
      [textControl = ShareRef(textControl)]
      {
        textControl->_hasScheduledSelectionChangeEvent = false;
        auto event = Event::Create(event_names::SelectionChange, true, false);
        (void)dom::EventDispatcher::DispatchToTarget(*event, textControl.get());
      });
  }
}
