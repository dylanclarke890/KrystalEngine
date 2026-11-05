#include "Krystal.Booey/DOM/Scheduling/DocumentTimer.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/HTML/EventLoop/EventLoop.hpp"

namespace krys::boo::dom
{
  DocumentTimer::DocumentTimer(Document &document, Func<void()> action, Duration delay,
                               bool repeating) noexcept
      : _document(CreateWeakPtr(&document)), _timer([this] { TimerFired(); }), _action(krys::move(action)),
        _repeating(repeating), _interval(std::max(delay, Duration {}))
  {
    // A repeating timer with no interval would fire on every run of the event loop.
    if (_repeating && _interval == Duration {})
    {
      _interval = std::chrono::milliseconds(1);
    }

    // A timer started in a suspended document waits for it to resume.
    if (document.ActiveDOMObjectsAreSuspended())
    {
      _suspendedNextFireInterval = _interval;
    }
    else
    {
      _timer.Start(_interval, _repeating ? _interval : Duration {});
    }
  }

  DocumentTimer::~DocumentTimer() noexcept = default;

  void DocumentTimer::Stop() noexcept
  {
    // Release the action, which may hold references back to the document.
    _timer.Stop();
    _suspendedNextFireInterval = null;
    _action = nullptr;
    // A raw pointer: the timer can be stopped while its document is being destroyed.
    if (auto *document = _document.get())
    {
      document->RemoveTimer(*this);
    }
  }

  void DocumentTimer::Suspend() noexcept
  {
    if (_suspendedNextFireInterval || !_timer.IsActive())
    {
      return;
    }
    _suspendedNextFireInterval = _timer.NextFireInterval();
    _timer.Stop();
  }

  void DocumentTimer::Resume() noexcept
  {
    if (!_suspendedNextFireInterval)
    {
      return;
    }
    _timer.Start(*_suspendedNextFireInterval, _repeating ? _interval : Duration {});
    _suspendedNextFireInterval = null;
  }

  void DocumentTimer::TimerFired() noexcept
  {
    RefPtr<Document> document = ShareRefPtr(_document.get());
    if (!document)
    {
      return;
    }

    html::EventLoop::Current().QueueTask(html::TaskSource::Timer, *document,
                                         [weakThis = CreateWeakPtr(this)]
                                         {
                                           if (RefPtr<DocumentTimer> protectedThis =
                                                 ShareRefPtr(weakThis.get()))
                                           {
                                             protectedThis->Fired();
                                           }
                                         });
  }

  void DocumentTimer::Fired() noexcept
  {
    // The action may stop the timer, releasing it.
    Ref<DocumentTimer> protectedThis = ShareRef(*this);
    if (!_action)
    {
      return;
    }

    if (_repeating)
    {
      auto action = _action;
      action();
      return;
    }

    auto action = krys::move(_action);
    Stop();
    action();
  }

  DocumentTimerHandle::DocumentTimerHandle(DocumentTimer &timer) noexcept : _timer(CreateWeakPtr(&timer))
  {
  }

  DocumentTimerHandle::DocumentTimerHandle(DocumentTimerHandle &&other) noexcept
      : _timer(std::exchange(other._timer, {}))
  {
  }

  DocumentTimerHandle &DocumentTimerHandle::operator=(DocumentTimerHandle &&other) noexcept
  {
    if (this != &other)
    {
      Stop();
      _timer = std::exchange(other._timer, {});
    }
    return *this;
  }

  DocumentTimerHandle::~DocumentTimerHandle() noexcept
  {
    Stop();
  }

  void DocumentTimerHandle::Stop() noexcept
  {
    RefPtr<DocumentTimer> timer = ShareRefPtr(_timer.get());
    _timer = nullptr;
    if (timer)
    {
      timer->Stop();
    }
  }

  void DocumentTimerHandle::Release() noexcept
  {
    _timer = nullptr;
  }

  bool DocumentTimerHandle::IsActive() const noexcept
  {
    auto *timer = _timer.get();
    return timer && timer->IsActive();
  }
}
