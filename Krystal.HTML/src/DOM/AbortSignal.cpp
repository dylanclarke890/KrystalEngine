#include "Krystal.HTML/DOM/AbortSignal.hpp"
#include "Krystal.HTML/Constants/EventNames.hpp"
#include "Krystal.HTML/DOM/AbortController.hpp"
#include "Krystal.HTML/DOM/Algorithms/AbortAlgorithms.hpp"

namespace Krys::HTML
{
  Ref<AbortSignal> AbortSignal::Abort(Maybe<::Krys::Any> reason) noexcept
  {
    auto signal = CreateRef<AbortSignal>();
    signal->_reason = reason.value_or(Exception {ExceptionCode::AbortError});
    return signal;
  }

  Ref<AbortSignal> AbortSignal::Timeout(size_t milliseconds) noexcept
  {
    auto signal = CreateRef<AbortSignal>();

    // SPEC-VIOLATION(DOM): global objects not currently supported.
    // TODO(impl): ABORT - timeout abort signal. We don't have task queues or timers yet.
    // Let global be signal’s relevant global object.
    // Run steps after a timeout given global, "AbortSignal-timeout", milliseconds, and the following step:
    //   Queue a global task on the timer task source given global to signal abort given signal and a new
    //   "TimeoutError" DOMException.
    //   For the duration of this timeout, if signal has any event listeners
    //   registered for its abort event, there must be a strong reference from global to signal.

    return signal;
  }

  Ref<AbortSignal> AbortSignal::Any(const List<Ref<AbortSignal>> &signals) noexcept
  {
    return AbortAlgorithms::CreateDependentAbortSignal(signals);
  }
}