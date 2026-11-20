#pragma once

#include "Krystal.Booey/DOM/Abort/AbortAlgorithmHandle.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/Any.hpp"
#include "Krystal.Core/Types/Func.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  class AbortController;
  class AbortSignal;
}

namespace krys::boo::dom
{
  /// @brief Implementations of the AbortController and AbortSignal algorithms.
  /// @see https://dom.spec.whatwg.org/#abortcontroller
  /// @see https://dom.spec.whatwg.org/#abortsignal
  class AbortAlgorithms
  {
    friend class krys::boo::AbortAlgorithmHandle;

  public:
    /// @see https://dom.spec.whatwg.org/#abortcontroller-signal-abort
    static void SignalAbort(AbortController &controller, Maybe<krys::Any> reason = null) noexcept;

    /// @brief Adds @p algorithm to @p signal's abort algorithms, owned by the returned handle: destroying the
    /// handle removes it, unless it was released to the signal.
    /// @return A handle that owns no algorithm if @p signal is aborted.
    /// @see https://dom.spec.whatwg.org/#abortsignal-add
    static AbortAlgorithmHandle Add(Func<void(const krys::Any &reason)> algorithm,
                                    AbortSignal &signal) noexcept;

    /// @see https://dom.spec.whatwg.org/#abortsignal-signal-abort
    static void SignalAbort(AbortSignal &signal, Maybe<krys::Any> reason = null) noexcept;

    /// @see https://dom.spec.whatwg.org/#run-the-abort-steps
    static void RunAbortSteps(AbortSignal &signal) noexcept;

    /// @see https://dom.spec.whatwg.org/#create-a-dependent-abort-signal
    static Ref<AbortSignal> CreateDependentAbortSignal(const List<Ref<AbortSignal>> &signals) noexcept;

  private:
    /// @brief Removes the abort algorithm with @p id from @p signal's abort algorithms, through its handle.
    /// @see https://dom.spec.whatwg.org/#abortsignal-remove
    static void Remove(AbortSignal &signal, uint64 id) noexcept;

    /// @brief Removes @p signal from its sources' dependents, and clears its sources.
    static void DetachFromSources(AbortSignal &signal) noexcept;
  };
}
