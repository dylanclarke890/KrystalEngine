/*
 * Copyright (C) 2004-2022 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Platform/SimpleCaretAnimator.hpp"
#include "Krystal.Booey/Rendering/RenderTheme.hpp"

namespace krys::boo
{
  SimpleCaretAnimator::SimpleCaretAnimator(CaretAnimationClient &client) noexcept : CaretAnimator(client)
  {
  }

  void SimpleCaretAnimator::UpdateAnimationProperties() noexcept
  {
    auto currentTime = Now();
    auto caretBlinkInterval = RenderTheme::Singleton().CaretBlinkInterval();

    SetBlinkingSuspended(!caretBlinkInterval);

    if (IsBlinkingSuspended())
    {
      // Ensure the caret is always visible when blinking is suspended.
      if (_presentationProperties.Blink == PresentationProperties::BlinkState::On)
      {
        _blinkTimer.StartOneShot(caretBlinkInterval.value_or(MonotonicTime::duration {}));
      }
      return;
    }

    krys_debug_assert(caretBlinkInterval.has_value());

    if (currentTime - _lastTimeCaretPaintWasToggled >= *caretBlinkInterval)
    {
      SetBlinkState(!_presentationProperties.Blink);
      _lastTimeCaretPaintWasToggled = currentTime;

      _blinkTimer.StartOneShot(*caretBlinkInterval);
    }
  }

  void SimpleCaretAnimator::Start() noexcept
  {
    _lastTimeCaretPaintWasToggled = Now();
    DidStart(_lastTimeCaretPaintWasToggled, RenderTheme::Singleton().CaretBlinkInterval());
  }
}
