#include "Krystal.Booey/SVG/SVGFEDisplacementMapElement.hpp"
#include "Krystal.Booey/SVG/Graphics/Filters/SVGFilterRenderer.hpp"
#include "Krystal.Booey/SVG/Internals/SVGParserUtilities.hpp"
#include "Krystal.Booey/SVG/Properties/SVGPropertyOwnerRegistry.hpp"
#include "Krystal.Booey/SVG/SVGNames.hpp"
#include <cmath>

namespace krys::boo::svg
{
  SVGFEDisplacementMapElement::SVGFEDisplacementMapElement(dom::Document &document) noexcept
      : SVGFilterPrimitiveStandardAttributes(document, CreateUnique<PropertyRegistry>(*this),
                                             SVGElementInterface::FEDisplacementMap),
        _in1(SVGAnimatedString::Create(this)), _in2(SVGAnimatedString::Create(this)),
        _xChannelSelector(SVGAnimatedEnumeration::Create(this, ChannelSelectorType::CHANNEL_A)),
        _yChannelSelector(SVGAnimatedEnumeration::Create(this, ChannelSelectorType::CHANNEL_A)),
        _scale(SVGAnimatedNumber::Create(this))
  {
    static bool didRegistration = false;
    if (!didRegistration) KRYS_UNLIKELY
    {
      didRegistration = true;
      PropertyRegistry::RegisterProperty<&SVGFEDisplacementMapElement::_in1>(SVGNames::InAttr());
      PropertyRegistry::RegisterProperty<&SVGFEDisplacementMapElement::_in2>(SVGNames::In2Attr());
      PropertyRegistry::RegisterProperty<ChannelSelectorType,
                                         &SVGFEDisplacementMapElement::_xChannelSelector>(
        SVGNames::XChannelSelectorAttr());
      PropertyRegistry::RegisterProperty<ChannelSelectorType,
                                         &SVGFEDisplacementMapElement::_yChannelSelector>(
        SVGNames::YChannelSelectorAttr());
      PropertyRegistry::RegisterProperty<&SVGFEDisplacementMapElement::_scale>(SVGNames::ScaleAttr());
    }
  }

  void SVGFEDisplacementMapElement::AttributeChanged(const dom::QualifiedName &name,
                                                     Maybe<dom::DOMStringView> oldValue,
                                                     Maybe<dom::DOMStringView> value) noexcept
  {
    auto string = value.value_or(dom::DOMStringView {});

    if (name == SVGNames::XChannelSelectorAttr())
    {
      auto propertyValue = SVGPropertyTraits<ChannelSelectorType>::FromString(*this, string);
      if (propertyValue != ChannelSelectorType::CHANNEL_UNKNOWN)
      {
        _xChannelSelector->SetBaseValInternal<ChannelSelectorType>(propertyValue);
      }
    }
    else if (name == SVGNames::YChannelSelectorAttr())
    {
      auto propertyValue = SVGPropertyTraits<ChannelSelectorType>::FromString(*this, string);
      if (propertyValue != ChannelSelectorType::CHANNEL_UNKNOWN)
      {
        _yChannelSelector->SetBaseValInternal<ChannelSelectorType>(propertyValue);
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
    else if (name == SVGNames::ScaleAttr())
    {
      _scale->SetBaseValInternal(ToFloatOrZero(string));
    }

    SVGFilterPrimitiveStandardAttributes::AttributeChanged(name, oldValue, value);
  }

  bool SVGFEDisplacementMapElement::SetFilterEffectAttribute(FilterEffect &filterEffect,
                                                             const dom::QualifiedName &attributeName) noexcept
  {
    auto &effect = Downcast<FEDisplacementMap>(filterEffect);
    if (attributeName == SVGNames::XChannelSelectorAttr())
    {
      return effect.SetXChannelSelector(XChannelSelector());
    }
    if (attributeName == SVGNames::YChannelSelectorAttr())
    {
      return effect.SetYChannelSelector(YChannelSelector());
    }
    if (attributeName == SVGNames::ScaleAttr())
    {
      return effect.SetScale(Scale());
    }

    krys_debug_assert_msg(false, "Not reached");
    return false;
  }

  void SVGFEDisplacementMapElement::SVGAttributeChanged(const dom::QualifiedName &attributeName) noexcept
  {
    if (attributeName == SVGNames::InAttr() || attributeName == SVGNames::In2Attr())
    {
      InstanceInvalidationGuard guard(*this);
      UpdateSVGRendererForElementChange();
      return;
    }

    if (attributeName == SVGNames::XChannelSelectorAttr() || attributeName == SVGNames::YChannelSelectorAttr()
        || attributeName == SVGNames::ScaleAttr())
    {
      InstanceInvalidationGuard guard(*this);
      PrimitiveAttributeChanged(attributeName);
      return;
    }

    SVGFilterPrimitiveStandardAttributes::SVGAttributeChanged(attributeName);
  }

  IntOutsets SVGFEDisplacementMapElement::Outsets(const FloatRect &targetBoundingBox,
                                                  SVGUnitTypes::SVGUnitType primitiveUnits) const noexcept
  {
    auto halfScale = std::abs(Scale() / 2);
    auto maxDisplacement = FloatSize {halfScale, halfScale};
    auto adjustedDisplacement =
      SVGFilterRenderer::CalculateResolvedSize(maxDisplacement, targetBoundingBox, primitiveUnits);
    return FEDisplacementMap::CalculateOutsets(adjustedDisplacement);
  }

  RefPtr<FilterEffect> SVGFEDisplacementMapElement::CreateFilterEffect(const FilterEffectVector &,
                                                                       const GraphicsContext &) const noexcept
  {
    return FEDisplacementMap::Create(XChannelSelector(), YChannelSelector(), Scale());
  }
}
