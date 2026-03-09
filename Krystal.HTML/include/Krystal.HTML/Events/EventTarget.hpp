#pragma once

#include "Krystal.HTML/DOMString.hpp"
#include "Krystal.HTML/Events/EventListener.hpp"
#include "Krystal.HTML/Events/EventListenerOptions.hpp"
#include "Krystal.HTML/Events/EventTargetFlag.hpp"
#include "Krystal.HTML/Events/RegisteredEventListener.hpp"
#include "Krystal.HTML/Utils/BoolOr.hpp"
#include "Krystal.HTML/Utils/ExceptionOr.hpp"
#include "Krystal.Lib/Mixins/CanMakeWeakPtr.hpp"
#include "Krystal.Lib/Mixins/RefCounted.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include "Krystal.Lib/Types/SmallList.hpp"

namespace Krys::HTML
{
  class EventTarget : public RefCounted<EventTarget>, public CanMakeWeakPtr<EventTarget>
  {
  private:
    EventTargetFlag _flags : BitCount<EventTargetFlag>() {EventTargetFlag::None};
    SmallList<Ref<RegisteredEventListener>> _listeners;

  public:
    virtual ~EventTarget() noexcept = default;

#pragma region EventTarget - https://dom.spec.whatwg.org/#eventtarget

    EventTarget() noexcept = default;

    virtual bool AddEventListener(DOMStringAtom type, RefPtr<EventListener> &&callback,
                                  const AddEventListenerOptionsOrBool &options) noexcept;

    virtual bool RemoveEventListener(DOMStringAtom type, RefPtr<EventListener> &&callback,
                                     const EventListenerOptionsOrBool &options) noexcept;

    virtual ExceptionOr<bool> DispatchEvent(Event &event) noexcept;

#pragma endregion

    /// @see https://dom.spec.whatwg.org/#remove-all-event-listeners
    virtual void RemoveAllEventListeners() noexcept;

#pragma region Type Checks

    KRYS_NODISCARD bool IsNode() const noexcept
    {
      return HasEventTargetFlag(EventTargetFlag::IsNode);
    }

    KRYS_NODISCARD bool IsWindow() const noexcept
    {
      return HasEventTargetFlag(EventTargetFlag::IsWindow);
    }

#pragma endregion

#pragma region EventTarget Flags

  protected:
    EventTarget(EventTargetFlag flags) noexcept : _flags(flags)
    {
    }

    KRYS_NODISCARD bool HasEventTargetFlag(EventTargetFlag flag) const noexcept
    {
      return HasFlag(_flags, flag);
    }

    void SetEventTargetFlag(EventTargetFlag flag) noexcept
    {
      _flags = _flags | flag;
    }

    void ClearEventTargetFlag(EventTargetFlag flag) noexcept
    {
      _flags = _flags & ~flag;
    }

#pragma endregion

    virtual void ActivationBehavior(Event &event) noexcept
    {
      (void)event;
    }

    virtual void LegacyPreActivationBehavior(Event &event) noexcept
    {
      (void)event;
    }

    virtual void LegacyCanceledActivationBehavior(Event &event) noexcept
    {
      (void)event;
    }
  };
}