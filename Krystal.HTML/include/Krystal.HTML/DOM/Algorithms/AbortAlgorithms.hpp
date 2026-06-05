#pragma once

#include "Krystal.HTML/DOM/Callbacks/AbortAlgorithmCallback.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include "Krystal.Lib/Types/Any.hpp"
#include "Krystal.Lib/Types/List.hpp"
#include "Krystal.Lib/Types/Maybe.hpp"

namespace Krys::HTML
{
  class AbortController;
  class AbortSignal;

  /// @brief Implementations of the AbortController and AbortSignal algorithms.
  /// @see https://dom.spec.whatwg.org/#abortcontroller
  /// @see https://dom.spec.whatwg.org/#abortsignal
  class AbortAlgorithms
  {
  public:
    /// @see https://dom.spec.whatwg.org/#abortcontroller-signal-abort
    static void SignalAbort(AbortController &controller, Maybe<Krys::Any> reason = Null) noexcept;

    /// @see https://dom.spec.whatwg.org/#abortsignal-add
    static void Add(const AbortAlgorithmCallback &algorithm, AbortSignal &signal) noexcept;

    /// @see https://dom.spec.whatwg.org/#abortsignal-remove
    static void Remove(const AbortAlgorithmCallback &algorithm, AbortSignal &signal) noexcept;

    /// @see https://dom.spec.whatwg.org/#abortsignal-signal-abort
    static void SignalAbort(AbortSignal &signal, Maybe<Krys::Any> reason = Null) noexcept;

    /// @see https://dom.spec.whatwg.org/#run-the-abort-steps
    static void RunAbortSteps(AbortSignal &signal) noexcept;

    /// @see https://dom.spec.whatwg.org/#create-a-dependent-abort-signal
    static Ref<AbortSignal> CreateDependentAbortSignal(const List<Ref<AbortSignal>> &signals) noexcept;
  };
}