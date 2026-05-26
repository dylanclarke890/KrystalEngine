#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include "Krystal.Lib/Types/Any.hpp"
#include "Krystal.Lib/Types/Maybe.hpp"

namespace Krys::HTML
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
    KRYS_NODISCARD RawPtr<AbortSignal> Signal() const noexcept
    {
      return _signal.get();
    }

    /// @see https://dom.spec.whatwg.org/#dom-abortcontroller-abort
    void Abort(Maybe<::Krys::Any> reason = Null) noexcept;

#pragma endregion
  };
}