#include "Krystal.Booey/SVG/SVGFilterElement.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGResourceFilter.hpp"
#include "Krystal.Booey/SVG/Properties/SVGPropertyOwnerRegistry.hpp"
#include "Krystal.Booey/SVG/SVGNames.hpp"
#include "Krystal.Booey/SVG/SVGParsingError.hpp"

namespace krys::boo::svg
{
  SVGFilterElement::SVGFilterElement(dom::Document &document) noexcept
      : SVGElement(document, CreateUnique<PropertyRegistry>(*this), SVGElementInterface::Filter),
        SVGURIReference(this),
        _filterUnits(SVGAnimatedEnumeration::Create(this, SVGUnitTypes::SVG_UNIT_TYPE_OBJECTBOUNDINGBOX)),
        _primitiveUnits(SVGAnimatedEnumeration::Create(this, SVGUnitTypes::SVG_UNIT_TYPE_USERSPACEONUSE)),
        _x(SVGAnimatedLength::Create(this, SVGLengthMode::Width, u8"-10%")),
        _y(SVGAnimatedLength::Create(this, SVGLengthMode::Height, u8"-10%")),
        _width(SVGAnimatedLength::Create(this, SVGLengthMode::Width, u8"120%")),
        _height(SVGAnimatedLength::Create(this, SVGLengthMode::Height, u8"120%"))
  {
    // Spec: If the x/y attribute is not specified, the effect is as if a value of "-10%" were specified.
    // Spec: If the width/height attribute is not specified, the effect is as if a value of "120%" were
    // specified.
    static bool didRegistration = false;
    if (!didRegistration) KRYS_UNLIKELY
    {
      didRegistration = true;
      PropertyRegistry::RegisterProperty<SVGUnitTypes::SVGUnitType, &SVGFilterElement::_filterUnits>(
        SVGNames::FilterUnitsAttr());
      PropertyRegistry::RegisterProperty<SVGUnitTypes::SVGUnitType, &SVGFilterElement::_primitiveUnits>(
        SVGNames::PrimitiveUnitsAttr());
      PropertyRegistry::RegisterProperty<&SVGFilterElement::_x>(SVGNames::XAttr());
      PropertyRegistry::RegisterProperty<&SVGFilterElement::_y>(SVGNames::YAttr());
      PropertyRegistry::RegisterProperty<&SVGFilterElement::_width>(SVGNames::WidthAttr());
      PropertyRegistry::RegisterProperty<&SVGFilterElement::_height>(SVGNames::HeightAttr());
    }
  }

  void SVGFilterElement::AttributeChanged(const dom::QualifiedName &name, Maybe<dom::DOMStringView> oldValue,
                                          Maybe<dom::DOMStringView> value) noexcept
  {
    auto parseError = SVGParsingError::None;
    auto string = value.value_or(dom::DOMStringView {});

    if (name == SVGNames::FilterUnitsAttr())
    {
      auto propertyValue = SVGPropertyTraits<SVGUnitTypes::SVGUnitType>::FromString(*this, string);
      if (propertyValue > 0)
      {
        _filterUnits->SetBaseValInternal<SVGUnitTypes::SVGUnitType>(propertyValue);
      }
    }
    else if (name == SVGNames::PrimitiveUnitsAttr())
    {
      auto propertyValue = SVGPropertyTraits<SVGUnitTypes::SVGUnitType>::FromString(*this, string);
      if (propertyValue > 0)
      {
        _primitiveUnits->SetBaseValInternal<SVGUnitTypes::SVGUnitType>(propertyValue);
      }
    }
    else if (name == SVGNames::XAttr())
    {
      _x->SetBaseValInternal(SVGLengthValue::Construct(SVGLengthMode::Width, string, parseError,
                                                       SVGLengthNegativeValuesMode::Allow, u8"-10%"));
    }
    else if (name == SVGNames::YAttr())
    {
      _y->SetBaseValInternal(SVGLengthValue::Construct(SVGLengthMode::Height, string, parseError,
                                                       SVGLengthNegativeValuesMode::Allow, u8"-10%"));
    }
    else if (name == SVGNames::WidthAttr())
    {
      _width->SetBaseValInternal(SVGLengthValue::Construct(SVGLengthMode::Width, string, parseError,
                                                           SVGLengthNegativeValuesMode::Allow, u8"120%"));
    }
    else if (name == SVGNames::HeightAttr())
    {
      _height->SetBaseValInternal(SVGLengthValue::Construct(SVGLengthMode::Height, string, parseError,
                                                            SVGLengthNegativeValuesMode::Allow, u8"120%"));
    }
    ReportAttributeParsingError(parseError, name, string);

    SVGURIReference::ParseAttribute(name, value);
    SVGElement::AttributeChanged(name, oldValue, value);
  }

  void SVGFilterElement::SVGAttributeChanged(const dom::QualifiedName &attributeName) noexcept
  {
    if (PropertyRegistry::IsAnimatedLengthAttribute(attributeName))
    {
      InstanceInvalidationGuard guard(*this);
      SetPresentationalHintStyleIsDirty();
      return;
    }

    if (PropertyRegistry::IsKnownAttribute(attributeName) || SVGURIReference::IsKnownAttribute(attributeName))
    {
      UpdateSVGRendererForElementChange();
      return;
    }

    SVGElement::SVGAttributeChanged(attributeName);
  }

  void SVGFilterElement::OnChildrenChanged() noexcept
  {
    SVGElement::OnChildrenChanged();

    if (!IsFinishedParsingChildren())
    {
      return;
    }

    UpdateSVGRendererForElementChange();
  }

  RenderPtr<RenderElement> SVGFilterElement::CreateElementRenderer(RenderStyle &&style) noexcept
  {
    return CreateRenderer<LegacyRenderSVGResourceFilter>(*this, krys::move(style));
  }

  bool SVGFilterElement::ChildShouldCreateRenderer(const dom::Node &child) const noexcept
  {
    auto *childElement = DynamicDowncast<SVGElement>(child);
    if (!childElement)
    {
      return false;
    }

    switch (childElement->Interface())
    {
      case SVGElementInterface::FEBlend:
      case SVGElementInterface::FEColorMatrix:
      case SVGElementInterface::FEComponentTransfer:
      case SVGElementInterface::FEComposite:
      case SVGElementInterface::FEConvolveMatrix:
      case SVGElementInterface::FEDiffuseLighting:
      case SVGElementInterface::FEDisplacementMap:
      case SVGElementInterface::FEDistantLight:
      case SVGElementInterface::FEDropShadow:
      case SVGElementInterface::FEFlood:
      case SVGElementInterface::FEFuncA:
      case SVGElementInterface::FEFuncB:
      case SVGElementInterface::FEFuncG:
      case SVGElementInterface::FEFuncR:
      case SVGElementInterface::FEGaussianBlur:
      case SVGElementInterface::FEImage:
      case SVGElementInterface::FEMerge:
      case SVGElementInterface::FEMergeNode:
      case SVGElementInterface::FEMorphology:
      case SVGElementInterface::FEOffset:
      case SVGElementInterface::FEPointLight:
      case SVGElementInterface::FESpecularLighting:
      case SVGElementInterface::FESpotLight:
      case SVGElementInterface::FETile:
      case SVGElementInterface::FETurbulence:        return true;
      default:                                       break;
    }

    return false;
  }
}
