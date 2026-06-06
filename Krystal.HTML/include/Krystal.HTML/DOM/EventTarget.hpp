#pragma once

#include "Krystal.HTML/DOM/Dicts/AddEventListenerOptions.hpp"
#include "Krystal.HTML/DOM/Dicts/EventListenerOptions.hpp"
#include "Krystal.HTML/DOM/Enums/EventTargetFlags.hpp"
#include "Krystal.HTML/DOM/Internals/Event/EventListener.hpp"
#include "Krystal.HTML/Types/DOMString.hpp"
#include "Krystal.HTML/Types/ExceptionOr.hpp"
#include "Krystal.Lib/Mixins/CanMakeWeakPtr.hpp"
#include "Krystal.Lib/Mixins/RefCounted.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include "Krystal.Lib/Types/SmallList.hpp"

namespace Krys::HTML
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

  protected:
    EventTarget(EventTargetFlags flags) noexcept : _flags(flags)
    {
    }

  public:
    virtual ~EventTarget() noexcept = default;

#pragma region EventTarget - https://dom.spec.whatwg.org/#eventtarget

    /// @see https://dom.spec.whatwg.org/#dom-eventtarget-eventtarget
    EventTarget() noexcept = default;

    /// @see https://dom.spec.whatwg.org/#dom-eventtarget-addeventlistener
    void AddEventListener(DOMStringAtom type, Ref<EventListenerCallback> &&callback,
                          const AddEventListenerOptionsOrBool &options) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-eventtarget-removeeventlistener
    void RemoveEventListener(DOMStringAtom type, Ref<EventListenerCallback> &&callback,
                             const EventListenerOptionsOrBool &options) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-eventtarget-dispatchevent
    ExceptionOr<bool> DispatchEvent(Event &event) noexcept;

  protected:
    /// @see https://dom.spec.whatwg.org/#get-the-parent
    KRYS_NODISCARD virtual RawPtr<EventTarget> GetParent(Event &event) const noexcept
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