#include "Krystal.Booey/HTML/Events/SubmitEvent.hpp"
#include "Krystal.Booey/DOM/Events/EventNames.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLElement.hpp"

namespace krys::boo::html
{
  SubmitEvent::SubmitEvent(dom::DOMStringAtom type, const SubmitEventInit &eventInitDict) noexcept
      : Event(type, eventInitDict), _submitter(eventInitDict.Submitter)
  {
  }

  Ref<SubmitEvent> SubmitEvent::Create(RefPtr<HTMLElement> &&submitter) noexcept
  {
    SubmitEventInit init;
    init.Bubbles = true;
    init.Cancelable = true;
    init.Submitter = krys::move(submitter);
    auto event = CreateRef<SubmitEvent>(dom::DOMStringAtom(EventNames::Submit), init);
    event->_isTrusted = true;
    return event;
  }
}
