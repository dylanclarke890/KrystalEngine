#pragma once

#include "Krystal.HTML/DOM/AbortAlgorithmCallback.hpp"
#include "Krystal.HTML/DOM/EventTarget.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Pointers/WeakPtr.hpp"
#include "Krystal.Lib/Time/Duration.hpp"
#include "Krystal.Lib/Types/Any.hpp"
#include "Krystal.Lib/Types/List.hpp"
#include "Krystal.Lib/Types/Pair.hpp"
#include "Krystal.Lib/Types/Set.hpp"

namespace Krys::HTML
{
  /// @see https://dom.spec.whatwg.org/#abortsignal
  class AbortSignal : public EventTarget
  {
    friend class AbortAlgorithms;

  private:
    Krys::Any _reason;
    List<AbortAlgorithmCallback> _abortAlgorithms;
    Set<WeakPtr<AbortSignal>> _sourceSignals;
    Set<WeakPtr<AbortSignal>> _dependentSignals;
    bool _dependent {false};

  public:
#pragma region Abort Signal - https://dom.spec.whatwg.org/#abortsignal

    /// @see https://dom.spec.whatwg.org/#dom-abortsignal-abort
    static Ref<AbortSignal> Abort(Maybe<Any> reason = Null) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-abortsignal-timeout
    static Ref<AbortSignal> Timeout(Milliseconds milliseconds) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-abortsignal-any
    static Ref<AbortSignal> Any(const List<Ref<AbortSignal>> &signals) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-abortsignal-aborted
    KRYS_NODISCARD bool Aborted() const noexcept
    {
      return _reason.has_value();
    }

    /// @see https://dom.spec.whatwg.org/#dom-abortsignal-reason
    KRYS_NODISCARD const Krys::Any &Reason() const noexcept
    {
      return _reason;
    }

    /// @see https://dom.spec.whatwg.org/#dom-abortsignal-throwifaborted
    void ThrowIfAborted() const
    {
      if (Aborted())
      {
        throw Reason();
      }
    }

    /// @see https://dom.spec.whatwg.org/#dom-abortsignal-onabort
    // TODO(impl): ABORT - attribute EventHandler onabort;

#pragma endregion
  };
}