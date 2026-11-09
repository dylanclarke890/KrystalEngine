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

#include "Krystal.Booey/DOM/Scheduling/ScriptedAnimationController.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/Page/Page.hpp"
#include <cmath>

namespace krys::boo::dom
{
  ScriptedAnimationController::ScriptedAnimationController(Document &document) noexcept
      : _document(CreateWeakPtr(&document))
  {
  }

  ScriptedAnimationController::~ScriptedAnimationController() noexcept = default;

  void ScriptedAnimationController::Suspend() noexcept
  {
    ++_suspendCount;
  }

  void ScriptedAnimationController::Resume() noexcept
  {
    if (_suspendCount > 0)
    {
      --_suspendCount;
    }

    if (!_suspendCount && !_callbackDataList.empty())
    {
      ScheduleAnimation();
    }
  }

  Page *ScriptedAnimationController::GetPage() const noexcept
  {
    auto *document = _document.get();
    return document ? document->GetPage() : nullptr;
  }

  ScriptedAnimationController::CallbackId
    ScriptedAnimationController::RegisterCallback(FrameRequestCallback &&callback) noexcept
  {
    // https://html.spec.whatwg.org/#dom-animationframeprovider-requestanimationframe
    // 1. If this is not supported, then throw a "NotSupportedError" DOMException.
    // (A window is always supported.)
    // 2. Let target be this's target object.
    // 3. Increment target's animation frame callback identifier by one, and let handle be the result.
    CallbackId callbackId = ++_nextCallbackId;
    // 4. Let callbacks be target's map of animation frame callbacks.
    // 5. Set callbacks[handle] to callback.
    _callbackDataList.push_back({callbackId, krys::move(callback), false});

    if (!_suspendCount)
    {
      ScheduleAnimation();
    }

    // 6. Return handle.
    return callbackId;
  }

  // https://html.spec.whatwg.org/#animationframeprovider-cancelanimationframe
  void ScriptedAnimationController::CancelCallback(CallbackId callbackId) noexcept
  {
    // 1. If this is not supported, then throw a "NotSupportedError" DOMException.
    // 2. Let callbacks be this's target object's map of animation frame callbacks.
    // 3. Remove callbacks[handle].
    auto it =
      std::ranges::find_if(_callbackDataList, [callbackId](auto &data) { return data.Id == callbackId; });
    if (it != _callbackDataList.end())
    {
      // A list being serviced keeps its entries until the end: mark it so it doesn't run.
      it->FiredOrCancelled = true;
      _callbackDataList.erase(it);
    }
  }

  void
    ScriptedAnimationController::ServiceRequestAnimationFrameCallbacks(DOMHighResTimeStamp timestamp) noexcept
  {
    if (_callbackDataList.empty() || _suspendCount)
    {
      return;
    }

    // FIXME: Throttling the callbacks below the page's rendering rate
    // (shouldRescheduleRequestAnimationFrame), once there's throttling.

    auto highResNowMs = std::round(timestamp);

    // https://html.spec.whatwg.org/#run-the-animation-frame-callbacks
    // 1. Let callbacks be target's map of animation frame callbacks.
    // 2. Let callbackHandles be the result of getting the keys of callbacks.
    // (Callbacks registered from this point on are considered only for the next frame.)
    List<CallbackId> callbackIds;
    callbackIds.reserve(_callbackDataList.size());
    for (auto &data : _callbackDataList)
    {
      callbackIds.push_back(data.Id);
    }

    // Invoking callbacks may detach elements from our document, which clears the document's reference to us,
    // so take a defensive reference.
    RefPtr<Document> document = ShareRefPtr(_document.get());

    // 3. For each handle in callbackHandles, if handle exists in callbacks:
    for (auto id : callbackIds)
    {
      // (A callback cancelled by an earlier one is gone from the list.)
      auto it = std::ranges::find_if(_callbackDataList, [id](auto &data) { return data.Id == id; });
      if (it == _callbackDataList.end() || it->FiredOrCancelled)
      {
        continue;
      }
      // 3a. Let callback be callbacks[handle].
      // 3b. Remove callbacks[handle].
      // (It's removed below, as the list is being walked.)
      it->FiredOrCancelled = true;
      auto callback = it->Callback;
      // 3c. Invoke callback with « now » and "report".
      callback(highResNowMs);
    }

    // Remove any callbacks we fired from the list of pending callbacks.
    std::erase_if(_callbackDataList, [](auto &data) { return data.FiredOrCancelled; });

    _lastAnimationFrameTimestamp = timestamp;

    if (!_callbackDataList.empty())
    {
      ScheduleAnimation();
    }
  }

  void ScriptedAnimationController::ScheduleAnimation() noexcept
  {
    if (auto *page = GetPage())
    {
      page->ScheduleRenderingUpdate(RenderingUpdateStep::AnimationFrameCallbacks);
    }
  }
}
