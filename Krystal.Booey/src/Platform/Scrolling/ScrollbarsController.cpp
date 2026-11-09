/*
 * Copyright (C) 2021 Apple Inc. All rights reserved.
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

/*
 * Copyright (c) 2016 Igalia S.L.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/Platform/Scrolling/ScrollbarsController.hpp"
#include "Krystal.Booey/Platform/Scrolling/Scrollbar.hpp"
#include "Krystal.Booey/Platform/ThreadTimers.hpp"
#include <algorithm>
#include <chrono>

namespace krys::boo
{
  namespace
  {
    using namespace std::chrono_literals;

    constexpr MonotonicTime::duration OverlayScrollbarsAnimationDuration = 1s;
    constexpr MonotonicTime::duration OverlayScrollbarsAnimationHideDelay = 2s;

    // The host's time: the time timers fire by.
    KRYS_NODISCARD MonotonicTime::duration ControllerTime() noexcept
    {
      return ThreadTimers::Current().CurrentTime();
    }

    KRYS_NODISCARD constexpr double EaseOutCubic(double t) noexcept
    {
      double p = t - 1;
      return p * p * p + 1;
    }
  }

  ScrollbarsController::ScrollbarsController() noexcept
      : _animationTimer(*this, &ScrollbarsController::AnimationTimerFired)
  {
  }

  ScrollbarsController::~ScrollbarsController() noexcept = default;

  void ScrollbarsController::DidAddScrollbar(Scrollbar &scrollbar, ScrollbarOrientation orientation) noexcept
  {
    if (!scrollbar.IsOverlayScrollbar())
    {
      return;
    }

    auto *&added =
      orientation == ScrollbarOrientation::Vertical ? _verticalOverlayScrollbar : _horizontalOverlayScrollbar;
    auto *other =
      orientation == ScrollbarOrientation::Vertical ? _horizontalOverlayScrollbar : _verticalOverlayScrollbar;
    added = &scrollbar;
    // A first scrollbar shows, then fades; a second takes the first's opacity.
    if (!other)
    {
      _animationCurrent = 1;
    }
    added->SetOpacity(_animationCurrent);
    HideOverlayScrollbars();
  }

  void ScrollbarsController::WillRemoveScrollbar(Scrollbar &scrollbar,
                                                 ScrollbarOrientation orientation) noexcept
  {
    auto *&removed =
      orientation == ScrollbarOrientation::Vertical ? _verticalOverlayScrollbar : _horizontalOverlayScrollbar;
    if (removed != &scrollbar)
    {
      return;
    }
    removed = nullptr;
    if (!HasOverlayScrollbars())
    {
      _animationCurrent = 0;
      _animationTimer.Stop();
    }
  }

  void ScrollbarsController::UpdateOverlayScrollbarsOpacity() noexcept
  {
    for (auto *scrollbar : {_verticalOverlayScrollbar, _horizontalOverlayScrollbar})
    {
      if (scrollbar && _animationCurrent != scrollbar->Opacity())
      {
        scrollbar->SetOpacity(_animationCurrent);
        // A hovered scrollbar stays opaque (the theme draws it so), so it needn't repaint.
        if (scrollbar->HoveredPart() == NoPart)
        {
          scrollbar->Invalidate();
        }
      }
    }
  }

  void ScrollbarsController::AnimationTimerFired() noexcept
  {
    if (!HasOverlayScrollbars() || _overlayScrollbarsLocked)
    {
      return;
    }

    auto currentTime = ControllerTime();
    double progress = 1;
    if (currentTime < _animationEndTime)
    {
      progress = std::chrono::duration<double>(currentTime - _animationStartTime)
                 / std::chrono::duration<double>(_animationEndTime - _animationStartTime);
    }
    auto opacity = static_cast<float>(_animationSource
                                      + EaseOutCubic(std::clamp(progress, 0.0, 1.0))
                                          * (_animationTarget - _animationSource));
    if (opacity != _animationCurrent)
    {
      _animationCurrent = opacity;
      UpdateOverlayScrollbarsOpacity();
    }

    if (_animationCurrent != _animationTarget)
    {
      // The next frame, at 60 frames a second.
      constexpr MonotonicTime::duration TickTime =
        std::chrono::duration_cast<MonotonicTime::duration>(std::chrono::duration<double>(1.0 / 60));
      _animationTimer.StartOneShot(TickTime);
    }
    else
    {
      HideOverlayScrollbars();
    }
  }

  void ScrollbarsController::ShowOverlayScrollbars() noexcept
  {
    if (_overlayScrollbarsLocked)
    {
      return;
    }

    if (_animationTimer.IsActive() && _animationTarget == 1)
    {
      return;
    }
    _animationTimer.Stop();

    if (!HasOverlayScrollbars())
    {
      return;
    }

    _animationSource = _animationCurrent;
    _animationTarget = 1;
    if (_animationTarget != _animationCurrent)
    {
      _animationStartTime = ControllerTime();
      _animationEndTime = _animationStartTime + OverlayScrollbarsAnimationDuration;
      _animationTimer.StartOneShot({});
    }
    else
    {
      // Already shown: the fade out starts again.
      HideOverlayScrollbars();
    }
  }

  void ScrollbarsController::HideOverlayScrollbars() noexcept
  {
    if (_animationTimer.IsActive() && !_animationTarget)
    {
      return;
    }
    _animationTimer.Stop();

    if (!HasOverlayScrollbars())
    {
      return;
    }

    _animationSource = _animationCurrent;
    _animationTarget = 0;
    if (_animationTarget == _animationCurrent)
    {
      return;
    }
    // The fade starts after the delay.
    _animationStartTime = ControllerTime() + OverlayScrollbarsAnimationHideDelay;
    _animationEndTime = _animationStartTime + OverlayScrollbarsAnimationDuration;
    _animationTimer.StartOneShot(OverlayScrollbarsAnimationHideDelay);
  }

  void ScrollbarsController::MouseEnteredContentArea() noexcept
  {
    ShowOverlayScrollbars();
  }

  void ScrollbarsController::MouseExitedContentArea() noexcept
  {
    HideOverlayScrollbars();
  }

  void ScrollbarsController::MouseMovedInContentArea() noexcept
  {
    ShowOverlayScrollbars();
  }

  void ScrollbarsController::ContentAreaDidShow() noexcept
  {
    ShowOverlayScrollbars();
  }

  void ScrollbarsController::ContentAreaDidHide() noexcept
  {
    if (_overlayScrollbarsLocked)
    {
      return;
    }
    _animationTimer.Stop();
    if (_animationCurrent)
    {
      _animationCurrent = 0;
      UpdateOverlayScrollbarsOpacity();
    }
  }

  void ScrollbarsController::NotifyContentAreaScrolled() noexcept
  {
    ShowOverlayScrollbars();
  }

  void ScrollbarsController::LockOverlayScrollbarStateToHidden(bool shouldLockState) noexcept
  {
    if (_overlayScrollbarsLocked == shouldLockState)
    {
      return;
    }
    _overlayScrollbarsLocked = shouldLockState;

    if (!HasOverlayScrollbars())
    {
      return;
    }

    if (_overlayScrollbarsLocked)
    {
      _animationTimer.Stop();
      for (auto *scrollbar : {_horizontalOverlayScrollbar, _verticalOverlayScrollbar})
      {
        if (scrollbar)
        {
          scrollbar->SetOpacity(0);
        }
      }
    }
    else if (_animationCurrent == 1)
    {
      UpdateOverlayScrollbarsOpacity();
    }
    else
    {
      ShowOverlayScrollbars();
    }
  }
}
