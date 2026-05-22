#include "Krystal.HTML/Events/EventTarget.hpp"
#include "Krystal.HTML/Abort/AbortSignal.hpp"
#include "Krystal.HTML/Algorithms/EventDispatcher.hpp"
#include "Krystal.HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/Events/EventNames.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.HTML/Node/Node.hpp"
#include "Krystal.HTML/Node/NodeList.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"

namespace Krys::HTML
{
  void EventTarget::AddEventListener(DOMStringAtom type, Ref<EventListener> &&callback,
                                     const AddEventListenerOptionsOrBool &optionsOrBool) noexcept
  {
    AddEventListenerOptions options = FlattenMoreOptions(optionsOrBool);
    auto listener = CreateRef<RegisteredEventListener>(type, Krys::Move(options), Krys::Move(callback));
    AddEventListener(*this, *listener.get());
  }

  void EventTarget::RemoveEventListener(DOMStringAtom type, Ref<EventListener> &&callback,
                                        const EventListenerOptionsOrBool &optionsOrBool) noexcept
  {
    EventListenerOptions options = FlattenOptions(optionsOrBool);

    auto AlreadyExists = [&](const Ref<RegisteredEventListener> &listener)
    {
      return (listener->Type() == type && listener->Callback() == callback.get()
              && listener->Capture() == options.Capture);
    };

    const auto &existing = std::ranges::find_if(_listeners, std::move(AlreadyExists));
    if (existing != _listeners.end())
    {
      RemoveEventListener(*this, *(*existing));
    }
  }

  ExceptionOr<bool> EventTarget::DispatchEvent(Event &event) noexcept
  {
    if (event._dispatched)
    {
      return Exception(ExceptionCode::InvalidStateError);
    }

    event._isTrusted = false;
    return EventDispatcher::DispatchToTarget(event, this);
  }

  void EventTarget::AddEventListener(EventTarget &eventTarget, RegisteredEventListener &listener) noexcept
  {
    // SPEC-VIOLATION(DOM, SERVICE-WORKERS): service workers currently not supported.
    // "If eventTarget is a ServiceWorkerGlobalScope object, its service worker’s script resource’s has ever
    // been evaluated flag is set, and listener’s type matches the type attribute value of any of the service
    // worker events, then report a warning to the console that this might not give the expected results."

    if (listener.Signal() != nullptr && listener.Signal()->Aborted())
    {
      return;
    }

    if (listener.Callback() == nullptr)
    {
      return;
    }

    if (!listener.Passive().has_value())
    {
      listener.Passive(DefaultPassiveValue(listener.Type(), eventTarget));
    }

    auto AlreadyExists = [&](const Ref<RegisteredEventListener> &l)
    {
      return (l->Type() == listener.Type() && l->Callback() == listener.Callback()
              && l->Capture() == listener.Capture());
    };

    if (const auto &existing = std::ranges::find_if(eventTarget._listeners, std::move(AlreadyExists));
        existing == eventTarget._listeners.end())
    {
      eventTarget._listeners.push_back(ShareRef(listener));
    }

    if (listener.Signal() != nullptr)
    {
      auto algorithm =
        [weakListener = CreateWeakPtr(&listener), weakTarget = CreateWeakPtr(&eventTarget)](Any)
      {
        auto strongListener = weakListener.lock();
        auto strongTarget = weakTarget.lock();

        if (strongListener && strongTarget)
        {
          EventTarget::RemoveEventListener(*strongTarget, *strongListener);
        }
      };

      AbortSignal::Add(algorithm, *listener.Signal());
    }
  }

  void EventTarget::RemoveEventListener(EventTarget &eventTarget, RegisteredEventListener &listener) noexcept
  {
    // SPEC-VIOLATION(DOM, SERVICE-WORKERS): service workers currently not supported.
    // "If eventTarget is a ServiceWorkerGlobalScope object and its service worker’s set of event types to
    // handle contains listener’s type, then report a warning to the console that this might not give the
    // expected results."

    listener.Remove();
    const auto &existing = std::ranges::find_if(
      eventTarget._listeners, [&](const Ref<RegisteredEventListener> &l) { return l.get() == &listener; });

    if (existing != eventTarget._listeners.end())
    {
      eventTarget._listeners.erase(existing);
    }
  }

  void EventTarget::RemoveAllEventListeners(EventTarget &eventTarget) noexcept
  {
    while (!eventTarget._listeners.empty())
    {
      RemoveEventListener(eventTarget, *eventTarget._listeners.front().get());
    }
  }

  bool EventTarget::DefaultPassiveValue(const DOMStringAtom &type, const EventTarget &eventTarget) noexcept
  {
    using namespace EventNames;
    if (type != TouchStart || type != TouchMove || type != Wheel || type != MouseWheel)
    {
      return false;
    }

    if (eventTarget.IsWindow())
    {
      return true;
    }

    if (RawPtr<const Node> node = DynamicDowncast<Node>(eventTarget))
    {
      if (&node->NodeDocument() == &eventTarget)
      {
        return true;
      }

      if (node->NodeDocument().DocumentElement().get() == &eventTarget)
      {
        return true;
      }
    }

    // TODO(impl): REQUIRED - return true if node is a node whose node document’s body element is eventTarget.

    return false;
  }
}