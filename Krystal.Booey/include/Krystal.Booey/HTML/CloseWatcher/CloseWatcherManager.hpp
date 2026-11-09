/*
 * Copyright (C) 2025 Igalia S.L. All rights reserved.
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

#include "Krystal.Booey/HTML/CloseWatcher/CloseWatcher.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::dom
{
  class KeyboardEvent;
}

namespace krys::boo::html
{
  /// @brief A window's close watchers, in groups: each user activation allows one more group, so a page can't
  /// trap the user with watchers made without one; a close request closes the last group.
  /// @see https://html.spec.whatwg.org/#close-watcher-manager
  class CloseWatcherManager final : public RefCounted<CloseWatcherManager>
  {
    KRYS_NON_COPY_MOVABLE(CloseWatcherManager);

  private:
    List<List<Ref<CloseWatcher>>> _groups;
    uint32 _allowedNumberOfGroups {1};
    bool _nextUserInteractionAllowsNewGroup {true};

  public:
    CloseWatcherManager() noexcept = default;

    KRYS_NODISCARD static Ref<CloseWatcherManager> Create() noexcept
    {
      return CreateRef<CloseWatcherManager>();
    }

    /// @see https://html.spec.whatwg.org/#establish-a-close-watcher
    void Add(Ref<CloseWatcher> watcher) noexcept;
    /// @see https://html.spec.whatwg.org/#close-watcher-destroy
    void Remove(CloseWatcher &watcher) noexcept;

    /// @see https://html.spec.whatwg.org/#notify-the-close-watcher-manager-about-user-activation
    void NotifyAboutUserActivation() noexcept;
    /// @brief Whether a close request can be canceled: there's a group to spare.
    KRYS_NODISCARD bool CanPreventClose() const noexcept;

    /// @brief Escape's close request: processes the close watchers, unless @p event was handled or is
    /// untrusted.
    /// @return Whether an enabled watcher was asked.
    bool EscapeKeyHandler(dom::KeyboardEvent &event) noexcept;
    /// @brief A close request (Escape, or another user action the host treats as one): asks the last group's
    /// watchers to close, newest first, until one cancels.
    /// @return Whether an enabled watcher was asked.
    /// @see https://html.spec.whatwg.org/#process-close-watchers
    bool ProcessCloseWatchers() noexcept;

    /// @brief Destroys every watcher: the window's document stopped.
    void DestroyAll() noexcept;
  };
}
