/*
 * Copyright (C) 2011 Google Inc. All rights reserved.
 * Copyright (C) 2020 Apple Inc. All rights reserved.
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
 *  THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS'' AND ANY
 *  EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *  DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 *  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 *  ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#pragma once

#include "Krystal.Booey/DOM/Types/DOMHighResTimeStamp.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Containers/List.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Memory/WeakPtr.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/Func.hpp"

namespace krys::boo
{
  class Document;
  class Frame;

  /// @brief Called with the frame's timestamp: milliseconds since the window's time origin.
  /// @see https://html.spec.whatwg.org/multipage/imagebitmap-and-animations.html#framerequestcallback
  using FrameRequestCallback = Func<void(dom::DOMHighResTimeStamp)>;
}

namespace krys::boo::html
{
  /// @brief A document's animation frame callbacks (`requestAnimationFrame()`), run by the rendering update.
  /// @see https://html.spec.whatwg.org/multipage/imagebitmap-and-animations.html#animation-frames
  class AnimationFrameCallbacks
  {
    KRYS_NON_COPY_MOVABLE(AnimationFrameCallbacks);

  public:
    using CallbackId = int32;

  private:
    struct CallbackData
    {
      CallbackId Id {0};
      FrameRequestCallback Callback;
      bool FiredOrCancelled {false};
    };

    WeakPtr<Document> _document;
    List<CallbackData> _callbackDataList;
    CallbackId _nextCallbackId {0};
    uint32 _suspendCount {0};
    dom::DOMHighResTimeStamp _lastAnimationFrameTimestamp {0};

  public:
    explicit AnimationFrameCallbacks(Document &document) noexcept;

    ~AnimationFrameCallbacks() noexcept;

    /// @return The callback's handle, positive.
    CallbackId RegisterCallback(FrameRequestCallback &&callback) noexcept;

    void CancelCallback(CallbackId callbackId) noexcept;

    /// @brief Runs the callbacks registered before this frame, with @p timestamp (milliseconds since the
    /// window's time origin).
    /// @see
    /// https://html.spec.whatwg.org/multipage/imagebitmap-and-animations.html#run-the-animation-frame-callbacks
    void Run(dom::DOMHighResTimeStamp timestamp) noexcept;

    /// @brief Holds the callbacks back (while the document is suspended), until as many resumes.
    void Suspend() noexcept;

    void Resume() noexcept;

    KRYS_NODISCARD bool HasCallbacks() const noexcept
    {
      return !_callbackDataList.empty();
    }

  private:
    KRYS_NODISCARD boo::Frame *Frame() const noexcept;

    /// @brief Asks the page for a rendering update that runs the callbacks.
    void ScheduleAnimation() noexcept;
  };
}
