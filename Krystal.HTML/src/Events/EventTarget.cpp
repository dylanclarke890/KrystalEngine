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
    static bool DefaultPassiveValue(const EventTarget &, const StringAtom &type) noexcept
    {
      using namespace EventNames;
      if (type == TouchStart || type == TouchMove || type == Wheel || type == MouseWheel)
      {
        return true;
      }

      // TODO(IMPL): return true if:
      //  - eventTarget is a Window object,
      //  - or is a node whose node document is eventTarget,
      //  - or is a node whose node document’s document element is eventTarget,
      //  - or is a node whose node document’s body element is eventTarget.[HTML]

      return false;
    }
  }

  bool EventTarget::AddEventListener(DOMStringAtom type, RefPtr<EventListener> &&callback,
                                     const AddEventListenerOptions &options) noexcept
  {
    if (options.Signal && options.Signal->Aborted())
    {
      return false;
    }

    if (callback == nullptr)
    {
      return false;
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

    if (const auto &existing = std::ranges::find_if(_listeners, std::move(alreadyExists));
        existing != _listeners.end())
    {
      return false;
    }

    Ref<RegisteredEventListener> listener = CreateRef<RegisteredEventListener>(
      type,
      RegisteredEventListener::Options {
        .Capture = options.Capture,
        .Passive = options.Passive.value_or(DefaultPassiveValue(*this, type)),
        .Once = options.Once,
      },
      std::move(callback));

    _listeners.push_back(listener);

    // TODO(IMPL): add abort signal handling -> https://dom.spec.whatwg.org/#add-an-event-listener (step 6)

    return true;
  }

  bool EventTarget::RemoveEventListener(DOMStringAtom type, RefPtr<EventListener> &&callback,
                                        const EventListenerOptions &options) noexcept
  {
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

    _listeners.erase(existing);

    return true;
  }

  void EventTarget::RemoveAllEventListeners() noexcept
  {
    _listeners.clear();
  }

  ExceptionOr<bool> EventTarget::DispatchEvent(Event &event) noexcept
  {
    if (event._dispatched)
    {
      return Exception(ExceptionCode::InvalidStateError);
    }

    event._dispatched = true;
    return EventDispatcher::DispatchTo(*this, event);
  }
}