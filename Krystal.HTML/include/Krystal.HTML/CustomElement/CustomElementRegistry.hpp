#pragma once

#include "Krystal.Lib/Mixins/RefCounted.hpp"

namespace Krys::HTML
{
  // TODO(impl): Implement this class
  class CustomElementRegistry : public RefCounted<CustomElementRegistry>
  {
  public:
    KRYS_NODISCARD bool IsScoped() const noexcept
    {
      return false;
    }
  };
}