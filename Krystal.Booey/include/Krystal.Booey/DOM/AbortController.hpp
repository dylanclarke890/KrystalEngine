#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/Any.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::dom
{
  class AbortSignal;

  /// @see https://dom.spec.whatwg.org/#abortcontroller
  class AbortController
  {
    friend class AbortAlgorithms;

  private:
    Ref<AbortSignal> _signal;

  public:
    ~AbortController() noexcept = default;

#pragma region Abort Controller - https://dom.spec.whatwg.org/#abortcontroller

    /// @see https://dom.spec.whatwg.org/#dom-abortcontroller-abortcontroller
    AbortController() noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-abortcontroller-signal
    KRYS_NODISCARD AbortSignal *Signal() const noexcept
    {
      return _signal.get();
    }

    /// @see https://dom.spec.whatwg.org/#dom-abortcontroller-abort
    void Abort(Maybe<::krys::Any> reason = null) noexcept;

#pragma endregion
  };
}