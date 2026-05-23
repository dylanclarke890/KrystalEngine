#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include "Krystal.Lib/Types/Any.hpp"
#include "Krystal.Lib/Types/Maybe.hpp"

namespace Krys::HTML
{
  class AbortSignal;

  class AbortController
  {
  private:
    Ref<AbortSignal> _signal;

  public:
    AbortController() noexcept;

    ~AbortController() noexcept = default;

    KRYS_NODISCARD RawPtr<AbortSignal> Signal() const noexcept
    {
      return _signal.get();
    }

    void Abort(Maybe<::Krys::Any> reason = std::nullopt) noexcept;
  };
}