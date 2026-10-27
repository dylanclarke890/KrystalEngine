#include "Krystal.Booey/SVG/SVGFECompositeElement.hpp"
#include "Krystal.Booey/SVG/Internals/SVGParserUtilities.hpp"
#include "Krystal.Booey/SVG/Properties/SVGPropertyOwnerRegistry.hpp"
#include "Krystal.Booey/SVG/SVGNames.hpp"

namespace krys::boo::svg
{
  SVGFECompositeElement::SVGFECompositeElement(dom::Document &document) noexcept
      : SVGFilterPrimitiveStandardAttributes(document, CreateUnique<PropertyRegistry>(*this),
                                             SVGElementInterface::FEComposite),
        _in1(SVGAnimatedString::Create(this)), _in2(SVGAnimatedString::Create(this)),
        _svgOperator(SVGAnimatedEnumeration::Create(this, CompositeOperationType::FECOMPOSITE_OPERATOR_OVER)),
        _k1(SVGAnimatedNumber::Create(this)), _k2(SVGAnimatedNumber::Create(this)),
        _k3(SVGAnimatedNumber::Create(this)), _k4(SVGAnimatedNumber::Create(this))
  {
    static bool didRegistration = false;
    if (!didRegistration) KRYS_UNLIKELY
    {
      didRegistration = true;
      PropertyRegistry::RegisterProperty<&SVGFECompositeElement::_in1>(SVGNames::InAttr());
      PropertyRegistry::RegisterProperty<&SVGFECompositeElement::_in2>(SVGNames::In2Attr());
      PropertyRegistry::RegisterProperty<CompositeOperationType, &SVGFECompositeElement::_svgOperator>(
        SVGNames::OperatorAttr());
      PropertyRegistry::RegisterProperty<&SVGFECompositeElement::_k1>(SVGNames::K1Attr());
      PropertyRegistry::RegisterProperty<&SVGFECompositeElement::_k2>(SVGNames::K2Attr());
      PropertyRegistry::RegisterProperty<&SVGFECompositeElement::_k3>(SVGNames::K3Attr());
      PropertyRegistry::RegisterProperty<&SVGFECompositeElement::_k4>(SVGNames::K4Attr());
    }
  }

  void SVGFECompositeElement::AttributeChanged(const dom::QualifiedName &name,
                                               Maybe<dom::DOMStringView> oldValue,
                                               Maybe<dom::DOMStringView> value) noexcept
  {
    auto string = value.value_or(dom::DOMStringView {});

    if (name == SVGNames::OperatorAttr())
    {
      auto propertyValue = SVGPropertyTraits<CompositeOperationType>::FromString(*this, string);
      if (propertyValue != CompositeOperationType::FECOMPOSITE_OPERATOR_UNKNOWN)
      {
        _svgOperator->SetBaseValInternal<CompositeOperationType>(propertyValue);
      }
    }
    else if (name == SVGNames::InAttr())
    {
      _in1->SetBaseValInternal(dom::DOMString(string));
    }
    else if (name == SVGNames::In2Attr())
    {
      _in2->SetBaseValInternal(dom::DOMString(string));
    }
    else if (name == SVGNames::K1Attr())
    {
      _k1->SetBaseValInternal(ToFloatOrZero(string));
    }
    else if (name == SVGNames::K2Attr())
    {
      _k2->SetBaseValInternal(ToFloatOrZero(string));
    }
    else if (name == SVGNames::K3Attr())
    {
      _k3->SetBaseValInternal(ToFloatOrZero(string));
    }
    else if (name == SVGNames::K4Attr())
    {
      _k4->SetBaseValInternal(ToFloatOrZero(string));
    }

    SVGFilterPrimitiveStandardAttributes::AttributeChanged(name, oldValue, value);
  }

  bool SVGFECompositeElement::SetFilterEffectAttribute(FilterEffect &filterEffect,
                                                       const dom::QualifiedName &attributeName) noexcept
  {
    auto &effect = Downcast<FEComposite>(filterEffect);
    if (attributeName == SVGNames::OperatorAttr())
    {
      return effect.SetOperation(SVGOperator());
    }
    if (attributeName == SVGNames::K1Attr())
    {
      return effect.SetK1(K1());
    }
    if (attributeName == SVGNames::K2Attr())
    {
      return effect.SetK2(K2());
    }
    if (attributeName == SVGNames::K3Attr())
    {
      return effect.SetK3(K3());
    }
    if (attributeName == SVGNames::K4Attr())
    {
      return effect.SetK4(K4());
    }

    krys_debug_assert_msg(false, "Not reached");
    return false;
  }

  void SVGFECompositeElement::SVGAttributeChanged(const dom::QualifiedName &attributeName) noexcept
  {
    if (attributeName == SVGNames::InAttr() || attributeName == SVGNames::In2Attr())
    {
      InstanceInvalidationGuard guard(*this);
      UpdateSVGRendererForElementChange();
      return;
    }

    if (attributeName == SVGNames::K1Attr() || attributeName == SVGNames::K2Attr()
        || attributeName == SVGNames::K3Attr() || attributeName == SVGNames::K4Attr()
        || attributeName == SVGNames::OperatorAttr())
    {
      InstanceInvalidationGuard guard(*this);
      PrimitiveAttributeChanged(attributeName);
      return;
    }

    SVGFilterPrimitiveStandardAttributes::SVGAttributeChanged(attributeName);
  }

  RefPtr<FilterEffect> SVGFECompositeElement::CreateFilterEffect(const FilterEffectVector &,
                                                                 const GraphicsContext &) const noexcept
  {
    return FEComposite::Create(SVGOperator(), K1(), K2(), K3(), K4());
  }
}
