#pragma once

#include "Krystal.HTML/Events/EventTarget.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"

namespace Krys::HTML
{
  class AbortSignal : public EventTarget
  {
  public:
    KRYS_NODISCARD bool Aborted() const noexcept
    {
      return false;
    }
  };
}