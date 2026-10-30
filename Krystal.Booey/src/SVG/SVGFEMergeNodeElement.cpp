#include "Krystal.Booey/SVG/SVGFEMergeNodeElement.hpp"
#include "Krystal.Booey/SVG/Properties/SVGPropertyOwnerRegistry.hpp"
#include "Krystal.Booey/SVG/SVGFilterPrimitiveStandardAttributes.hpp"
#include "Krystal.Booey/SVG/SVGNames.hpp"

namespace krys::boo::svg
{
  SVGFEMergeNodeElement::SVGFEMergeNodeElement(dom::Document &document) noexcept
      : SVGElement(document, CreateUnique<PropertyRegistry>(*this), SVGElementInterface::FEMergeNode),
        _in1(SVGAnimatedString::Create(this))
  {
    static bool didRegistration = false;
    if (!didRegistration) KRYS_UNLIKELY
    {
      didRegistration = true;
      PropertyRegistry::RegisterProperty<&SVGFEMergeNodeElement::_in1>(SVGNames::InAttr());
    }
  }

  void SVGFEMergeNodeElement::AttributeChanged(const dom::QualifiedName &name,
                                               Maybe<dom::DOMStringView> oldValue,
                                               Maybe<dom::DOMStringView> value) noexcept
  {
    if (name == SVGNames::InAttr())
    {
      _in1->SetBaseValInternal(dom::DOMString(value.value_or(dom::DOMStringView {})));
    }

    SVGElement::AttributeChanged(name, oldValue, value);
  }

  void SVGFEMergeNodeElement::SVGAttributeChanged(const dom::QualifiedName &attributeName) noexcept
  {
    if (PropertyRegistry::IsKnownAttribute(attributeName))
    {
      krys_debug_assert(attributeName == SVGNames::InAttr());
      InstanceInvalidationGuard guard(*this);
      SVGFilterPrimitiveStandardAttributes::InvalidateFilterPrimitiveParent(this);
      return;
    }

    SVGElement::SVGAttributeChanged(attributeName);
  }
}
