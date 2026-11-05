#pragma once

#include "Krystal.Booey/Platform/Timer.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/CanMakeWeakPtr.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Time/MonotonicTime.hpp"
#include "Krystal.Core/Types/Func.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::dom
{
  class Document;

  /// @brief A timer started from C++ in a document: it runs its action as a task on the timer task source,
  /// once or every interval, until it's stopped. It waits while its document is suspended, keeping the time
  /// it had left, and stops for good when its document leaves its frame.
  /// @note It replaces `setTimeout()` and `setInterval()`: there are no IDs or nesting clamp.
  class DocumentTimer final : public RefCounted<DocumentTimer>, public CanMakeWeakPtr<DocumentTimer>
  {
    KRYS_NON_COPY_MOVABLE(DocumentTimer);

  public:
    using Duration = MonotonicTime::duration;

  private:
    WeakPtr<Document> _document;
    Timer _timer;
    Func<void()> _action;
    bool _repeating {false};
    Duration _interval {};
    /// @brief The time left until the timer fires, while it's suspended.
    Maybe<Duration> _suspendedNextFireInterval;

  public:
    DocumentTimer(Document &document, Func<void()> action, Duration delay, bool repeating) noexcept;
    ~DocumentTimer() noexcept;

    /// @brief Stops the timer for good, releasing its action.
    void Stop() noexcept;

    /// @brief Stops the timer for now, keeping the time left until it fires: its document is suspended.
    void Suspend() noexcept;
    /// @brief Starts the timer again with the time it had left.
    void Resume() noexcept;

    /// @brief Whether the timer will still run its action.
    KRYS_NODISCARD bool IsActive() const noexcept
    {
      return static_cast<bool>(_action);
    }

  private:
    /// @brief The timer is due: queues its task.
    void TimerFired() noexcept;

    /// @brief The timer's task: runs the action.
    void Fired() noexcept;
  };

  /// @brief Owns a document timer started from C++: stops it when it's destroyed, unless it was released to
  /// its document.
  /// @note Stopping a timer that's already gone (a one-shot that ran, or one dropped with its document) does
  /// nothing. The handle doesn't hold the timer alive.
  class KRYS_NODISCARD DocumentTimerHandle
  {
  private:
    WeakPtr<DocumentTimer> _timer;

  public:
    /// @brief A handle that owns no timer.
    DocumentTimerHandle() noexcept = default;
    explicit DocumentTimerHandle(DocumentTimer &timer) noexcept;
    DocumentTimerHandle(DocumentTimerHandle &&other) noexcept;
    DocumentTimerHandle &operator=(DocumentTimerHandle &&other) noexcept;
    DocumentTimerHandle(const DocumentTimerHandle &) = delete;
    DocumentTimerHandle &operator=(const DocumentTimerHandle &) = delete;
    ~DocumentTimerHandle() noexcept;

    /// @brief Stops the timer now, if it's still active; the handle then owns none.
    void Stop() noexcept;

    /// @brief Gives the timer up to its document, which runs it until it's done (a one-shot) or the document
    /// leaves its frame; the handle then owns none.
    void Release() noexcept;

    /// @brief Whether the handle owns a timer that's still active.
    KRYS_NODISCARD bool IsActive() const noexcept;

    KRYS_NODISCARD explicit operator bool() const noexcept
    {
      return IsActive();
    }
  };
}
