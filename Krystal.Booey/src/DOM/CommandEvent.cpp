#include "Krystal.Booey/DOM/CommandEvent.hpp"
#include "Krystal.Booey/DOM/Algorithms/EventTargetAlgorithms.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"

namespace krys::boo::dom
{
  CommandEvent::CommandEvent(DOMStringAtom type, const CommandEventInit &eventInitDict) noexcept
      : Event(type, eventInitDict), _source(eventInitDict.Source), _command(eventInitDict.Command)
  {
  }

  Ref<CommandEvent> CommandEvent::Create(DOMStringAtom type, const CommandEventInit &init,
                                         bool isTrusted) noexcept
  {
    auto event = CreateRef<CommandEvent>(type, init);
    event->_isTrusted = isTrusted;
    return event;
  }

  Element *CommandEvent::Source() const noexcept
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
