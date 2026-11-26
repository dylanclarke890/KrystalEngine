#include "Krystal.Web/Html/Events/SubmitEvent.hpp"
#include "Krystal.Web/Dom/Events/EventNames.hpp"
#include "Krystal.Web/Html/Elements/HTMLElement.hpp"

namespace krys::web
{
  SubmitEvent::SubmitEvent(DOMStringAtom type, const SubmitEventInit &eventInitDict) noexcept
      : Event(krys::move(type), eventInitDict), _submitter(ShareRefPtr(eventInitDict.Submitter))
  {
  }

  Ref<SubmitEvent> SubmitEvent::Create(RefPtr<HTMLElement> &&submitter) noexcept
  {
    SubmitEventInit init;
    init.Bubbles = true;
    init.Cancelable = true;
    init.Composed = true;
    init.Submitter = submitter.get();
    auto event = CreateRef<SubmitEvent>(event_names::Submit, init);
    event->_isTrusted = true;
    return event;
  }
}
