#include "Krystal.HTML/Abort/AbortSignal.hpp"
#include "Krystal.HTML/Abort/AbortController.hpp"
#include "Krystal.HTML/Algorithms/EventDispatcher.hpp"
#include "Krystal.HTML/Events/EventNames.hpp"
#include "Krystal.HTML/Utils/Exception.hpp"

namespace Krys::HTML
{
  Ref<AbortSignal> AbortSignal::Abort(Maybe<::Krys::Any> reason) noexcept
  {
    auto signal = CreateRef<AbortSignal>();
    signal->_reason = reason.value_or(Exception {ExceptionCode::AbortError});
    return signal;
  }

  Ref<AbortSignal> AbortSignal::Timeout(Milliseconds milliseconds) noexcept
  {
    auto signal = CreateRef<AbortSignal>();

    // SPEC-VIOLATION(DOM): global objects not currently supported.
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
    return CreateDependentAbortSignal(signals);
  }

  size_t AbortSignal::Add(const AbortAlgorithm &algorithm, AbortSignal &signal) noexcept
  {
    if (signal.Aborted())
    {
      return 0;
    }

    auto id = signal._abortAlgorithms.size();
    signal._abortAlgorithms.push_back({id, algorithm});
    return id;
  }

  void AbortSignal::Remove(size_t algorithm, Ref<AbortSignal> &signal) noexcept
  {
    auto &algorithms = signal->_abortAlgorithms;
    std::erase_if(algorithms,
                  [algorithm](const Pair<size_t, AbortAlgorithm> &a) { return a.first == algorithm; });
  }

  void AbortSignal::SignalAbort(Ref<AbortSignal> &signal, Maybe<::Krys::Any> reason) noexcept
  {
    if (signal->Aborted())
    {
      return;
    }

    signal->_reason = reason.value_or(Exception {ExceptionCode::AbortError});
    List<Ref<AbortSignal>> signalsToAbort {};

    for (const auto &dependentSignal : signal->_dependentSignals)
    {
      if (!dependentSignal->Aborted())
      {
        dependentSignal->_reason = signal->_reason;
        signalsToAbort.push_back(ShareRef(*dependentSignal));
      }
    }

    RunAbortSteps(*signal);

    for (const auto &dependantSignal : signalsToAbort)
    {
      RunAbortSteps(*dependantSignal);
    }
  }

  void AbortSignal::RunAbortSteps(AbortSignal &signal) noexcept
  {
    for (const auto &[_id, algorithm] : signal._abortAlgorithms)
    {
      algorithm(*signal.Reason());
    }

    signal._abortAlgorithms.clear();
    EventDispatcher::FireEvent(::Krys::HTML::EventNames::Abort, signal);
  }

  Ref<AbortSignal> AbortSignal::CreateDependentAbortSignal(const List<Ref<AbortSignal>> &signals) noexcept
  {
    auto resultSignal = CreateRef<AbortSignal>();

    for (const auto &signal : signals)
    {
      if (signal->Aborted())
      {
        resultSignal->_reason = *signal->Reason();
        return resultSignal;
      }
    }

    resultSignal->_dependent = true;

    for (const auto &signal : signals)
    {
      if (!signal->_dependent)
      {
        resultSignal->_sourceSignals.insert(CreateWeakRef(*signal));
        signal->_dependentSignals.insert(CreateWeakRef(*resultSignal));
      }
      else
      {
        for (const auto &sourceSignal : signal->_sourceSignals)
        {
          if (auto srcSignal = sourceSignal.lock())
          {
            assert(!sourceSignal->Aborted() && !sourceSignal->_dependent);
            resultSignal->_sourceSignals.insert(CreateWeakPtr(sourceSignal.get()));
            sourceSignal->_dependentSignals.insert(CreateWeakPtr(resultSignal.get()));
          }
        }
      }
    }

    return resultSignal;
  }
}