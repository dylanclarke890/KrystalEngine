#include "Krystal.Booey/SVG/SVGViewElement.hpp"
#include "Krystal.Booey/SVG/Properties/SVGPropertyOwnerRegistry.hpp"
#include "Krystal.Booey/SVG/SVGSvgElement.hpp"

namespace krys::boo::svg
{
  SVGViewElement::SVGViewElement(dom::Document &document) noexcept
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
    _targetElement = WeakPtr<SVGSvgElement>::WithRef(targetElement);
  }

  void SVGViewElement::ResetTargetElement() noexcept
  {
    _targetElement = nullptr;
  }

  void SVGViewElement::AttributeChanged(const dom::QualifiedName &name, Maybe<dom::DOMStringView> oldValue,
                                        Maybe<dom::DOMStringView> value) noexcept
  {
    SVGFitToViewBox::ParseAttribute(name, value);
    SVGZoomAndPan::ParseAttribute(*this, name, value);
    SVGElement::AttributeChanged(name, oldValue, value);
  }

  void SVGViewElement::SVGAttributeChanged(const dom::QualifiedName &attributeName) noexcept
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
