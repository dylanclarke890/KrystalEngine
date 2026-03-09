#include "Krystal.HTML/Events/EventTarget.hpp"
#include "Krystal.HTML/Abort/AbortSignal.hpp"
#include "Krystal.HTML/Events/EventDispatcher.hpp"
#include "Krystal.HTML/Events/EventNames.hpp"
#include "Krystal.HTML/Node/CustomElementRegistry.hpp"
#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Node/Node.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"

namespace Krys::HTML
{
  namespace
  {
    /// @see https://dom.spec.whatwg.org/#default-passive-value
    static bool DefaultPassiveValue(const DOMStringAtom &type, const EventTarget &eventTarget) noexcept
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
      }

      // TODO(impl): we don't have document/body elements yet.
      // return true if:
      //  - or is a node whose node document’s document element is eventTarget,
      //  - or is a node whose node document’s body element is eventTarget.[HTML]

      return false;
    }
  }

  bool EventTarget::AddEventListener(DOMStringAtom type, RefPtr<EventListener> &&callback,
                                     const AddEventListenerOptionsOrBool &optionsOrBool) noexcept
  {
    AddEventListenerOptions options;
    if (std::holds_alternative<bool>(optionsOrBool))
    {
      options.Capture = std::get<bool>(optionsOrBool);
    }
    else
    {
      options = std::get<AddEventListenerOptions>(optionsOrBool);
    }

    // SPEC-VIOLATION(DOM, SERVICE-WORKERS): we don't implement service workers.
    // If eventTarget is a ServiceWorkerGlobalScope object, its service worker’s script resource’s has ever
    // been evaluated flag is set, and listener’s type matches the type attribute value of any of the service
    // worker events, then report a warning to the console that this might not give the expected results.

    if (options.Signal.get() != nullptr && options.Signal->Aborted())
    {
      return false;
    }

    if (callback == nullptr)
    {
      return false;
    }

    auto passive = options.Passive.has_value() ? options.Passive.value() : DefaultPassiveValue(type, *this);

    auto alreadyExists = [&](const Ref<RegisteredEventListener> &listener)
    {
      if (listener->GetType() == type && &listener->Callback() == callback.get()
          && listener->UseCapture() == options.Capture)
      {
        return true;
      }

      return false;
    };

    if (const auto &existing = std::ranges::find_if(_listeners, std::move(alreadyExists));
        existing != _listeners.end())
    {
      return false;
    }

    Ref<RegisteredEventListener> listener = CreateRef<RegisteredEventListener>(
      type,
      RegisteredEventListener::Options {
        .Capture = options.Capture, .Passive = passive, .Once = options.Once, .Signal = options.Signal},
      std::move(callback));

    _listeners.push_back(listener);

    if (options.Signal.get() != nullptr)
    {
      auto onAbort = [weakListener = CreateWeakPtr(listener.get()), weakThis = CreateWeakPtr(this)]() mutable
      {
        auto strongListener = ShareRefPtr(weakListener.get());
        auto strongThis = ShareRefPtr(weakThis.get());

        if (strongListener && strongThis)
        {
          strongThis->RemoveEventListener(strongListener->GetType(), ShareRefPtr(&strongListener->Callback()),
                                          strongListener->UseCapture());
        }
      };

      options.Signal->OnAbort(onAbort);
    }

    return true;
  }

  bool EventTarget::RemoveEventListener(DOMStringAtom type, RefPtr<EventListener> &&callback,
                                        const EventListenerOptionsOrBool &optionsOrBool) noexcept
  {
    EventListenerOptions options;
    if (std::holds_alternative<bool>(optionsOrBool))
    {
      options.Capture = std::get<bool>(optionsOrBool);
    }
    else
    {
      options = std::get<EventListenerOptions>(optionsOrBool);
    }

    auto alreadyExists = [&](const Ref<RegisteredEventListener> &listener)
    {
      if (listener->GetType() == type && &listener->Callback() == callback.get()
          && listener->UseCapture() == options.Capture)
      {
        return true;
      }

      return false;
    };

    const auto &existing = std::ranges::find_if(_listeners, std::move(alreadyExists));
    if (existing == _listeners.end())
    {
      return false;
    }

    // SPEC-VIOLATION(DOM, SERVICE-WORKERS): we don't implement service workers.
    // If eventTarget is a ServiceWorkerGlobalScope object and its service worker’s set of event types to
    // handle contains listener’s type, then report a warning to the console that this might not give the
    // expected results.

    (*existing)->MarkAsRemoved();
    _listeners.erase(existing);

    return true;
  }

  ExceptionOr<bool> EventTarget::DispatchEvent(Event &event) noexcept
  {
    if (event._dispatched)
    {
      return Exception(ExceptionCode::InvalidStateError);
    }

    event._isTrusted = false;
    return EventDispatcher::DispatchToTarget(event, *this);
  }

  void EventTarget::RemoveAllEventListeners() noexcept
  {
    for (const auto &listener : _listeners)
    {
      // SPEC-VIOLATION(DOM, SERVICE-WORKERS): we don't implement service workers.
      // If eventTarget is a ServiceWorkerGlobalScope object and its service worker’s set of event types to
      // handle contains listener’s type, then report a warning to the console that this might not give the
      // expected results.

      listener->MarkAsRemoved();
    }

    _listeners.clear();
  }
}