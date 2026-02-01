#include "Krystal.HTML/Events/EventTarget.hpp"
#include "Krystal.HTML/Events/AbortSignal.hpp"
#include "Krystal.HTML/Events/EventNames.hpp"

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

      return false;
    }
  }

  EventTarget::EventTarget() noexcept = default;

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

  bool EventTarget::DispatchEvent(Event &event) noexcept
  {
    // TODO(IMPL): https://dom.spec.whatwg.org/#concept-event-dispatch
    return false;
  }
}