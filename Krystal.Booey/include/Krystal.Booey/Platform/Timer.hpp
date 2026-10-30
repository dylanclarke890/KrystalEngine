#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Time/MonotonicTime.hpp"
#include "Krystal.Core/Types/Func.hpp"

namespace krys::boo
{
  class ThreadTimers;

  /// @brief A timer that fires once, or repeatedly, after an interval of the host's time: the event loop
  /// fires the timers that are due when the host runs it with the current time.
  /// @note A timer stops when it's destroyed, so its owner may hold it as a member.
  // FIXME: Timer alignment and nesting levels, once there are script timers.
  class TimerBase
  {
    friend class ThreadTimers;
    KRYS_NON_COPY_MOVABLE(TimerBase);

    static constexpr size_t NotInHeap = static_cast<size_t>(-1);

  private:
    MonotonicTime::duration _nextFireTime {};
    MonotonicTime::duration _repeatInterval {};
    uint64 _heapInsertionOrder {0};
    size_t _heapIndex {NotInHeap};

  public:
    TimerBase() noexcept = default;
    virtual ~TimerBase() noexcept;

    /// @brief Fires after @p nextFireInterval, then every @p repeatInterval if it isn't zero.
    void Start(MonotonicTime::duration nextFireInterval, MonotonicTime::duration repeatInterval) noexcept;
    void StartRepeating(MonotonicTime::duration repeatInterval) noexcept
    {
      Start(repeatInterval, repeatInterval);
    }
    void StartOneShot(MonotonicTime::duration delay) noexcept
    {
      Start(delay, {});
    }
    void Stop() noexcept;

    KRYS_NODISCARD bool IsActive() const noexcept
    {
      return _heapIndex != NotInHeap;
    }

    /// @brief When the timer fires next, in the host's time, if it's active.
    KRYS_NODISCARD MonotonicTime::duration NextFireTime() const noexcept
    {
      return _nextFireTime;
    }
    /// @brief How long until the timer fires, from the host's current time; zero if it's due.
    KRYS_NODISCARD MonotonicTime::duration NextFireInterval() const noexcept;
    KRYS_NODISCARD MonotonicTime::duration RepeatInterval() const noexcept
    {
      return _repeatInterval;
    }

  private:
    virtual void Fired() noexcept = 0;
  };

  /// @brief A timer that calls a function when it fires.
  class Timer final : public TimerBase
  {
  private:
    Func<void()> _function;

  public:
    explicit Timer(Func<void()> &&function) noexcept : _function(krys::move(function))
    {
    }

    template <typename TFiredClass>
    Timer(TFiredClass &object, void (TFiredClass::*function)() noexcept) noexcept
        : _function([&object, function] { (object.*function)(); })
    {
    }

  private:
    void Fired() noexcept override
    {
      _function();
    }
  };
}
