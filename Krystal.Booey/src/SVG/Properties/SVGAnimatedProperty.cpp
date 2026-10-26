#include "Krystal.Booey/SVG/Properties/SVGAnimatedProperty.hpp"
#include "Krystal.Booey/SVG/SVGElement.hpp"

namespace krys::boo::svg
{
  SVGAnimatedProperty::SVGAnimatedProperty(SVGElement *contextElement) noexcept
      : _contextElement(CreateWeakPtr(contextElement))
  {
  }

  SVGElement *SVGAnimatedProperty::ContextElement() const noexcept
  {
    return _contextElement.get();
  }

  SVGPropertyOwner *SVGAnimatedProperty::Owner() const noexcept
  {
    return _contextElement.get();
  }

  void SVGAnimatedProperty::CommitPropertyChange(SVGProperty *) noexcept
  {
    if (!_contextElement)
    {
      return;
    }
    RefPtr protectedContextElement = ShareRefPtr(_contextElement.get());
    protectedContextElement->CommitPropertyChange(*this);
  }
}
