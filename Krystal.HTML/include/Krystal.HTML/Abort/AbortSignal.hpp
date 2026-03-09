#pragma once

#include "Krystal.HTML/Events/EventTarget.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Types/Func.hpp"
#include "Krystal.Lib/Types/List.hpp"

namespace Krys::HTML
{
  using AbortAlgorithm = Func<void()>;

  class AbortSignal : public EventTarget
  {
  private:
    List<AbortAlgorithm> _abortAlgorithms;

  public:
    KRYS_NODISCARD bool Aborted() const noexcept
    {
      return false;
    }

    void OnAbort(const AbortAlgorithm &abortAlgorithm) noexcept
    {
      _abortAlgorithms.push_back(abortAlgorithm);
    }

    void RemoveOnAbort(const Func<void()> &abortAlgorithm) noexcept
    {
      // TODO(impl):
    }
  };
}