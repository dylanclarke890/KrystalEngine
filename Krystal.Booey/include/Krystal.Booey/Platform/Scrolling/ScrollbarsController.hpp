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
