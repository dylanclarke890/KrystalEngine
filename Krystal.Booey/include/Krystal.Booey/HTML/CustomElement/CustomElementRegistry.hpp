#pragma once

#include "Krystal.Core/RefCounted.hpp"

namespace krys::boo::html
{
  // TODO(impl): CUSTOM-ELEMENTS
  class CustomElementRegistry : public RefCounted<CustomElementRegistry>
  {
  public:
    KRYS_NODISCARD bool IsScoped() const noexcept
    {
      return false;
    }
  };
}