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
