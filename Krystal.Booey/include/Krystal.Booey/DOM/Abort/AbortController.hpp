#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Core/Types/Any.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::dom
{
  class AbortAlgorithms;
}

namespace krys::boo
{
  class AbortSignal;

  /// @see https://dom.spec.whatwg.org/#abortcontroller
  class AbortController
  {
    friend class dom::AbortAlgorithms;

  private:
    /// @see https://dom.spec.whatwg.org/#abortcontroller-signal
    Ref<AbortSignal> _signal;

  public:
    ~AbortController() noexcept;

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
