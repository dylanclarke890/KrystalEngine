#pragma once

#include "Krystal.Booey/DOM/Callbacks/EventListenerCallback.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/Func.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::dom
{
  class Event;
  class EventListener;
  class EventTarget;

  /// @brief An event listener callback that calls a function, for listeners added from C++.
  class FunctionEventListenerCallback final : public EventListenerCallback
  {
  private:
    Func<void(Event &)> _function;

  public:
    explicit FunctionEventListenerCallback(Func<void(Event &)> &&function) noexcept
        : _function(krys::move(function))
    {
    }

    void HandleEvent(Event &event) noexcept override
    {
      _function(event);
    }
  };

  /// @brief Owns an event listener added from C++: removes it when it's destroyed, unless it was released to
  /// its target.
  /// @note Removing a listener that's already gone (removed another way, run once, aborted by its signal, or
  /// dropped with its target) does nothing. The handle holds neither the target nor the listener alive.
  class KRYS_NODISCARD EventListenerHandle
  {
  private:
    WeakPtr<EventTarget> _target;
    WeakPtr<EventListener> _listener;

  public:
    /// @brief A handle that owns no listener.
    EventListenerHandle() noexcept = default;
    EventListenerHandle(EventTarget &target, EventListener &listener) noexcept;
    EventListenerHandle(EventListenerHandle &&other) noexcept;
    EventListenerHandle &operator=(EventListenerHandle &&other) noexcept;
    EventListenerHandle(const EventListenerHandle &) = delete;
    EventListenerHandle &operator=(const EventListenerHandle &) = delete;
    ~EventListenerHandle() noexcept;

    /// @brief Removes the listener now, if it's still registered; the handle then owns none.
    void Remove() noexcept;

    /// @brief Gives the listener up to its target, which keeps it for as long as the target lives (or until
    /// it's removed another way); the handle then owns none.
    void Release() noexcept;

    /// @brief Whether the handle owns a listener that's still registered.
    KRYS_NODISCARD bool IsActive() const noexcept;

    KRYS_NODISCARD explicit operator bool() const noexcept
    {
      return IsActive();
    }
  };
}
