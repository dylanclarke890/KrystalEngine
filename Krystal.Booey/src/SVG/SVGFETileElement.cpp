#include "Krystal.Booey/SVG/SVGFETileElement.hpp"
#include "Krystal.Booey/Graphics/Filters/FETile.hpp"
#include "Krystal.Booey/SVG/Properties/SVGPropertyOwnerRegistry.hpp"
#include "Krystal.Booey/SVG/SVGNames.hpp"

namespace krys::boo::svg
{
  SVGFETileElement::SVGFETileElement(dom::Document &document) noexcept
      : SVGFilterPrimitiveStandardAttributes(document, CreateUnique<PropertyRegistry>(*this),
                                             SVGElementInterface::FETile),
        _in1(SVGAnimatedString::Create(this))
  {
    static bool didRegistration = false;
    if (!didRegistration) KRYS_UNLIKELY
    {
      didRegistration = true;
      PropertyRegistry::RegisterProperty<&SVGFETileElement::_in1>(SVGNames::InAttr());
    }
  }

  void SVGFETileElement::AttributeChanged(const dom::QualifiedName &name, Maybe<dom::DOMStringView> oldValue,
                                          Maybe<dom::DOMStringView> value) noexcept
  {
    if (name == SVGNames::InAttr())
    {
      _in1->SetBaseValInternal(dom::DOMString(value.value_or(dom::DOMStringView {})));
    }

    SVGFilterPrimitiveStandardAttributes::AttributeChanged(name, oldValue, value);
  }

  void SVGFETileElement::SVGAttributeChanged(const dom::QualifiedName &attributeName) noexcept
  {
    if (PropertyRegistry::IsKnownAttribute(attributeName))
    {
      krys_debug_assert(attributeName == SVGNames::InAttr());
      InstanceInvalidationGuard guard(*this);
      UpdateSVGRendererForElementChange();
      return;
    }

    SVGFilterPrimitiveStandardAttributes::SVGAttributeChanged(attributeName);
  }

  RefPtr<FilterEffect> SVGFETileElement::CreateFilterEffect(const FilterEffectVector &,
                                                            const GraphicsContext &) const noexcept
  {
    return FETile::Create();
  }
}
