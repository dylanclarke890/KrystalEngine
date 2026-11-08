#pragma once

#include "Krystal.Booey/Platform/Scrolling/ScrollTypes.hpp"
#include "Krystal.Booey/Platform/Timer.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Time/MonotonicTime.hpp"

namespace krys::boo
{
  class Scrollbar;

  /// @brief Shows and hides a scrollable area's overlay scrollbars. They fade in when the mouse enters or
  /// moves over the area, or it scrolls, and fade out two seconds after the last of those, each fade taking a
  /// second. Classic scrollbars always show, and are left alone.
  class ScrollbarsController final
  {
    KRYS_NON_COPY_MOVABLE(ScrollbarsController);

  private:
    Scrollbar *_horizontalOverlayScrollbar {nullptr};
    Scrollbar *_verticalOverlayScrollbar {nullptr};
    bool _overlayScrollbarsLocked {false};
    Timer _animationTimer;
    /// @brief The fade's opacity at its start, its end, and now.
    float _animationSource {0};
    float _animationTarget {0};
    float _animationCurrent {0};
    MonotonicTime::duration _animationStartTime {};
    MonotonicTime::duration _animationEndTime {};

  public:
    ScrollbarsController() noexcept;
    ~ScrollbarsController() noexcept;

    /// @brief Starts fading @p scrollbar with the other one, if it's an overlay scrollbar.
    void DidAddScrollbar(Scrollbar &scrollbar, ScrollbarOrientation orientation) noexcept;
    void WillRemoveScrollbar(Scrollbar &scrollbar, ScrollbarOrientation orientation) noexcept;

    void MouseEnteredContentArea() noexcept;
    void MouseExitedContentArea() noexcept;
    void MouseMovedInContentArea() noexcept;
    /// @brief The page was shown: the scrollbars show, then fade.
    void ContentAreaDidShow() noexcept;
    /// @brief The page was hidden: the scrollbars hide at once.
    void ContentAreaDidHide() noexcept;
    void NotifyContentAreaScrolled() noexcept;
    /// @brief Keeps the overlay scrollbars hidden (@p shouldLockState) while the document is suspended, or
    /// lets them show again.
    void LockOverlayScrollbarStateToHidden(bool shouldLockState) noexcept;

    /// @brief The overlay scrollbars' opacity now, for tests and hit testing.
    KRYS_NODISCARD float OverlayScrollbarsOpacity() const noexcept
    {
      return _animationCurrent;
    }

  private:
    KRYS_NODISCARD bool HasOverlayScrollbars() const noexcept
    {
      return _horizontalOverlayScrollbar || _verticalOverlayScrollbar;
    }

    void AnimationTimerFired() noexcept;
    void ShowOverlayScrollbars() noexcept;
    void HideOverlayScrollbars() noexcept;
    void UpdateOverlayScrollbarsOpacity() noexcept;
  };
}
