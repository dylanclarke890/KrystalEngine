#pragma once

#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::html::Attributes
{
  class Reflection;
}

namespace krys::boo::html
{
  class HTMLElement;

  class ElementInternals : public RefCounted<ElementInternals>
  {
    friend class Attributes::Reflection;

  private:
    WeakPtr<HTMLElement> _target;

  private:
    KRYS_NODISCARD HTMLElement *TargetElement() noexcept
    {
      return _target.get();
    }
  };
}