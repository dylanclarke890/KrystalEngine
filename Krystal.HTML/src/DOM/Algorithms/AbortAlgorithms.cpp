#include "Krystal.HTML/DOM/Algorithms/AbortAlgorithms.hpp"
#include "Krystal.HTML/Constants/EventNames.hpp"
#include "Krystal.HTML/DOM/AbortController.hpp"
#include "Krystal.HTML/DOM/AbortSignal.hpp"
#include "Krystal.HTML/DOM/Algorithms/EventDispatcher.hpp"
#include <cassert>

namespace Krys::HTML
{
  void AbortAlgorithms::SignalAbort(AbortController &controller, Maybe<Krys::Any> reason) noexcept
  {
    SignalAbort(*controller._signal, reason);
  }

  void AbortAlgorithms::Add(const AbortAlgorithmCallback &algorithm, AbortSignal &signal) noexcept
  {
    if (signal.Aborted())
    {
      return;
    }

    if (!std::ranges::contains(signal._abortAlgorithms, algorithm))
    {
      signal._abortAlgorithms.push_back(algorithm);
    }
  }

  void AbortAlgorithms::Remove(const AbortAlgorithmCallback &algorithm, AbortSignal &signal) noexcept
  {
    std::erase(signal._abortAlgorithms, algorithm);
  }

  void AbortAlgorithms::SignalAbort(AbortSignal &signal, Maybe<Krys::Any> reason) noexcept
  {
    if (signal.Aborted())
    {
      return;
    }

    signal._reason = reason.value_or(Exception {ExceptionCode::AbortError});

    List<Ref<AbortSignal>> dependentSignalsToAbort {};
    for (const auto &dependentSignal : signal._dependentSignals)
    {
      if (!dependentSignal->Aborted())
      {
        dependentSignal->_reason = signal._reason;
        dependentSignalsToAbort.push_back(ShareRef(*dependentSignal));
      }
    }

    RunAbortSteps(signal);

    for (const auto &dependantSignal : dependentSignalsToAbort)
    {
      RunAbortSteps(*dependantSignal);
    }
  }

  void AbortAlgorithms::RunAbortSteps(AbortSignal &signal) noexcept
  {
    for (const auto &algorithm : signal._abortAlgorithms)
    {
      algorithm(signal.Reason());
    }

    signal._abortAlgorithms.clear();

    EventDispatcher::FireEvent(EventNames::Abort, signal);
  }

  Ref<AbortSignal> AbortAlgorithms::CreateDependentAbortSignal(const List<Ref<AbortSignal>> &signals) noexcept
  {
    // SPEC-VIOLATION(DOM): Let resultSignal be a new object implementing signalInterface using realm.
    // We don't implement realms and only have AbortSignal, so we just create a new AbortSignal object.

    auto resultSignal = CreateRef<AbortSignal>();

    for (const auto &signal : signals)
    {
      if (signal->Aborted())
      {
        resultSignal->_reason = signal->Reason();
        return resultSignal;
      }
    }

    resultSignal->_dependent = true;

    for (const auto &signal : signals)
    {
      if (!signal->_dependent)
      {
        if (!std::ranges::any_of(resultSignal->_sourceSignals,
                                 [&](auto &s) { return s.get() == signal; }))
        {
          resultSignal->_sourceSignals.push_back(CreateWeakRef(*signal));
        }
        if (!std::ranges::any_of(signal->_dependentSignals,
                                 [&](auto &s) { return s.get() == resultSignal; }))
        {
          signal->_dependentSignals.push_back(CreateWeakRef(*resultSignal));
        }
      }
      else
      {
        for (const auto &sourceSignal : signal->_sourceSignals)
        {
          if (auto srcSignal = sourceSignal.lock())
          {
            assert(!sourceSignal->Aborted() && !sourceSignal->_dependent);

            if (!std::ranges::any_of(resultSignal->_sourceSignals,
                                     [&](auto &s) { return s.get() == signal; }))
            {
              resultSignal->_sourceSignals.push_back(CreateWeakRef(*signal));
            }
            if (!std::ranges::any_of(signal->_dependentSignals,
                                     [&](auto &s) { return s.get() == resultSignal; }))
            {
              signal->_dependentSignals.push_back(CreateWeakRef(*resultSignal));
            }
          }
        }
      }
    }

    return resultSignal;
  }
}