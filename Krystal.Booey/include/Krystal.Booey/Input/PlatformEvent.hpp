/*
 * Copyright (C) 2011-2020 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

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

    // PlatformTouchEvent
    TouchStart,
    TouchMove,
    TouchEnd,
    TouchCancel,
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
