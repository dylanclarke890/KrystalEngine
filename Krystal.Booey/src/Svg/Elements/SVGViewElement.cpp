#include "Krystal.Booey/Svg/Elements/SVGViewElement.hpp"
#include "Krystal.Booey/Svg/Elements/SVGSvgElement.hpp"
#include "Krystal.Booey/Svg/Properties/SVGPropertyOwnerRegistry.hpp"

namespace krys::boo
{
  SVGViewElement::SVGViewElement(Document &document) noexcept
      : SVGElement(document, CreateUnique<PropertyRegistry>(*this), SVGElementInterface::View),
        SVGFitToViewBox(this)
  {
  }

  const SVGSvgElement *SVGViewElement::TargetElement() const noexcept
  {
    return _targetElement.get();
  }

  void SVGViewElement::SetTargetElement(const SVGSvgElement &targetElement) noexcept
  {
    _targetElement = CreateWeakPtr(&targetElement);
  }

  void SVGViewElement::ResetTargetElement() noexcept
  {
    _targetElement = nullptr;
  }

  void SVGViewElement::AttributeChanged(const QualifiedName &name, Maybe<DOMStringView> oldValue,
                                        Maybe<DOMStringView> value) noexcept
  {
    SVGFitToViewBox::ParseAttribute(name, value);
    SVGElement::AttributeChanged(name, oldValue, value);
  }

  void SVGViewElement::SVGAttributeChanged(const QualifiedName &attributeName) noexcept
  {
    // We ignore changes to SVGNames::viewTargetAttr, which is deprecated and unused.
    if (PropertyRegistry::IsKnownAttribute(attributeName))
    {
      return;
    }

    if (SVGFitToViewBox::IsKnownAttribute(attributeName))
    {
      auto *targetElement = _targetElement.get();
      if (!targetElement)
      {
        return;
      }
      targetElement->InheritViewAttributes(*this);
      targetElement->UpdateSVGRendererForElementChange();
      return;
    }

    SVGElement::SVGAttributeChanged(attributeName);
  }
}
