#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/CanMakeCheckedPtr.hpp"
#include "Krystal.Core/CanMakeWeakPtr.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Time/MonotonicTime.hpp"
#include "Krystal.Core/Types/Func.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo
{
  class Timer;
}

namespace krys::boo::dom
{
  class Document;
  class IdleDeadline;

  /// @brief Called in an idle period, or once its timeout passes.
  /// @see https://w3c.github.io/requestidlecallback/#the-idlerequestcallback-callback
  using IdleRequestCallback = Func<void(IdleDeadline &)>;

  /// @see https://w3c.github.io/requestidlecallback/#the-idlerequestoptions-dictionary
  struct IdleRequestOptions
  {
    /// @brief Milliseconds after which the callback is called even without an idle period; 0 for none.
    uint32 Timeout {0};
  };

  /// @brief A document's idle callbacks (`requestIdleCallback()`): the list of idle request callbacks, and
  /// the runnable ones of the current idle period.
  /// @see https://w3c.github.io/requestidlecallback/
  class IdleCallbackController : public CanMakeWeakPtr<IdleCallbackController>,
                                 public CanMakeCheckedPtr<IdleCallbackController>
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(IdleCallbackController);
    KRYS_NON_COPY_MOVABLE(IdleCallbackController);

  private:
    struct IdleRequest
    {
      uint32 Identifier {0};
      IdleRequestCallback Callback;
      /// @brief When the timeout passes, in the host's time.
      Maybe<MonotonicTime::duration> Timeout;
    };

    WeakPtr<Document> _document;
    uint32 _idleCallbackIdentifier {0};
    /// @see https://w3c.github.io/requestidlecallback/#dfn-list-of-idle-request-callbacks
    List<IdleRequest> _idleRequestCallbacks;
    /// @see https://w3c.github.io/requestidlecallback/#dfn-list-of-runnable-idle-callbacks
    List<IdleRequest> _runnableIdleCallbacks;
    /// @brief The timers of the callbacks' timeouts; fired ones go when the next is added.
    List<UniquePtr<Timer>> _timeoutTimers;

  public:
    explicit IdleCallbackController(Document &document) noexcept;
    ~IdleCallbackController() noexcept;

    /// @return The callback's handle, positive.
    /// @see https://w3c.github.io/requestidlecallback/#the-requestidlecallback-method
    int32 QueueIdleCallback(IdleRequestCallback &&callback, MonotonicTime::duration timeout) noexcept;

    /// @see https://w3c.github.io/requestidlecallback/#the-cancelidlecallback-method
    void RemoveIdleCallback(int32 identifier) noexcept;

    /// @brief Starts an idle period that ends at @p deadline (in real time): runs the callbacks queued before
    /// it, until it ends.
    /// @see https://w3c.github.io/requestidlecallback/#start-an-idle-period-algorithm
    void StartIdlePeriod(MonotonicTime::duration deadline) noexcept;

    KRYS_NODISCARD bool HasPendingIdleCallback() const noexcept
    {
      return !_idleRequestCallbacks.empty() || !_runnableIdleCallbacks.empty();
    }

  private:
    /// @return Whether callbacks remain to run.
    /// @see https://w3c.github.io/requestidlecallback/#invoke-idle-callbacks-algorithm
    bool InvokeIdleCallbacks(MonotonicTime::duration deadline) noexcept;

    /// @see https://w3c.github.io/requestidlecallback/#dfn-invoke-idle-callback-timeout-algorithm
    void InvokeIdleCallbackTimeout(uint32 identifier) noexcept;
  };
}
