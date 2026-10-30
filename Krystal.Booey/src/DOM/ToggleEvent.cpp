#include "Krystal.Booey/DOM/ToggleEvent.hpp"
#include "Krystal.Booey/DOM/Algorithms/EventTargetAlgorithms.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"

namespace krys::boo::dom
{
  ToggleEvent::ToggleEvent(DOMStringAtom type, const ToggleEventInit &eventInitDict) noexcept
      : Event(type, eventInitDict), _oldState(eventInitDict.OldState), _newState(eventInitDict.NewState),
        _source(eventInitDict.Source)
  {
  }

  Ref<ToggleEvent> ToggleEvent::Create(DOMStringAtom type, const ToggleEventInit &init,
                                       bool isCancelable) noexcept
  {
    ToggleEventInit eventInit = init;
    eventInit.Bubbles = false;
    eventInit.Cancelable = isCancelable;
    eventInit.Composed = false;
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
      DynamicDowncast<Node>(EventTargetAlgorithms::Retarget(_source.get(), *target)));
  }
}
