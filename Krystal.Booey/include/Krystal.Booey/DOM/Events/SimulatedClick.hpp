/*
 * Copyright (C) 2016 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Core/Numeric.hpp"

namespace krys::boo::dom
{
  class Element;
  class Event;

  enum class SimulatedClickMouseEventOptions : uint8
  {
    SendNoEvents,
    SendMouseUpDownEvents,
  };

  enum class SimulatedClickVisualOptions : uint8
  {
    DoNotShowPressedLook,
    ShowPressedLook,
  };

  enum class SimulatedClickSource : uint8
  {
    Bindings,
    UserAgent,
  };

  /// @brief Dispatches a click at @p element, as a press of Space or Enter or `click()` does, with the press
  /// and release before it if @p mouseEventOptions asks for them.
  /// @return False if the element is a disabled form control, or is already being clicked.
  bool SimulateClick(Element &element, Event *underlyingEvent,
                     SimulatedClickMouseEventOptions mouseEventOptions,
                     SimulatedClickVisualOptions visualOptions, SimulatedClickSource source) noexcept;
}
