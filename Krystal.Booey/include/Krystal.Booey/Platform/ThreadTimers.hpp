#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Time/MonotonicTime.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo
{
  class TimerBase;

  /// @brief The active timers, in the order they fire (a heap on their fire times, then the order they were
  /// started), and the current time, which the host gives when it runs the event loop.
  /// @note Time never goes backwards: an earlier time than the current one is ignored.
  // FIXME: Process-wide until there are agents and threads.
  class ThreadTimers
  {
    friend class TimerBase;
    KRYS_NON_COPY_MOVABLE(ThreadTimers);

  private:
    List<TimerBase *> _timerHeap;
    uint64 _currentHeapInsertionOrder {0};
    MonotonicTime::duration _currentTime {};
    bool _firingTimers {false};

  public:
    ThreadTimers() noexcept = default;

    KRYS_NODISCARD static ThreadTimers &Current() noexcept;

    /// @brief The host's time, as of the last time it ran the event loop.
    KRYS_NODISCARD MonotonicTime::duration CurrentTime() const noexcept
    {
      return _currentTime;
    }
    void SetCurrentTime(MonotonicTime::duration now) noexcept;

    /// @brief Fires the timers that are due at the current time, soonest first. Timers started while they
    /// fire fire next time, even if they're due, so a timer that restarts itself with no delay can't fire
    /// forever.
    /// @return Whether any timer fired.
    bool FireTimersDue() noexcept;

    /// @brief When the next timer fires, if any is active.
    KRYS_NODISCARD Maybe<MonotonicTime::duration> NextFireTime() const noexcept;

  private:
    /// @brief Moves @p timer to its new fire time, adding it to the heap or (if @p active is false) removing
    /// it.
    void SetNextFireTime(TimerBase &timer, MonotonicTime::duration fireTime, bool active) noexcept;

    KRYS_NODISCARD static bool FiresBefore(const TimerBase &a, const TimerBase &b) noexcept;
    void HeapSwap(size_t a, size_t b) noexcept;
    void HeapUp(size_t index) noexcept;
    void HeapDown(size_t index) noexcept;
    void HeapRemove(TimerBase &timer) noexcept;
  };
}
