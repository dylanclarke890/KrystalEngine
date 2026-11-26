#include "Krystal.Web/Html/Events/ToggleEvent.hpp"
#include "Krystal.Web/Dom/Events/EventTargetAlgorithms.hpp"
#include "Krystal.Web/Dom/Nodes/Document.hpp"
#include "Krystal.Web/Dom/Nodes/Element.hpp"

namespace krys::web
{
  ToggleEvent::ToggleEvent(DOMStringAtom type, const ToggleEventInit &eventInitDict) noexcept
      : Event(krys::move(type), eventInitDict), _oldState(eventInitDict.OldState),
        _newState(eventInitDict.NewState), _source(ShareRefPtr(eventInitDict.Source))
  {
  }

  Ref<ToggleEvent> ToggleEvent::Create(const DOMStringAtom &type, const ToggleEventInit &init,
                                       bool isCancelable) noexcept
  {
    ToggleEventInit eventInit = init;
    eventInit.Bubbles = false;
    eventInit.Cancelable = isCancelable;
    auto event = CreateRef<ToggleEvent>(type, eventInit);
    event->_isTrusted = true;
    return event;
  }

  Element *ToggleEvent::Source() const noexcept
  {
    if (!_source)
    {
      return nullptr;
    }
    // Retarget against the current target, or the source's document outside dispatch.
    RefPtr<EventTarget> currentTarget = CurrentTarget();
    EventTarget *target = currentTarget.get();
    if (!target || !Is<Node>(*target))
    {
      target = &_source->NodeDocument();
    }
    return DynamicDowncast<Element>(
      DynamicDowncast<Node>(dom::EventTargetAlgorithms::Retarget(_source.get(), *target)));
  }
}
