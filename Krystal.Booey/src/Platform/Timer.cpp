#include "Krystal.Booey/Platform/Timer.hpp"
#include "Krystal.Booey/Platform/ThreadTimers.hpp"

namespace krys::boo
{
  TimerBase::~TimerBase() noexcept
  {
    Stop();
  }

  void TimerBase::Start(MonotonicTime::duration nextFireInterval,
                        MonotonicTime::duration repeatInterval) noexcept
  {
    auto &timers = ThreadTimers::Current();
    _repeatInterval = repeatInterval;
    timers.SetNextFireTime(*this, timers.CurrentTime() + nextFireInterval, true);
  }

  void TimerBase::Stop() noexcept
  {
    _repeatInterval = {};
    if (IsActive())
    {
      ThreadTimers::Current().SetNextFireTime(*this, {}, false);
    }
  }

  MonotonicTime::duration TimerBase::NextFireInterval() const noexcept
  {
    krys_debug_assert(IsActive());
    auto current = ThreadTimers::Current().CurrentTime();
    if (_nextFireTime < current)
    {
      return {};
    }
    return _nextFireTime - current;
  }
}
