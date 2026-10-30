#include "Krystal.Booey/HTML/BeforeUnloadEvent.hpp"
#include "Krystal.Booey/Constants/EventNames.hpp"

namespace krys::boo::html
{
  namespace
  {
    KRYS_NODISCARD dom::EventInit CancelableInit() noexcept
    {
      dom::EventInit init;
      init.Cancelable = true;
      return init;
    }
  }

  BeforeUnloadEvent::BeforeUnloadEvent() noexcept
      : Event(dom::DOMStringAtom(EventNames::BeforeUnload), CancelableInit())
  {
  }

  Ref<BeforeUnloadEvent> BeforeUnloadEvent::Create() noexcept
  {
    auto event = CreateRef<BeforeUnloadEvent>();
    event->_isTrusted = true;
    return event;
  }
}
