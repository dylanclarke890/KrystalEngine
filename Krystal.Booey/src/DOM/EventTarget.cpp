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
    // 1. Let capture, passive, once, and signal be the result of flattening more options.
    AddEventListenerOptions options = EventTargetAlgorithms::FlattenMoreOptions(optionsOrBool);

    // 2. Add an event listener with this and an event listener whose type is type, callback is callback,
    // capture is capture, passive is passive, once is once, and signal is signal.
    auto listener = CreateRef<EventListener>(type, krys::move(callback), krys::move(options));
    (void)EventTargetAlgorithms::AddEventListener(*this, *listener.get());
  }

  EventListenerHandle
    EventTarget::AddEventListener(DOMStringAtom type, Func<void(Event &)> &&callback,
                                  const AddEventListenerOptionsOrBool &optionsOrBool) noexcept
  {
    AddEventListenerOptions options = EventTargetAlgorithms::FlattenMoreOptions(optionsOrBool);
    auto listener = CreateRef<EventListener>(
      type, CreateRef<FunctionEventListenerCallback>(krys::move(callback)), krys::move(options));
    if (!EventTargetAlgorithms::AddEventListener(*this, *listener.get()))
    {
      return {};
    }
    return EventListenerHandle(*this, *listener);
  }

  void EventTarget::RemoveEventListener(DOMStringAtom type, Ref<EventListenerCallback> &&callback,
                                        const EventListenerOptionsOrBool &optionsOrBool) noexcept
  {
    // 1. Let capture be the result of flattening options.
    EventListenerOptions options = EventTargetAlgorithms::FlattenOptions(optionsOrBool);

    // 2. If this's event listener list contains an event listener whose type is type, callback is callback,
    // and capture is capture, then remove an event listener with this and that event listener.
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

  const Func<void(Event &)> *EventTarget::EventHandlerValue(DOMStringAtom name) const noexcept
  {
    if (!_eventHandlers)
    {
      return nullptr;
    }
    auto it = std::ranges::find_if(*_eventHandlers,
                                   [&](const EventHandler &handler) { return handler.Name == name; });
    return it != _eventHandlers->end() ? &it->Value : nullptr;
  }

  void EventTarget::SetEventHandlerValue(DOMStringAtom name, Func<void(Event &)> &&value) noexcept
  {
    if (!_eventHandlers)
    {
      if (!value)
      {
        return;
      }
      _eventHandlers = CreateUnique<List<EventHandler>>();
    }

    auto it = std::ranges::find_if(*_eventHandlers,
                                   [&](const EventHandler &handler) { return handler.Name == name; });

    // Deactivate: remove the event handler's listener from the target, and set its value to null.
    if (!value)
    {
      if (it != _eventHandlers->end())
      {
        RefPtr<EventListener> listener = it->Listener;
        _eventHandlers->erase(it);
        if (listener)
        {
          EventTargetAlgorithms::RemoveEventListener(*this, *listener);
        }
      }
      return;
    }

    // A handler that's already active keeps its listener, and so its place among the target's listeners.
    if (it != _eventHandlers->end())
    {
      it->Value = krys::move(value);
      return;
    }

    // Activate: a listener whose callback runs the event handler processing algorithm, which calls the
    // handler's current value.
    auto callback = CreateRef<FunctionEventListenerCallback>(
      [weakThis = CreateWeakPtr(this), name](Event &event)
      {
        RefPtr<EventTarget> target = ShareRefPtr(weakThis.get());
        if (!target)
        {
          return;
        }
        // https://html.spec.whatwg.org/#the-event-handler-processing-algorithm
        if (auto *handler = target->EventHandlerValue(name))
        {
          auto current = *handler;
          current(event);
        }
      });
    auto listener = CreateRef<EventListener>(name, krys::move(callback), AddEventListenerOptions {});
    _eventHandlers->push_back({name, krys::move(value), listener});
    (void)EventTargetAlgorithms::AddEventListener(*this, *listener);
  }

  ExceptionOr<bool> EventTarget::DispatchEvent(Event &event) noexcept
  {
    // 1. If event's dispatch flag is set, or if its initialized flag is not set, then throw an
    // "InvalidStateError" DOMException.
    if (event._dispatched || !event._initialized)
    {
      return Exception(ExceptionCode::InvalidStateError);
    }

    // 2. Initialize event's isTrusted attribute to false.
    event._isTrusted = false;

    // 3. Return the result of dispatching event to this.
    return EventDispatcher::DispatchToTarget(event, this);
  }
}