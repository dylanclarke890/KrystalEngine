#pragma once

#include "Krystal.HTML/Events/EventTarget.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Types/Any.hpp"
#include "Krystal.Lib/Types/Func.hpp"
#include "Krystal.Lib/Types/List.hpp"
#include "Krystal.Lib/Types/Maybe.hpp"
#include "Krystal.Lib/Types/Pair.hpp"
#include "Krystal.Lib/Types/Set.hpp"

namespace Krys::HTML
{
  using AbortAlgorithm = Func<void(Any)>;

  class AbortSignal : public EventTarget
  {
    friend class EventTarget;

  private:
    Maybe<::Krys::Any> _reason;
    List<Pair<size_t, AbortAlgorithm>> _abortAlgorithms;
    Set<WeakPtr<AbortSignal>> _sourceSignals;
    Set<WeakPtr<AbortSignal>> _dependentSignals;
    bool _dependent {false};

  public:
#pragma region Abort Signal - https://dom.spec.whatwg.org/#abortsignal

    static Ref<AbortSignal> Abort(Maybe<Any> reason = std::nullopt) noexcept;

    static Ref<AbortSignal> Timeout(Milliseconds milliseconds) noexcept;

    static Ref<AbortSignal> Any(const List<Ref<AbortSignal>> &signals) noexcept;

    KRYS_NODISCARD bool Aborted() const noexcept
    {
      return _reason.has_value();
    }

    KRYS_NODISCARD const Maybe<::Krys::Any> &Reason() const noexcept
    {
      return _reason;
    }

    void ThrowIfAborted() const
    {
      if (Aborted())
      {
        throw Reason();
      }
    }

#pragma endregion
  private:
    /// @see https://dom.spec.whatwg.org/#abortsignal-add
    static size_t Add(const AbortAlgorithm &algorithm, AbortSignal &signal) noexcept;

    /// @see https://dom.spec.whatwg.org/#abortsignal-remove
    static void Remove(size_t algorithm, Ref<AbortSignal> &signal) noexcept;

    /// @see https://dom.spec.whatwg.org/#abortsignal-signal-abort
    static void SignalAbort(Ref<AbortSignal> &signal, Maybe<::Krys::Any> reason = std::nullopt) noexcept;

    /// @see https://dom.spec.whatwg.org/#run-the-abort-steps
    static void RunAbortSteps(AbortSignal &signal) noexcept;

    /// @see https://dom.spec.whatwg.org/#create-a-dependent-abort-signal
    static Ref<AbortSignal> CreateDependentAbortSignal(const List<Ref<AbortSignal>> &signals) noexcept;
  };
}