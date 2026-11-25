#pragma once

#include "Krystal.Booey/DOM/Events/EventListener.hpp"
#include "Krystal.Booey/DOM/Events/EventListenerHandle.hpp"
#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Core/Containers/EnumSet.hpp"
#include "Krystal.Core/Containers/List.hpp"
#include "Krystal.Core/Containers/SmallList.hpp"
#include "Krystal.Core/Containers/Span.hpp"
#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Memory/CanMakeWeakPtr.hpp"
#include "Krystal.Core/Memory/RefCounted.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Core/Memory/UniquePtr.hpp"
#include "Krystal.Core/Types/Func.hpp"

namespace krys::boo::dom
{
  class EventDispatcher;
  class EventTargetAlgorithms;

  /// @brief Various flags for optimizing event target checks.
  enum class EventTargetFlag : uint8
  {
    IsNode,
    IsWindow,
    HasActivationBehavior,
    HasLegacyPreActivationBehavior,
    HasLegacyCanceledActivationBehavior,
    IsInTopLayer,
  };

  consteval EventTargetFlag MaxEnumValue(EventTargetFlag) noexcept
  {
    return EventTargetFlag::IsInTopLayer;
  }

  using EventTargetFlags = EnumSet<EventTargetFlag>;
}

namespace krys::boo
{
  /// @see https://dom.spec.whatwg.org/#eventtarget
  class EventTarget : public RefCounted<EventTarget>, public CanMakeWeakPtr<EventTarget>
  {
    KRYS_TYPE_CAST_TRAITS_ACCESS();

    friend class dom::EventDispatcher;
    friend class dom::EventTargetAlgorithms;

  protected:
    dom::EventTargetFlags _eventTargetFlags;

  private:
    /// @see https://dom.spec.whatwg.org/#eventtarget-event-listener-list
    SmallList<Ref<EventListener>> _eventListenerList;

    /// @brief An event handler: its value and the listener that calls it, added when it was first set.
    /// @see https://html.spec.whatwg.org/multipage/webappapis.html#event-handlers
    struct EventHandler
    {
      DOMStringAtom Name;
      Func<void(Event &)> Value;
      RefPtr<EventListener> Listener;
    };

    /// @brief The target's event handlers (its event handler map), made when one is first set.
    /// @see https://html.spec.whatwg.org/multipage/webappapis.html#event-handler-map
    UniquePtr<List<EventHandler>> _eventHandlers;

  protected:
    EventTarget(dom::EventTargetFlags flags) noexcept : _eventTargetFlags(flags)
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
    void AddEventListener(const DOMStringAtom &type, Ref<EventListenerCallback> callback,
                          const AddEventListenerOptionsOrBool &options) noexcept;

    /// @brief Adds a listener that calls @p callback, owned by the returned handle: destroying the handle
    /// removes the listener, unless it was released to the target.
    /// @note Listeners released to their target can still be removed together by a signal in @p options.
    KRYS_NODISCARD EventListenerHandle
      AddEventListener(const DOMStringAtom &type, Func<void(Event &)> &&callback,
                       const AddEventListenerOptionsOrBool &options = false) noexcept;

    /// @brief Adds a listener that calls @p callback for the events of type @p type that are a @p TEvent (a
    /// `MouseEvent`, a `KeyboardEvent`), owned by the returned handle.
    template <typename TEvent>
    requires(DerivedFrom<TEvent, Event> && !SameType<TEvent, Event>)
    KRYS_NODISCARD EventListenerHandle
      AddEventListener(const DOMStringAtom &type, Func<void(TEvent &)> &&callback,
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
    void RemoveEventListener(DOMStringAtom type, Ref<EventListenerCallback> callback,
                             const EventListenerOptionsOrBool &options) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-eventtarget-dispatchevent
    ExceptionOr<bool> DispatchEvent(Event &event) noexcept;

    /// @brief The value of the target's own event handler @p name; null if it's not set.
    /// @see https://html.spec.whatwg.org/multipage/webappapis.html#getting-the-current-value-of-the-event-handler
    KRYS_NODISCARD const Func<void(Event &)> *EventHandlerValue(DOMStringAtom name) const noexcept;

    /// @brief Sets the target's own event handler @p name to @p value, activating it (adding its listener)
    /// the first time; an empty @p value deactivates it (removing its listener).
    /// @see https://html.spec.whatwg.org/multipage/webappapis.html#activate-an-event-handler
    /// @see https://html.spec.whatwg.org/multipage/webappapis.html#deactivate-an-event-handler
    void SetEventHandlerValue(DOMStringAtom name, Func<void(Event &)> &&value) noexcept;

    /// @brief The target's event listener list, in the order they were added.
    /// @see https://dom.spec.whatwg.org/#eventtarget-event-listener-list
    KRYS_NODISCARD Span<const Ref<EventListener>> EventListeners() const noexcept
    {
      return _eventListenerList;
    }

    /// @brief Whether the target has a listener for @p type.
    KRYS_NODISCARD bool HasEventListener(DOMStringAtom type) const noexcept;

    /// @brief Whether the target has a capturing listener for @p type.
    KRYS_NODISCARD bool HasCapturingEventListener(DOMStringAtom type) const noexcept;

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
      return _eventTargetFlags.contains(dom::EventTargetFlag::HasActivationBehavior);
    }

    /// @see https://dom.spec.whatwg.org/#eventtarget-activation-behavior
    virtual void ActivationBehavior(Event &event) noexcept
    {
      (void)event;
    }

    /// @see https://dom.spec.whatwg.org/#eventtarget-legacy-pre-activation-behavior
    KRYS_NODISCARD bool HasLegacyPreActivationBehavior() const noexcept
    {
      return _eventTargetFlags.contains(dom::EventTargetFlag::HasLegacyPreActivationBehavior);
    }

    /// @see https://dom.spec.whatwg.org/#eventtarget-legacy-pre-activation-behavior
    virtual void LegacyPreActivationBehavior(Event &event) noexcept
    {
      (void)event;
    }

    /// @see https://dom.spec.whatwg.org/#eventtarget-legacy-canceled-activation-behavior
    KRYS_NODISCARD bool HasLegacyCanceledActivationBehavior() const noexcept
    {
      return _eventTargetFlags.contains(dom::EventTargetFlag::HasLegacyCanceledActivationBehavior);
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
      return _eventTargetFlags.contains(dom::EventTargetFlag::IsNode);
    }

    KRYS_NODISCARD bool IsWindow() const noexcept
    {
      return _eventTargetFlags.contains(dom::EventTargetFlag::IsWindow);
    }

#pragma endregion

#pragma region EventTarget Flags

#pragma endregion
  };
}
