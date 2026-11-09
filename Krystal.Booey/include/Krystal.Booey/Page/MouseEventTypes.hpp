/*
 * Copyright (C) 2025 Apple Inc. All rights reserved.
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

namespace krys::boo
{
  static constexpr double ForceAtClick = 1;
  static constexpr double ForceAtForceClick = 2;

  /// @brief Whether a mouse event stands in for a tap, and with how many fingers.
  /// @brief How a pen is held: its tilt along each axis and its twist about its own axis, in degrees.
  /// @see https://w3c.github.io/pointerevents/#dom-pointerevent-tiltx
  struct PenOrientation
  {
    int32 TiltX {0};
    int32 TiltY {0};
    int32 Twist {0};
  };

  enum class SyntheticClickType : uint8
  {
    NoTap,
    OneFingerTap,
    TwoFingerTap
  };

  // These button numbers match the ones used in the DOM API, 0 through 2, except for None and Other which
  // aren't specified. We reserve -2 for the former and -1 to represent pointer events that indicate that the
  // pressed mouse button hasn't changed since the last event, as specified in the DOM API for Pointer Events.
  // https://w3c.github.io/uievents/#dom-mouseevent-button
  // https://w3c.github.io/pointerevents/#the-button-property
  enum class MouseButton : int8
  {
    None = -2,
    PointerHasNotChanged,
    Left,
    Middle,
    Right,
    Back,
    Forward,
    Other
  };

  /// @brief The button a DOM `button` value names, or `Other` if it names none.
  KRYS_NODISCARD constexpr MouseButton ButtonFromShort(int16 buttonValue) noexcept
  {
    if (buttonValue >= static_cast<int16>(MouseButton::None)
        && buttonValue < static_cast<int16>(MouseButton::Other))
    {
      return static_cast<MouseButton>(buttonValue);
    }
    return MouseButton::Other;
  }
}
