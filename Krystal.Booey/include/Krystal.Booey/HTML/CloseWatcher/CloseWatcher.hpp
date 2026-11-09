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

#include "Krystal.Booey/DOM/Abort/AbortSignal.hpp"
#include "Krystal.Booey/DOM/Events/EventTarget.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Booey/HTML/Attributes/EventHandlerAttributes.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/Func.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::dom
{
  class Document;
}

namespace krys::boo::html
{
  /// @brief Something the user can close (with Escape, or the host's back gesture): it's asked with its
  /// cancel action, which can refuse while the user has interacted with the page since, then closed with its
  /// close action. A watcher made with `Create` fires `cancel` and `close` at itself; dialogs and popovers
  /// establish their own with their own actions.
  /// @see https://html.spec.whatwg.org/#close-watchers
  class CloseWatcher final : public dom::EventTarget
  {
  public:
    /// @see https://html.spec.whatwg.org/#closewatcheroptions
    struct Options
    {
      RefPtr<dom::AbortSignal> Signal;
    };

    /// @brief Given whether it can refuse, returns whether to close.
    /// @see https://html.spec.whatwg.org/#close-watcher-cancel-action
    using CancelAction = Func<bool(bool canPreventClose)>;
    /// @see https://html.spec.whatwg.org/#close-watcher-close-action
    using CloseAction = Func<void()>;
    /// @brief Whether the watcher takes close requests now.
    /// @see https://html.spec.whatwg.org/#close-watcher-get-enabled-state
    using GetEnabledState = Func<bool()>;

  private:
    WeakPtr<dom::Document> _document;
    CancelAction _cancelAction;
    CloseAction _closeAction;
    GetEnabledState _getEnabledState;
    bool _active {true};
    bool _isRunningCancelAction {false};
    RefPtr<dom::AbortSignal> _signal;
    Maybe<dom::AbortAlgorithmCallback> _signalAlgorithm;

  public:
    CloseWatcher(dom::Document &document, CancelAction &&cancelAction, CloseAction &&closeAction,
                 GetEnabledState &&getEnabledState) noexcept;

    /// @brief A close watcher of @p document's window that fires `cancel` and `close` at itself, destroyed
    /// once @p options' signal aborts.
    /// @return An `InvalidStateError` if @p document isn't fully active.
    /// @see https://html.spec.whatwg.org/#dom-closewatcher
    static dom::ExceptionOr<Ref<CloseWatcher>> Create(dom::Document &document,
                                                      const Options &options = {}) noexcept;

    /// @brief A close watcher of @p document's window, which must be fully active, with the given actions.
    /// @see https://html.spec.whatwg.org/#establish-a-close-watcher
    static Ref<CloseWatcher> Establish(dom::Document &document, CancelAction &&cancelAction,
                                       CloseAction &&closeAction, GetEnabledState &&getEnabledState) noexcept;

    /// @brief Whether the watcher is still in its window's close watcher manager.
    /// @see https://html.spec.whatwg.org/#close-watcher-is-active
    KRYS_NODISCARD bool IsActive() const noexcept
    {
      return _active;
    }

    /// @see https://html.spec.whatwg.org/#close-watcher-get-enabled-state
    KRYS_NODISCARD bool IsEnabled() const noexcept
    {
      return _getEnabledState();
    }

    /// @brief Requests to close the watcher; its cancel action can always refuse.
    /// @see https://html.spec.whatwg.org/#dom-closewatcher-requestclose
    void RequestClose() noexcept;
    /// @brief Requests to close the watcher; its cancel action can refuse unless
    /// @p requireHistoryActionActivation and the window has no history-action activation or group to spare.
    /// @return Whether the watcher closed (or couldn't be asked): false if the cancel action refused.
    /// @see https://html.spec.whatwg.org/#close-watcher-request-close
    bool RequestToClose(bool requireHistoryActionActivation) noexcept;
    /// @see https://html.spec.whatwg.org/#close-watcher-close
    void Close() noexcept;
    /// @see https://html.spec.whatwg.org/#close-watcher-destroy
    void Destroy() noexcept;

    KRYS_DECLARE_EVENT_HANDLER(Cancel, u8"cancel")
    KRYS_DECLARE_EVENT_HANDLER(Close, u8"close")
  };
}
