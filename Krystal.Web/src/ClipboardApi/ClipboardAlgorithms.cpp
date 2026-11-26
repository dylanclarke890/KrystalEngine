#include "Krystal.Web/ClipboardApi/ClipboardAlgorithms.hpp"
#include "Krystal.Web/ClipboardApi/ClipboardEvent.hpp"
#include "Krystal.Web/Dom/Events/EventDispatcher.hpp"
#include "Krystal.Web/Dom/Events/EventNames.hpp"
#include "Krystal.Web/Dom/Nodes/Element.hpp"
#include "Krystal.Web/Html/DragAndDrop/DataTransfer.hpp"
#include "Krystal.Web/Platform/Pasteboard/Pasteboard.hpp"

namespace krys::web
{
  bool ClipboardAlgorithms::FireClipboardEvent(const DOMStringAtom &name, Element &target,
                                               DataTransfer &clipboardData) noexcept
  {
    auto event = ClipboardEvent::Create(name, ShareRef(clipboardData));
    (void)dom::EventDispatcher::DispatchToTarget(*event, &target);

    // A cancelled copy or cut writes what its listeners put in the clipboard data.
    bool cancelled = event->DefaultPrevented();
    if (cancelled && (name == event_names::Copy || name == event_names::Cut)
        && clipboardData.GetPasteboard().HasData())
    {
      clipboardData.CommitToPasteboard(*Pasteboard::CreateForCopyAndPaste());
    }

    clipboardData.MakeInvalidForSecurity();
    return !cancelled;
  }
}
