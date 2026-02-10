#pragma once

#include "Krystal.HTML/DOMString.hpp"
#include "Krystal.HTML/Events/AddEventListenerOptions.hpp"
#include "Krystal.HTML/Events/EventListener.hpp"
#include "Krystal.HTML/Events/EventListenerOptions.hpp"
#include "Krystal.HTML/Events/RegisteredEventListener.hpp"
#include "Krystal.HTML/Utils/BoolOr.hpp"
#include "Krystal.HTML/Utils/ExceptionOr.hpp"
#include "Krystal.Lib/Core/Enum.hpp"
#include "Krystal.Lib/Pointers/RefCounted/RefCounted.hpp"
#include "Krystal.Lib/Pointers/RefCounted/RefPtr.hpp"
#include "Krystal.Lib/Pointers/RefCounted/WeakPtr.hpp"
#include "Krystal.Lib/Types/SmallList.hpp"

namespace Krys::HTML
{
  enum class EventTargetFlag : uint8
  {
    None = 0,
    IsNode = 1 << 0,
    IsConnected = 1 << 1,
    IsInShadowTree = 1 << 2,
  };
}

KRYS_DEFINE_FLAGS_ENUM_TRAITS(Krys::HTML::EventTargetFlag, 3u)

namespace Krys::HTML
{
  class EventTarget : public CanMakeWeakPtr<EventTarget>
  {
  private:
    EventTargetFlag _flags : BitCount<EventTargetFlag>() {EventTargetFlag::None};
    SmallList<Ref<RegisteredEventListener>> _listeners;

  public:
    EventTarget() noexcept;

    void AddRef() const noexcept;
    void SubRef() const noexcept;

    // TODO(IMPL): Use BoolOr for AddEventListenerOptions. The bool is used for 'capture'.
    virtual bool AddEventListener(DOMStringAtom type, RefPtr<EventListener> &&callback,
                                  const AddEventListenerOptions &options) noexcept;

    virtual bool RemoveEventListener(DOMStringAtom type, RefPtr<EventListener> &&callback,
                                     const EventListenerOptions &options) noexcept;

    virtual void RemoveAllEventListeners() noexcept;

    virtual ExceptionOr<bool> DispatchEvent(Event &event) noexcept;

    KRYS_NODISCARD bool IsNode() const noexcept
    {
      return HasEventTargetFlag(EventTargetFlag::IsNode);
    }

  protected:
    struct EventTargetTag
    {
    };

    virtual void AddRef(EventTargetTag) const noexcept = 0;

    virtual void SubRef(EventTargetTag) const noexcept = 0;

    struct ConstructNodeTag
    {
    };

    EventTarget(ConstructNodeTag) noexcept : EventTarget()
    {
      SetEventTargetFlag(EventTargetFlag::IsNode);
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
  };
}