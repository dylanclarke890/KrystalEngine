#include "Krystal.Web/Html/Events/CommandEvent.hpp"
#include "Krystal.Web/Dom/Algorithms/EventTargetAlgorithms.hpp"
#include "Krystal.Web/Dom/Nodes/Document.hpp"
#include "Krystal.Web/Dom/Nodes/Element.hpp"

namespace krys::web
{
  CommandEvent::CommandEvent(DOMStringAtom type, const CommandEventInit &eventInitDict) noexcept
      : Event(krys::move(type), eventInitDict), _source(ShareRefPtr(eventInitDict.Source)),
        _command(eventInitDict.Command)
  {
  }

  Ref<CommandEvent> CommandEvent::Create(const DOMStringAtom &type, const CommandEventInit &init,
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
      DynamicDowncast<Node>(dom::EventTargetAlgorithms::Retarget(_source.get(), *target)));
  }
}
