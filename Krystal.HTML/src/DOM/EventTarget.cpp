#include "Krystal.HTML/DOM/EventTarget.hpp"
#include "Krystal.HTML/Abort/AbortSignal.hpp"
#include "Krystal.HTML/Algorithms/EventDispatcher.hpp"
#include "Krystal.HTML/Constants/EventNames.hpp"
#include "Krystal.HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/DOM/Algorithms/EventTargetAlgorithms.hpp"
#include "Krystal.HTML/HTMLElement/HTMLSlotElement.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.HTML/Node/Node.hpp"
#include "Krystal.HTML/Node/NodeList.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"
#include <ranges>

namespace Krys::HTML
{
  void EventTarget::AddEventListener(DOMStringAtom type, Ref<EventListenerCallback> &&callback,
                                     const AddEventListenerOptionsOrBool &optionsOrBool) noexcept
  {
    AddEventListenerOptions options = EventTargetAlgorithms::FlattenMoreOptions(optionsOrBool);
    auto listener = CreateRef<EventListener>(type, Krys::Move(callback), Krys::Move(options));
    EventTargetAlgorithms::AddEventListener(*this, *listener.get());
  }

  void EventTarget::RemoveEventListener(DOMStringAtom type, Ref<EventListenerCallback> &&callback,
                                        const EventListenerOptionsOrBool &optionsOrBool) noexcept
  {
    EventListenerOptions options = EventTargetAlgorithms::FlattenOptions(optionsOrBool);

    auto AlreadyExists = [&](const Ref<EventListener> &listener)
    {
      return (listener->Type() == type && listener->Callback() == callback.get()
              && listener->Capture() == options.Capture);
    };

    const auto &existing = std::ranges::find_if(_eventListenerList, std::move(AlreadyExists));
    if (existing != _eventListenerList.end())
    {
      EventTargetAlgorithms::RemoveEventListener(*this, *existing->get());
    }
  }

  ExceptionOr<bool> EventTarget::DispatchEvent(Event &event) noexcept
  {
    if (event._dispatched || !event._initialized)
    {
      return Exception(ExceptionCode::InvalidStateError);
    }

    event._isTrusted = false;
    return EventDispatcher::DispatchToTarget(event, this);
  }
}