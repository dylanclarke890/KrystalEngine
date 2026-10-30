#include "Krystal.Booey/DOM/Algorithms/AbortAlgorithms.hpp"
#include "Krystal.Booey/Constants/EventNames.hpp"
#include "Krystal.Booey/DOM/AbortController.hpp"
#include "Krystal.Booey/DOM/AbortSignal.hpp"
#include "Krystal.Booey/DOM/Algorithms/EventDispatcher.hpp"

namespace krys::boo::dom
{
  void AbortAlgorithms::SignalAbort(AbortController &controller, Maybe<krys::Any> reason) noexcept
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
    signal.UpdateHeldBySources();
  }

  void AbortAlgorithms::Remove(const AbortAlgorithmCallback &algorithm, AbortSignal &signal) noexcept
  {
    std::erase(signal._abortAlgorithms, algorithm);
    signal.UpdateHeldBySources();
  }

  void AbortAlgorithms::SignalAbort(AbortSignal &signal, Maybe<krys::Any> reason) noexcept
  {
    if (signal.Aborted())
    {
      return;
    }

    signal._reason = reason.value_or(Exception {ExceptionCode::AbortError});

    // An aborted signal can't abort again, so it lets go of its dependents and its sources.
    auto dependentSignals = std::exchange(signal._dependentSignals, {});
    DetachFromSources(signal);

    List<Ref<AbortSignal>> dependentSignalsToAbort {};
    for (const auto &dependent : dependentSignals)
    {
      auto *dependentSignal = dependent.Weak.get();
      if (dependentSignal && !dependentSignal->Aborted())
      {
        dependentSignal->_reason = signal._reason;
        DetachFromSources(*dependentSignal);
        dependentSignalsToAbort.push_back(ShareRef(*dependentSignal));
      }
    }

    RunAbortSteps(signal);

    for (const auto &dependantSignal : dependentSignalsToAbort)
    {
      RunAbortSteps(*dependantSignal);
    }
  }

  void AbortAlgorithms::DetachFromSources(AbortSignal &signal) noexcept
  {
    for (auto &weakSource : signal._sourceSignals)
    {
      if (auto *source = weakSource.get())
      {
        std::erase_if(source->_dependentSignals, [&](const auto &dependent)
                      { return !dependent.Weak || dependent.Weak.get() == &signal; });
      }
    }
    signal._sourceSignals.clear();
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

    // Links resultSignal and source, each once.
    auto link = [&resultSignal](AbortSignal &source)
    {
      if (!std::ranges::any_of(resultSignal->_sourceSignals, [&](auto &s) { return s.get() == &source; }))
      {
        resultSignal->_sourceSignals.push_back(CreateWeakPtr(&source));
      }
      if (!std::ranges::any_of(source._dependentSignals,
                               [&](auto &d) { return d.Weak.get() == resultSignal.get(); }))
      {
        source._dependentSignals.push_back({CreateWeakPtr(resultSignal.get()), nullptr});
      }
    };

    for (const auto &signal : signals)
    {
      if (!signal->_dependent)
      {
        link(*signal);
      }
      else
      {
        for (const auto &sourceSignal : signal->_sourceSignals)
        {
          if (auto *source = sourceSignal.get())
          {
            krys_debug_assert(!source->Aborted() && !source->_dependent);
            link(*source);
          }
        }
      }
    }

    return resultSignal;
  }
}