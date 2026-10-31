#pragma once

#include "Krystal.Booey/DOM/Callbacks/AbortAlgorithmCallback.hpp"
#include "Krystal.Booey/DOM/EventTarget.hpp"
#include "Krystal.Booey/HTML/EventHandlerAttributes.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/Any.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::dom
{
  /// @see https://dom.spec.whatwg.org/#abortsignal
  class AbortSignal : public EventTarget
  {
    friend class AbortAlgorithms;

  private:
    /// @brief A signal that depends on this one: held strongly while it has to stay alive for this signal to
    /// abort it (while it has abort event listeners or abort algorithms).
    /// @see https://dom.spec.whatwg.org/#abortsignal-garbage-collection
    struct DependentSignal
    {
      WeakPtr<AbortSignal> Weak;
      RefPtr<AbortSignal> Strong;
    };

    /// @see https://dom.spec.whatwg.org/#abortsignal-abort-reason
    krys::Any _reason;
    /// @see https://dom.spec.whatwg.org/#abortsignal-abort-algorithms
    List<AbortAlgorithmCallback> _abortAlgorithms;
    /// @see https://dom.spec.whatwg.org/#abortsignal-source-signals
    List<WeakPtr<AbortSignal>> _sourceSignals;
    /// @see https://dom.spec.whatwg.org/#abortsignal-dependent-signals
    List<DependentSignal> _dependentSignals;
    uint32 _abortEventListenerCount {0};
    /// @see https://dom.spec.whatwg.org/#abortsignal-dependent
    bool _dependent {false};

  public:
#pragma region Abort Signal - https://dom.spec.whatwg.org/#abortsignal

    /// @see https://dom.spec.whatwg.org/#dom-abortsignal-abort
    static Ref<AbortSignal> Abort(Maybe<Any> reason = null) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-abortsignal-timeout
    static Ref<AbortSignal> Timeout(size_t milliseconds) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-abortsignal-any
    static Ref<AbortSignal> Any(const List<Ref<AbortSignal>> &signals) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-abortsignal-aborted
    /// @see https://dom.spec.whatwg.org/#abortsignal-aborted
    KRYS_NODISCARD bool Aborted() const noexcept
    {
      return _reason.has_value();
    }

    /// @see https://dom.spec.whatwg.org/#dom-abortsignal-reason
    KRYS_NODISCARD const krys::Any &Reason() const noexcept
    {
      return _reason;
    }

    /// @note The reason is thrown as a C++ exception (a `krys::Any`), since `ExceptionOr` can only hold a
    /// `DOMException`.
    /// @see https://dom.spec.whatwg.org/#dom-abortsignal-throwifaborted
    void ThrowIfAborted() const
    {
      if (Aborted())
      {
        throw Reason();
      }
    }

    /// @see https://dom.spec.whatwg.org/#dom-abortsignal-onabort
    /// @see https://dom.spec.whatwg.org/#abortsignal-onabort
    /// @see https://dom.spec.whatwg.org/#eventdef-abortsignal-abort
    KRYS_DECLARE_EVENT_HANDLER(Abort, u8"abort")

#pragma endregion

  private:
    /// @brief @p reason if it is given, otherwise a new "AbortError" `DOMException`. An empty reason
    /// (undefined) counts as not given.
    KRYS_NODISCARD static krys::Any ReasonOrAbortError(Maybe<krys::Any> &&reason) noexcept;

    void DidAddEventListener(const EventListener &listener) noexcept final;
    void DidRemoveEventListener(const EventListener &listener) noexcept final;

    /// @brief Has the signal's sources hold it strongly while it must stay alive for them to abort it, and
    /// weakly otherwise.
    void UpdateHeldBySources() noexcept;
  };
}