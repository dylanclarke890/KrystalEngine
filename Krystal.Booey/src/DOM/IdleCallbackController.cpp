#include "Krystal.Booey/DOM/IdleCallbackController.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/IdleDeadline.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/Platform/Timer.hpp"
#include <chrono>

namespace krys::boo::dom
{
  IdleCallbackController::IdleCallbackController(Document &document) noexcept
      : _document(CreateWeakPtr(&document))
  {
  }

  IdleCallbackController::~IdleCallbackController() noexcept = default;

  int32 IdleCallbackController::QueueIdleCallback(IdleRequestCallback &&callback,
                                                  MonotonicTime::duration timeout) noexcept
  {
    ++_idleCallbackIdentifier;
    auto handle = _idleCallbackIdentifier;
    bool hasTimeout = timeout > MonotonicTime::duration::zero();
    auto &eventLoop = html::EventLoop::Current();
    _idleRequestCallbacks.push_back(
      {handle, krys::move(callback), hasTimeout ? Maybe(eventLoop.Now() + timeout) : null});

    if (hasTimeout)
    {
      std::erase_if(_timeoutTimers, [](auto &timer) { return !timer->IsActive(); });
      auto timer = CreateUnique<Timer>(
        [weakThis = CreateWeakPtr(this), handle]
        {
          auto *controller = weakThis.get();
          auto *document = controller ? controller->_document.get() : nullptr;
          if (!document)
          {
            return;
          }
          html::EventLoop::Current().QueueTask(html::TaskSource::IdleTask, *document,
                                               [weakThis, handle]
                                               {
                                                 if (auto *controller = weakThis.get())
                                                 {
                                                   controller->InvokeIdleCallbackTimeout(handle);
                                                 }
                                               });
        });
      timer->StartOneShot(timeout);
      _timeoutTimers.push_back(krys::move(timer));
    }

    // The host starts an idle period after its next rendering update
    // (Page::OpportunisticallyRunIdleCallbacks).
    return static_cast<int32>(handle);
  }

  void IdleCallbackController::RemoveIdleCallback(int32 signedIdentifier) noexcept
  {
    if (signedIdentifier <= 0)
    {
      return;
    }
    auto identifier = static_cast<uint32>(signedIdentifier);
    std::erase_if(_idleRequestCallbacks,
                  [identifier](auto &request) { return request.Identifier == identifier; });
    std::erase_if(_runnableIdleCallbacks,
                  [identifier](auto &request) { return request.Identifier == identifier; });
  }

  void IdleCallbackController::StartIdlePeriod(MonotonicTime::duration deadline) noexcept
  {
    for (auto &request : _idleRequestCallbacks)
    {
      _runnableIdleCallbacks.push_back(krys::move(request));
    }
    _idleRequestCallbacks.clear();

    if (_runnableIdleCallbacks.empty())
    {
      return;
    }

    while (InvokeIdleCallbacks(deadline))
    {
    }
  }

  bool IdleCallbackController::InvokeIdleCallbacks(MonotonicTime::duration deadline) noexcept
  {
    RefPtr<Document> document = ShareRefPtr(_document.get());
    if (!document || !document->Frame())
    {
      return false;
    }

    // FIXME: Implement "if the user-agent believes it should end the idle period early due to newly scheduled
    // high-priority work, return from the algorithm."
    auto now = MonotonicTime::Now();
    if (now >= deadline || _runnableIdleCallbacks.empty())
    {
      return false;
    }

    auto request = krys::move(_runnableIdleCallbacks.front());
    _runnableIdleCallbacks.erase(_runnableIdleCallbacks.begin());
    auto didTimeout = request.Timeout && *request.Timeout < html::EventLoop::Current().Now()
                        ? IdleDeadline::DidTimeout::Yes
                        : IdleDeadline::DidTimeout::No;
    auto idleDeadline = IdleDeadline::Create(deadline, didTimeout);
    request.Callback(*idleDeadline);
    return !_runnableIdleCallbacks.empty();
  }

  void IdleCallbackController::InvokeIdleCallbackTimeout(uint32 identifier) noexcept
  {
    if (!_document)
    {
      return;
    }

    auto it = std::ranges::find_if(_idleRequestCallbacks,
                                   [identifier](auto &request) { return request.Identifier == identifier; });
    if (it == _idleRequestCallbacks.end())
    {
      return;
    }

    auto idleDeadline = IdleDeadline::Create(MonotonicTime::Now(), IdleDeadline::DidTimeout::Yes);
    auto callback = krys::move(it->Callback);
    _idleRequestCallbacks.erase(it);
    callback(*idleDeadline);
  }
}
