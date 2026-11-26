#pragma once

#include "Krystal.Booey/Dom/Events/EventTarget.hpp"
#include "Krystal.Booey/Dom/Types/ExceptionOr.hpp"
#include "Krystal.Booey/Html/Attributes/EventHandlerAttributes.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Containers/List.hpp"
#include "Krystal.Core/Memory/WeakPtr.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/Func.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::dom
{
  class AbortAlgorithms;
}

namespace krys::boo
{
  class AbortAlgorithmHandle;
  class Document;

  /// @see https://dom.spec.whatwg.org/#abortsignal
  class AbortSignal : public EventTarget
  {
    friend class AbortAlgorithmHandle;
    friend class dom::AbortAlgorithms;

  private:
    /// @brief An abort algorithm, with the identity its handle removes it by.
    struct AbortAlgorithm
    {
      uint64 Id;
      Func<void(const Exception &reason)> Run;
    };

    /// @brief A signal that depends on this one: held strongly while it has to stay alive for this signal to
    /// abort it (while it has abort event listeners or abort algorithms).
    /// @see https://dom.spec.whatwg.org/#abort-signal-garbage-collection
    struct DependentSignal
    {
      WeakPtr<AbortSignal> Weak;
      RefPtr<AbortSignal> Strong;
    };

    /// @see https://dom.spec.whatwg.org/#abortsignal-abort-reason
    Maybe<Exception> _reason;
    /// @see https://dom.spec.whatwg.org/#abortsignal-abort-algorithms
    List<AbortAlgorithm> _abortAlgorithms;
    /// @see https://dom.spec.whatwg.org/#abortsignal-source-signals
    List<WeakPtr<AbortSignal>> _sourceSignals;
    /// @see https://dom.spec.whatwg.org/#abortsignal-dependent-signals
    List<DependentSignal> _dependentSignals;
    uint64 _lastAbortAlgorithmId {0};
    uint32 _abortEventListenerCount {0};
    /// @see https://dom.spec.whatwg.org/#abortsignal-dependent
    bool _dependent {false};

  public:
#pragma region Abort Signal - https://dom.spec.whatwg.org/#abortsignal

    /// @see https://dom.spec.whatwg.org/#dom-abortsignal-abort
    static Ref<AbortSignal> Abort(Maybe<Exception> reason = null) noexcept;

    /// @brief A signal that aborts once @p document, standing in for the global, has been active for
    /// @p milliseconds.
    /// @see https://dom.spec.whatwg.org/#dom-abortsignal-timeout
    static Ref<AbortSignal> Timeout(Document &document, size_t milliseconds) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-abortsignal-any
    static Ref<AbortSignal> Any(const List<Ref<AbortSignal>> &signals) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-abortsignal-aborted
    /// @see https://dom.spec.whatwg.org/#abortsignal-aborted
    KRYS_NODISCARD bool Aborted() const noexcept
    {
      return _reason.has_value();
    }

    /// @brief The abort reason, or nothing (the spec's undefined) if the signal isn't aborted.
    /// @see https://dom.spec.whatwg.org/#dom-abortsignal-reason
    KRYS_NODISCARD const Maybe<Exception> &Reason() const noexcept
    {
      return _reason;
    }

    /// @see https://dom.spec.whatwg.org/#dom-abortsignal-throwifaborted
    KRYS_NODISCARD ExceptionOr<void> ThrowIfAborted() const noexcept
    {
      if (Aborted())
      {
        return Exception(*_reason);
      }
      return {};
    }

    /// @see https://dom.spec.whatwg.org/#dom-abortsignal-onabort
    /// @see https://dom.spec.whatwg.org/#abortsignal-onabort
    /// @see https://dom.spec.whatwg.org/#eventdef-abortsignal-abort
    KRYS_DECLARE_EVENT_HANDLER(Abort, u8"abort")

#pragma endregion

  private:
    /// @brief @p reason if it is given, otherwise a new "AbortError" `DOMException`.
    KRYS_NODISCARD static Exception ReasonOrAbortError(Maybe<Exception> reason) noexcept;

    /// @brief Whether the signal still has the abort algorithm with @p id.
    KRYS_NODISCARD bool HasAbortAlgorithm(uint64 id) const noexcept;

    void DidAddEventListener(const EventListener &listener) noexcept final;

    void DidRemoveEventListener(const EventListener &listener) noexcept final;

    /// @brief Has the signal's sources hold it strongly while it must stay alive for them to abort it, and
    /// weakly otherwise.
    /// @see https://dom.spec.whatwg.org/#abort-signal-garbage-collection
    void UpdateHeldBySources() noexcept;
  };
}
