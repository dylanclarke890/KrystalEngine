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

#include "Krystal.Booey/Platform/CaretAnimator.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/HTML/EventLoop/EventLoop.hpp"
#include "Krystal.Booey/Page/Page.hpp"

namespace krys::boo
{
  CaretAnimator::CaretAnimator(CaretAnimationClient &client) noexcept
      : _client(client), _blinkTimer(*this, &CaretAnimator::ScheduleAnimation)
  {
  }

  bool CaretAnimator::IsBlinkingSuspended() const noexcept
  {
    return _isBlinkingSuspended;
  }

  Page *CaretAnimator::GetPage() const noexcept
  {
    if (auto *document = _client.GetDocument())
    {
      return document->GetPage();
    }

    return nullptr;
  }

  MonotonicTime::duration CaretAnimator::Now() noexcept
  {
    return html::EventLoop::Current().Now();
  }

  void CaretAnimator::DidStart(MonotonicTime::duration currentTime,
                               Maybe<MonotonicTime::duration> interval) noexcept
  {
    _startTime = currentTime;
    _isActive = true;
    SetBlinkingSuspended(!interval);
    if (interval)
    {
      _blinkTimer.StartOneShot(*interval);
    }
  }

  void CaretAnimator::DidEnd() noexcept
  {
    _isActive = false;
    _blinkTimer.Stop();
  }

  void CaretAnimator::Stop(CaretAnimatorStopReason) noexcept
  {
    if (!_isActive)
    {
      return;
    }
    DidEnd();
  }

  void CaretAnimator::ServiceCaretAnimation() noexcept
  {
    if (!IsActive())
    {
      return;
    }
    UpdateAnimationProperties();
  }

  void CaretAnimator::ScheduleAnimation() noexcept
  {
    if (auto *page = GetPage())
    {
      page->ScheduleRenderingUpdate({RenderingUpdateStep::CaretAnimation});
    }
  }

  void CaretAnimator::Paint(GraphicsContext &context, const FloatRect &caret, const Color &color,
                            const LayoutPoint &) const noexcept
  {
    context.FillRect(caret, color);
  }

  LayoutRect CaretAnimator::CaretRepaintRectForLocalRect(LayoutRect rect) const noexcept
  {
    return rect;
  }
}
