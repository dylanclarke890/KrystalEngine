#pragma once

#include "Krystal.Booey/DOM/Dicts/AddEventListenerOptions.hpp"
#include "Krystal.Booey/DOM/Dicts/EventListenerOptions.hpp"
#include "Krystal.Booey/DOM/Enums/EventTargetFlags.hpp"
#include "Krystal.Booey/DOM/EventListenerHandle.hpp"
#include "Krystal.Booey/DOM/Internals/Event/EventListener.hpp"
#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Core/CanMakeWeakPtr.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/Func.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/SmallList.hpp"
#include "Krystal.Core/Types/Span.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo::dom
{
  /// @see https://dom.spec.whatwg.org/#eventtarget
  class EventTarget : public RefCounted<EventTarget>, public CanMakeWeakPtr<EventTarget>
  {
    KRYS_TYPE_CAST_TRAITS_ACCESS();

    friend class EventDispatcher;
    friend class EventTargetAlgorithms;

  private:
    EventTargetFlags _flags : BitCount<EventTargetFlags>() {EventTargetFlags::None};

    /// @see https://dom.spec.whatwg.org/#eventtarget-event-listener-list
    SmallList<Ref<EventListener>> _eventListenerList;

    /// @brief An event handler: its value and the listener that calls it, added when it was first set.
    /// @see https://html.spec.whatwg.org/#event-handlers
    struct EventHandler
    {
      DOMStringAtom Name;
      Func<void(Event &)> Value;
      RefPtr<EventListener> Listener;
    };

    /// @brief The target's event handlers (its event handler map), made when one is first set.
    /// @see https://html.spec.whatwg.org/#event-handler-map
    UniquePtr<List<EventHandler>> _eventHandlers;

  protected:
    EventTarget(EventTargetFlags flags) noexcept : _flags(flags)
    {
    }

  public:
    virtual ~EventTarget() noexcept = default;

    /// @brief Drops a reference; the last one calls `RemovedLastRef`.
    void SubRef() const noexcept
    {
      if (SubRefBase())
      {
        const_cast<EventTarget *>(this)->RemovedLastRef();
      }
    }

#pragma region EventTarget - https://dom.spec.whatwg.org/#eventtarget

    /// @see https://dom.spec.whatwg.org/#dom-eventtarget-eventtarget
    EventTarget() noexcept = default;

    /// @see https://dom.spec.whatwg.org/#dom-eventtarget-addeventlistener
    /// @note The target holds its listeners' callbacks: a callback that holds a reference to the target (or
    /// to a node of its document) keeps both alive, as there's no garbage collector to find the cycle.
    void AddEventListener(DOMStringAtom type, Ref<EventListenerCallback> &&callback,
                          const AddEventListenerOptionsOrBool &options) noexcept;

    /// @brief Adds a listener that calls @p callback, owned by the returned handle: destroying the handle
    /// removes the listener, unless it was released to the target.
    /// @note Listeners released to their target can still be removed together by a signal in @p options.
    KRYS_NODISCARD EventListenerHandle
      AddEventListener(DOMStringAtom type, Func<void(Event &)> &&callback,
                       const AddEventListenerOptionsOrBool &options = false) noexcept;

    /// @brief Adds a listener that calls @p callback for the events of type @p type that are a @p TEvent (a
    /// `MouseEvent`, a `KeyboardEvent`), owned by the returned handle.
    template <typename TEvent>
    requires(DerivedFrom<TEvent, Event> && !SameType<TEvent, Event>)
    KRYS_NODISCARD EventListenerHandle
      AddEventListener(DOMStringAtom type, Func<void(TEvent &)> &&callback,
                       const AddEventListenerOptionsOrBool &options = false) noexcept
    {
      return AddEventListener(type,
                              Func<void(Event &)>(
                                [callback = krys::move(callback)](Event &event)
                                {
                                  if (auto *typedEvent = DynamicDowncast<TEvent>(event))
                                  {
                                    callback(*typedEvent);
                                  }
                                }),
                              options);
    }

    /// @see https://dom.spec.whatwg.org/#dom-eventtarget-removeeventlistener
    void RemoveEventListener(DOMStringAtom type, Ref<EventListenerCallback> &&callback,
                             const EventListenerOptionsOrBool &options) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-eventtarget-dispatchevent
    ExceptionOr<bool> DispatchEvent(Event &event) noexcept;

    /// @brief The value of the target's own event handler @p name; null if it's not set.
    /// @see https://html.spec.whatwg.org/#getting-the-current-value-of-the-event-handler
    KRYS_NODISCARD const Func<void(Event &)> *EventHandlerValue(DOMStringAtom name) const noexcept;

    /// @brief Sets the target's own event handler @p name to @p value, activating it (adding its listener)
    /// the first time; an empty @p value deactivates it (removing its listener).
    /// @see https://html.spec.whatwg.org/#activate-an-event-handler
    /// @see https://html.spec.whatwg.org/#deactivate-an-event-handler
    void SetEventHandlerValue(DOMStringAtom name, Func<void(Event &)> &&value) noexcept;

    /// @brief The target's event listener list, in the order they were added.
    /// @see https://dom.spec.whatwg.org/#eventtarget-event-listener-list
    KRYS_NODISCARD Span<const Ref<EventListener>> EventListeners() const noexcept
    {
      return _eventListenerList;
    }

  protected:
    /// @brief The last reference was dropped: deletes the target, unless something else keeps it (a document
    /// that nodes still refer to).
    virtual void RemovedLastRef() noexcept
    {
      delete this;
    }

    /// @brief @p listener was added to the target's listeners.
    virtual void DidAddEventListener(const EventListener &listener) noexcept
    {
      (void)listener;
    }

    /// @brief @p listener was removed from the target's listeners.
    virtual void DidRemoveEventListener(const EventListener &listener) noexcept
    {
      (void)listener;
    }

    /// @see https://dom.spec.whatwg.org/#get-the-parent
    KRYS_NODISCARD virtual EventTarget *GetParent(Event &event) const noexcept
    {
      (void)event;
      return nullptr;
    }

#pragma region Activation Behavior

    /// @see https://dom.spec.whatwg.org/#eventtarget-activation-behavior
    KRYS_NODISCARD bool HasActivationBehavior() const noexcept
    {
      return HasFlag(_flags, EventTargetFlags::HasActivationBehavior);
    }

    /// @see https://dom.spec.whatwg.org/#eventtarget-activation-behavior
    virtual void ActivationBehavior(Event &event) noexcept
    {
      (void)event;
    }

    /// @see https://dom.spec.whatwg.org/#eventtarget-legacy-pre-activation-behavior
    KRYS_NODISCARD bool HasLegacyPreActivationBehavior() const noexcept
    {
      return HasFlag(_flags, EventTargetFlags::HasLegacyPreActivationBehavior);
    }

    /// @see https://dom.spec.whatwg.org/#eventtarget-legacy-pre-activation-behavior
    virtual void LegacyPreActivationBehavior(Event &event) noexcept
    {
      (void)event;
    }

    /// @see https://dom.spec.whatwg.org/#eventtarget-legacy-canceled-activation-behavior
    KRYS_NODISCARD bool HasLegacyCanceledActivationBehavior() const noexcept
    {
      return HasFlag(_flags, EventTargetFlags::HasLegacyCanceledActivationBehavior);
    }

    /// @see https://dom.spec.whatwg.org/#eventtarget-legacy-canceled-activation-behavior
    virtual void LegacyCanceledActivationBehavior(Event &event) noexcept
    {
      (void)event;
    }

#pragma endregion

#pragma endregion

#pragma region Type Checks

    KRYS_NODISCARD bool IsNode() const noexcept
    {
      return HasEventTargetFlag(EventTargetFlags::IsNode);
    }

    KRYS_NODISCARD bool IsWindow() const noexcept
    {
      return HasEventTargetFlag(EventTargetFlags::IsWindow);
    }

#pragma endregion

#pragma region EventTarget Flags

    KRYS_NODISCARD bool HasEventTargetFlag(EventTargetFlags flags) const noexcept
    {
      return HasFlag(_flags, flags);
    }

    void SetEventTargetFlag(EventTargetFlags flags) noexcept
    {
      _flags = _flags | flags;
    }

    void ClearEventTargetFlag(EventTargetFlags flags) noexcept
    {
      _flags = _flags & ~flags;
    }

#pragma endregion
  };
}