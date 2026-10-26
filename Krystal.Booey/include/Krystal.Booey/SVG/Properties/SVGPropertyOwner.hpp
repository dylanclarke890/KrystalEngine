#pragma once

#include "Krystal.Core/Attributes.hpp"

namespace krys::boo::svg
{
  class SVGElement;
  class SVGProperty;

  /// @brief Something that owns SVG properties: an element, an animated property, a list or a transform.
  /// Changes to a property are committed up the chain of owners to the element.
  class SVGPropertyOwner
  {
  public:
    virtual ~SVGPropertyOwner() noexcept = default;

    KRYS_NODISCARD virtual SVGPropertyOwner *Owner() const noexcept
    {
      return nullptr;
    }

    KRYS_NODISCARD virtual const SVGElement *AttributeContextElement() const noexcept
    {
      if (!Owner())
      {
        return nullptr;
      }
      return Owner()->AttributeContextElement();
    }

    virtual void CommitPropertyChange(SVGProperty *property) noexcept
    {
      if (!Owner())
      {
        return;
      }
      Owner()->CommitPropertyChange(property);
    }

  protected:
    SVGPropertyOwner() noexcept = default;
  };
}
