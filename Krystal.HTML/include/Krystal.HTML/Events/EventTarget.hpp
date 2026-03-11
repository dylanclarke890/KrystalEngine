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
  class EventDispatcher;

  class EventTarget : public RefCounted<EventTarget>, public CanMakeWeakPtr<EventTarget>
  {
    friend class EventDispatcher;

  private:
    EventTargetFlag _flags : BitCount<EventTargetFlag>() {EventTargetFlag::None};
    SmallList<Ref<RegisteredEventListener>> _listeners;

  public:
    virtual ~EventTarget() noexcept = default;

#pragma region EventTarget - https://dom.spec.whatwg.org/#eventtarget

    EventTarget() noexcept = default;

    void AddEventListener(DOMStringAtom type, Ref<EventListener> &&callback,
                          const AddEventListenerOptionsOrBool &options) noexcept;

    void RemoveEventListener(DOMStringAtom type, Ref<EventListener> &&callback,
                             const EventListenerOptionsOrBool &options) noexcept;

    ExceptionOr<bool> DispatchEvent(Event &event) noexcept;

#pragma endregion

    /// @see https://dom.spec.whatwg.org/#get-the-parent
    RawPtr<EventTarget> GetParent(Event &event) const noexcept
    {
      (void)event;
      return nullptr;
    }

#pragma region Activation Behavior

    KRYS_NODISCARD bool HasActivationBehavior() const noexcept
    {
      return HasFlag(_flags, EventTargetFlag::HasActivationBehavior);
    }

    virtual void ActivationBehavior(Event &event) noexcept
    {
      (void)event;
    }

    KRYS_NODISCARD bool HasLegacyPreActivationBehavior() const noexcept
    {
      return HasFlag(_flags, EventTargetFlag::HasLegacyPreActivationBehavior);
    }

    virtual void LegacyPreActivationBehavior(Event &event) noexcept
    {
      (void)event;
    }

    KRYS_NODISCARD bool HasLegacyCanceledActivationBehavior() const noexcept
    {
      return HasFlag(_flags, EventTargetFlag::HasLegacyCanceledActivationBehavior);
    }

    virtual void LegacyCanceledActivationBehavior(Event &event) noexcept
    {
      (void)event;
    }

#pragma endregion

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

  protected:
#pragma region EventTarget Flags

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

    /// @see https://dom.spec.whatwg.org/#concept-flatten-options
    KRYS_NODISCARD static EventListenerOptions
      FlattenOptions(const EventListenerOptionsOrBool &optionsOrBool) noexcept
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

      return options;
    }

    /// @see https://dom.spec.whatwg.org/#event-flatten-more
    KRYS_NODISCARD static AddEventListenerOptions
      FlattenMoreOptions(const AddEventListenerOptionsOrBool &optionsOrBool) noexcept
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

      return options;
    }

    /// @see https://dom.spec.whatwg.org/#default-passive-value
    KRYS_NODISCARD static bool DefaultPassiveValue(const DOMStringAtom &type,
                                                   const EventTarget &eventTarget) noexcept;

    /// @see https://dom.spec.whatwg.org/#add-an-event-listener
    static void AddEventListener(EventTarget &eventTarget, RegisteredEventListener &listener) noexcept;

    /// @see https://dom.spec.whatwg.org/#remove-an-event-listener
    static void RemoveEventListener(EventTarget &eventTarget, RegisteredEventListener &listener) noexcept;

    /// @see https://dom.spec.whatwg.org/#remove-all-event-listeners
    static void RemoveAllEventListeners(EventTarget &eventTarget) noexcept;
  };
}