#include "Krystal.Booey/DOM/EventTarget.hpp"
#include "Krystal.Booey/Constants/EventNames.hpp"
#include "Krystal.Booey/DOM/AbortSignal.hpp"
#include "Krystal.Booey/DOM/Algorithms/EventDispatcher.hpp"
#include "Krystal.Booey/DOM/Algorithms/EventTargetAlgorithms.hpp"
#include "Krystal.Booey/DOM/Attr.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/Node.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/HTML/HTMLSlotElement.hpp"
#include <ranges>

namespace krys::boo::dom
{
  void EventTarget::AddEventListener(DOMStringAtom type, Ref<EventListenerCallback> &&callback,
                                     const AddEventListenerOptionsOrBool &optionsOrBool) noexcept
  {
    AddEventListenerOptions options = EventTargetAlgorithms::FlattenMoreOptions(optionsOrBool);
    auto listener = CreateRef<EventListener>(type, krys::move(callback), krys::move(options));
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