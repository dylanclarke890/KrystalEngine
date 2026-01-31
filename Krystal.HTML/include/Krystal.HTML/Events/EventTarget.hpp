#pragma once

#include "Krystal.HTML/Events/AddEventListenerOptions.hpp"
#include "Krystal.HTML/Events/EventListener.hpp"
#include "Krystal.HTML/Events/EventListenerOptions.hpp"
#include "Krystal.HTML/Events/RegisteredEventListener.hpp"
#include "Krystal.Lib/Core/Enum.hpp"
#include "Krystal.Lib/Pointers/Ref.hpp"
#include "Krystal.Lib/Pointers/RefCounted.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include "Krystal.Lib/String/StringAtom.hpp"
#include "Krystal.Lib/Types/SmallList.hpp"

namespace Krys::HTML
{
  enum class EventTargetFlag : uint8
  {
    None = 0,
    IsNode = 1 << 0,
  };
}

KRYS_DEFINE_FLAGS_ENUM_TRAITS(Krys::HTML::EventTargetFlag, 2u)

namespace Krys::HTML
{
  class EventTarget : public WeakRefCounted<EventTarget>
  {
  private:
    EventTargetFlag _flags : BitCount<EventTargetFlag>() {EventTargetFlag::None};
    SmallList<Ref<RegisteredEventListener>> _listeners;

  public:
    EventTarget() noexcept;

    virtual bool AddEventListener(StringAtom type, RefPtr<EventListener> &&callback,
                                  const AddEventListenerOptions &options) noexcept;

    virtual bool RemoveEventListener(StringAtom type, RefPtr<EventListener> &&callback,
                                     const EventListenerOptions &options) noexcept;

    virtual bool DispatchEvent(Event &event) noexcept;

    KRYS_NODISCARD bool IsNode() const noexcept
    {
      return HasFlag(_flags, EventTargetFlag::IsNode);
    }

  protected:
    struct ConstructNodeTag
    {
    };

    EventTarget(ConstructNodeTag) noexcept : EventTarget()
    {
      _flags = _flags | EventTargetFlag::IsNode;
    }
  };
}