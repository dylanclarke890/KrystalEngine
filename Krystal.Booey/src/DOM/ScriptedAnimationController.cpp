#include "Krystal.Booey/DOM/ScriptedAnimationController.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
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
    CallbackId callbackId = ++_nextCallbackId;
    _callbackDataList.push_back({callbackId, krys::move(callback), false});

    if (!_suspendCount)
    {
      ScheduleAnimation();
    }

    return callbackId;
  }

  void ScriptedAnimationController::CancelCallback(CallbackId callbackId) noexcept
  {
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

    // First, generate a list of callbacks to consider. Callbacks registered from this point on are considered
    // only for the "next" frame, not this one.
    List<CallbackId> callbackIds;
    callbackIds.reserve(_callbackDataList.size());
    for (auto &data : _callbackDataList)
    {
      callbackIds.push_back(data.Id);
    }

    // Invoking callbacks may detach elements from our document, which clears the document's reference to us,
    // so take a defensive reference.
    RefPtr<Document> document = ShareRefPtr(_document.get());

    for (auto id : callbackIds)
    {
      // A callback cancelled by an earlier one is gone from the list.
      auto it = std::ranges::find_if(_callbackDataList, [id](auto &data) { return data.Id == id; });
      if (it == _callbackDataList.end() || it->FiredOrCancelled)
      {
        continue;
      }
      it->FiredOrCancelled = true;
      auto callback = it->Callback;
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
