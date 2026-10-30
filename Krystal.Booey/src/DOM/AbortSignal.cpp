#include "Krystal.Booey/DOM/AbortSignal.hpp"
#include "Krystal.Booey/Constants/EventNames.hpp"
#include "Krystal.Booey/DOM/AbortController.hpp"
#include "Krystal.Booey/DOM/Algorithms/AbortAlgorithms.hpp"
#include "Krystal.Booey/DOM/Internals/Event/EventListener.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"

namespace krys::boo::dom
{
  Ref<AbortSignal> AbortSignal::Abort(Maybe<::krys::Any> reason) noexcept
  {
    auto signal = CreateRef<AbortSignal>();
    signal->_reason = reason.value_or(Exception {ExceptionCode::AbortError});
    return signal;
  }

  Ref<AbortSignal> AbortSignal::Timeout(size_t milliseconds) noexcept
  {
    // 1. Let signal be a new AbortSignal object.
    auto signal = CreateRef<AbortSignal>();

    // 2. Let global be signal's relevant global object.
    // SPEC-VIOLATION(DOM): global objects not currently supported.

    // 3. Run steps after a timeout given global, "AbortSignal-timeout", milliseconds, and the following step:
    //   1. Queue a global task on the timer task source given global to signal abort given signal and a new
    //      "TimeoutError" DOMException.
    // For the duration of this timeout, if signal has any event listeners registered for its abort event,
    // there must be a strong reference from global to signal.
    // NOTE: the task holds the signal strongly whether or not it has listeners.
    html::EventLoop::Current().ScheduleTask(
      std::chrono::milliseconds(milliseconds), html::TaskSource::Timer,
      [signal] { AbortAlgorithms::SignalAbort(*signal, Exception {ExceptionCode::TimeoutError}); });

    // 4. Return signal.
    return signal;
  }

  Ref<AbortSignal> AbortSignal::Any(const List<Ref<AbortSignal>> &signals) noexcept
  {
    return AbortAlgorithms::CreateDependentAbortSignal(signals);
  }

  void AbortSignal::DidAddEventListener(const EventListener &listener) noexcept
  {
    if (listener.Type() == EventNames::Abort)
    {
      ++_abortEventListenerCount;
      UpdateHeldBySources();
    }
  }

  void AbortSignal::DidRemoveEventListener(const EventListener &listener) noexcept
  {
    if (listener.Type() == EventNames::Abort && _abortEventListenerCount > 0)
    {
      --_abortEventListenerCount;
      UpdateHeldBySources();
    }
  }

  void AbortSignal::UpdateHeldBySources() noexcept
  {
    bool held = !Aborted() && (_abortEventListenerCount > 0 || !_abortAlgorithms.empty());

    // A reference released here may be the signal's last; it goes once the caller is done with the signal.
    List<RefPtr<AbortSignal>> released;
    for (auto &weakSource : _sourceSignals)
    {
      auto *source = weakSource.get();
      if (!source)
      {
        continue;
      }
      for (auto &dependent : source->_dependentSignals)
      {
        if (dependent.Weak.get() != this)
        {
          continue;
        }
        if (held)
        {
          dependent.Strong = ShareRefPtr(this);
        }
        else if (dependent.Strong)
        {
          released.push_back(krys::move(dependent.Strong));
        }
      }
    }

    if (!released.empty())
    {
      html::EventLoop::Current().QueueMicrotask([released = krys::move(released)] {});
    }
  }
}