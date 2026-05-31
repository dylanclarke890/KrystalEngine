#pragma once

#include "Krystal.Lib/Mixins/RefCounted.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include "Krystal.Lib/Pointers/WeakPtr.hpp"

namespace Krys::HTML::Attributes
{
  class Reflection;
}

namespace Krys::HTML
{
  class HTMLElement;

  class ElementInternals : public RefCounted<ElementInternals>
  {
    friend class Attributes::Reflection;

  private:
    WeakPtr<HTMLElement> _target;

  private:
    KRYS_NODISCARD RawPtr<HTMLElement> TargetElement() noexcept
    {
      return _target.get();
    }
  };
}