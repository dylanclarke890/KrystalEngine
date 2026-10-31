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
    // 1. If signal is aborted, then return.
    if (signal.Aborted())
    {
      return;
    }

    // 2. Append algorithm to signal's abort algorithms.
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
    // 1. If signal is aborted, then return.
    if (signal.Aborted())
    {
      return;
    }

    // 2. Set signal's abort reason to reason if it is given; otherwise to a new "AbortError" DOMException.
    signal._reason = AbortSignal::ReasonOrAbortError(krys::move(reason));

    // An aborted signal can't abort again, so it lets go of its dependents and its sources.
    auto dependentSignals = std::exchange(signal._dependentSignals, {});
    DetachFromSources(signal);

    // 3. Let dependentSignalsToAbort be a new list.
    List<Ref<AbortSignal>> dependentSignalsToAbort {};

    // 4. For each dependentSignal of signal's dependent signals:
    for (const auto &dependent : dependentSignals)
    {
      // 4a. If dependentSignal is not aborted:
      auto *dependentSignal = dependent.Weak.get();
      if (dependentSignal && !dependentSignal->Aborted())
      {
        // 4a.i. Set dependentSignal's abort reason to signal's abort reason.
        dependentSignal->_reason = signal._reason;
        DetachFromSources(*dependentSignal);

        // 4a.ii. Append dependentSignal to dependentSignalsToAbort.
        dependentSignalsToAbort.push_back(ShareRef(*dependentSignal));
      }
    }

    // 5. Run the abort steps for signal.
    RunAbortSteps(signal);

    // 6. For each dependentSignal of dependentSignalsToAbort, run the abort steps for dependentSignal.
    for (const auto &dependentSignal : dependentSignalsToAbort)
    {
      RunAbortSteps(*dependentSignal);
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
    // 1. For each algorithm of signal's abort algorithms: run algorithm.
    // 2. Empty signal's abort algorithms.
    // The list is emptied first, as an algorithm may remove itself.
    for (const auto &algorithm : std::exchange(signal._abortAlgorithms, {}))
    {
      algorithm(signal.Reason());
    }

    // 3. Fire an event named abort at signal.
    EventDispatcher::FireEvent(EventNames::Abort, signal);
  }

  Ref<AbortSignal> AbortAlgorithms::CreateDependentAbortSignal(const List<Ref<AbortSignal>> &signals) noexcept
  {
    // 1. Let resultSignal be a new object implementing signalInterface using realm.
    // There are no realms, and AbortSignal is the only signal interface.
    auto resultSignal = CreateRef<AbortSignal>();

    // 2. For each signal of signals: if signal is aborted, then set resultSignal's abort reason to signal's
    //    abort reason and return resultSignal.
    for (const auto &signal : signals)
    {
      if (signal->Aborted())
      {
        resultSignal->_reason = signal->Reason();
        return resultSignal;
      }
    }

    // 3. Set resultSignal's dependent to true.
    resultSignal->_dependent = true;

    // Appends source to resultSignal's source signals and resultSignal to source's dependent signals, each
    // once.
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

    // 4. For each signal of signals:
    for (const auto &signal : signals)
    {
      // 4a. If signal's dependent is false:
      if (!signal->_dependent)
      {
        // 4a.i. Append signal to resultSignal's source signals.
        // 4a.ii. Append resultSignal to signal's dependent signals.
        link(*signal);
      }
      // 4b. Otherwise, for each sourceSignal of signal's source signals:
      else
      {
        for (const auto &sourceSignal : signal->_sourceSignals)
        {
          if (auto *source = sourceSignal.get())
          {
            // 4b.i. Assert: sourceSignal is not aborted and not dependent.
            krys_debug_assert(!source->Aborted() && !source->_dependent);

            // 4b.ii. Append sourceSignal to resultSignal's source signals.
            // 4b.iii. Append resultSignal to sourceSignal's dependent signals.
            link(*source);
          }
        }
      }
    }

    // 5. Return resultSignal.
    return resultSignal;
  }
}