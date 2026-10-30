#pragma once

#include "Krystal.Booey/Platform/Timer.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/CanMakeWeakPtr.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Time/MonotonicTime.hpp"
#include "Krystal.Core/Types/Func.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"
#include <chrono>

namespace krys::boo::dom
{
  class Document;
}

namespace krys::boo
{
  /// @brief A timer that `setTimeout()` or `setInterval()` installed in a document: it runs its action as a
  /// task on the timer task source, once or every interval, until it's removed.
  /// @see https://html.spec.whatwg.org/#timers
  class DOMTimer final : public RefCounted<DOMTimer>, public CanMakeWeakPtr<DOMTimer>
  {
    KRYS_NON_COPY_MOVABLE(DOMTimer);

  public:
    using Duration = MonotonicTime::duration;

    static constexpr Duration DefaultMinimumInterval() noexcept
    {
      return std::chrono::milliseconds(4);
    }

    enum class Type : bool
    {
      SingleShot,
      Repeating
    };

  private:
    WeakPtr<dom::Document> _document;
    int32 _timeoutId {0};
    int32 _nestingLevel {0};
    Timer _timer;
    Func<void()> _action;
    Duration _originalInterval {};
    bool _oneShot {true};
    bool _hasReachedMaxNestingLevel {false};
    Duration _currentTimerInterval {};
    /// @brief The time left until the timer fires, while it's suspended.
    Maybe<Duration> _suspendedNextFireInterval;

    DOMTimer(dom::Document &document, Func<void()> &&action, Duration interval, Type type) noexcept;

  public:
    ~DOMTimer() noexcept;

    /// @brief Installs a timer in @p document that runs @p action after @p timeout (every @p timeout for a
    /// repeating one).
    /// @return The timer's ID, positive.
    /// @see https://html.spec.whatwg.org/#timer-initialisation-steps
    static int32 Install(dom::Document &document, Func<void()> &&action, Duration timeout,
                         Type type) noexcept;

    /// @brief Removes @p document's timer @p timeoutId, if it has one.
    static void RemoveById(dom::Document &document, int32 timeoutId) noexcept;

    /// @brief Stops the timer for good, releasing its action: its document is going away.
    void Stop() noexcept;

    /// @brief Stops the timer for now, keeping the time left until it fires: its document is suspended.
    void Suspend() noexcept;
    /// @brief Starts the timer again with the time it had left.
    void Resume() noexcept;

    KRYS_NODISCARD bool HasReachedMaxNestingLevel() const noexcept
    {
      return _hasReachedMaxNestingLevel;
    }

  private:
    KRYS_NODISCARD Duration IntervalClampedToMinimum() const noexcept;

    /// @brief The interval may need updating, as the nesting level changed.
    void UpdateTimerIntervalIfNecessary() noexcept;

    /// @brief The timer is due: queues its task.
    void TimerFired() noexcept;

    /// @brief The timer's task: runs the action.
    void Fired() noexcept;
  };
}
