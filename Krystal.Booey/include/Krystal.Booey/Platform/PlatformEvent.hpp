#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Time/MonotonicTime.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"

namespace krys::boo
{
  enum class PlatformEventType : uint8
  {
    NoType = 0,

    // PlatformKeyboardEvent
    KeyDown,
    KeyUp,
    RawKeyDown,
    Char,

    // PlatformMouseEvent
    MouseMoved,
    MousePressed,
    MouseReleased,
    MouseForceChanged,
    MouseForceDown,
    MouseForceUp,
    MouseScroll,

    // PlatformWheelEvent
    Wheel,
  };

  enum class PlatformEventModifier : uint8
  {
    AltKey,
    ControlKey,
    MetaKey,
    ShiftKey,
    CapsLockKey,
    // Never used in native platforms but added for initEvent
    AltGraphKey,
  };

  /// @brief An input event from the host, before it becomes a DOM event.
  class PlatformEvent
  {
  public:
    using Type = PlatformEventType;
    using Modifier = PlatformEventModifier;

  protected:
    MonotonicTime::duration _timestamp {};
    Type _type {Type::NoType};
    EnumSet<Modifier> _modifiers;

  public:
    KRYS_NODISCARD Type GetType() const noexcept
    {
      return _type;
    }

    KRYS_NODISCARD bool ShiftKey() const noexcept
    {
      return _modifiers.Contains(Modifier::ShiftKey);
    }

    KRYS_NODISCARD bool ControlKey() const noexcept
    {
      return _modifiers.Contains(Modifier::ControlKey);
    }

    KRYS_NODISCARD bool AltKey() const noexcept
    {
      return _modifiers.Contains(Modifier::AltKey);
    }

    KRYS_NODISCARD bool MetaKey() const noexcept
    {
      return _modifiers.Contains(Modifier::MetaKey);
    }

    KRYS_NODISCARD EnumSet<Modifier> Modifiers() const noexcept
    {
      return _modifiers;
    }

    KRYS_NODISCARD MonotonicTime::duration Timestamp() const noexcept
    {
      return _timestamp;
    }

  protected:
    PlatformEvent() noexcept = default;

    explicit PlatformEvent(Type type) noexcept : _type(type)
    {
    }

    PlatformEvent(Type type, EnumSet<Modifier> modifiers, MonotonicTime::duration timestamp) noexcept
        : _timestamp(timestamp), _type(type), _modifiers(modifiers)
    {
    }

    // Explicit protected destructor so that people don't accidentally
    // delete a PlatformEvent.
    ~PlatformEvent() noexcept = default;
  };
}
