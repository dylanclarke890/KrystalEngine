#include "Krystal.Booey/SVG/SVGMaskElement.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGResourceMasker.hpp"
#include "Krystal.Booey/SVG/Properties/SVGPropertyOwnerRegistry.hpp"
#include "Krystal.Booey/SVG/SVGNames.hpp"

namespace krys::boo::svg
{
  SVGMaskElement::SVGMaskElement(dom::Document &document) noexcept
      : SVGElement(document, CreateUnique<PropertyRegistry>(*this), SVGElementInterface::Mask),
        SVGTests(this), _x(SVGAnimatedLength::Create(this, SVGLengthMode::Width, u8"-10%")),
        _y(SVGAnimatedLength::Create(this, SVGLengthMode::Height, u8"-10%")),
        _width(SVGAnimatedLength::Create(this, SVGLengthMode::Width, u8"120%")),
        _height(SVGAnimatedLength::Create(this, SVGLengthMode::Height, u8"120%")),
        _maskUnits(SVGAnimatedEnumeration::Create(this, SVGUnitTypes::SVG_UNIT_TYPE_OBJECTBOUNDINGBOX)),
        _maskContentUnits(SVGAnimatedEnumeration::Create(this, SVGUnitTypes::SVG_UNIT_TYPE_USERSPACEONUSE))
  {
    // Spec: If the x/y attribute is not specified, the effect is as if a value of "-10%" were specified.
    // Spec: If the width/height attribute is not specified, the effect is as if a value of "120%" were
    // specified.
    static bool didRegistration = false;
    if (!didRegistration) KRYS_UNLIKELY
    {
      didRegistration = true;
      PropertyRegistry::RegisterProperty<&SVGMaskElement::_x>(SVGNames::XAttr());
      PropertyRegistry::RegisterProperty<&SVGMaskElement::_y>(SVGNames::YAttr());
      PropertyRegistry::RegisterProperty<&SVGMaskElement::_width>(SVGNames::WidthAttr());
      PropertyRegistry::RegisterProperty<&SVGMaskElement::_height>(SVGNames::HeightAttr());
      PropertyRegistry::RegisterProperty<SVGUnitTypes::SVGUnitType, &SVGMaskElement::_maskUnits>(
        SVGNames::MaskUnitsAttr());
      PropertyRegistry::RegisterProperty<SVGUnitTypes::SVGUnitType, &SVGMaskElement::_maskContentUnits>(
        SVGNames::MaskContentUnitsAttr());
    }
  }

  void SVGMaskElement::AttributeChanged(const dom::QualifiedName &name, Maybe<dom::DOMStringView> oldValue,
                                        Maybe<dom::DOMStringView> value) noexcept
  {
    auto parseError = SVGParsingError::None;
    auto string = value.value_or(dom::DOMStringView {});

    if (name == SVGNames::MaskUnitsAttr())
    {
      auto propertyValue = SVGPropertyTraits<SVGUnitTypes::SVGUnitType>::FromString(*this, string);
      if (propertyValue > 0)
      {
        _maskUnits->SetBaseValInternal<SVGUnitTypes::SVGUnitType>(propertyValue);
      }
    }
    else if (name == SVGNames::MaskContentUnitsAttr())
    {
      auto propertyValue = SVGPropertyTraits<SVGUnitTypes::SVGUnitType>::FromString(*this, string);
      if (propertyValue > 0)
      {
        _maskContentUnits->SetBaseValInternal<SVGUnitTypes::SVGUnitType>(propertyValue);
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

    SVGTests::ParseAttribute(name, value);
    SVGElement::AttributeChanged(name, oldValue, value);
  }

  void SVGMaskElement::SVGAttributeChanged(const dom::QualifiedName &attributeName) noexcept
  {
    if (PropertyRegistry::IsAnimatedLengthAttribute(attributeName))
    {
      InstanceInvalidationGuard guard(*this);
      SetPresentationalHintStyleIsDirty();
      return;
    }

    if (PropertyRegistry::IsKnownAttribute(attributeName))
    {
      UpdateSVGRendererForElementChange();
      return;
    }

    SVGElement::SVGAttributeChanged(attributeName);
  }

  void SVGMaskElement::OnChildrenChanged() noexcept
  {
    SVGElement::OnChildrenChanged();

    if (!IsFinishedParsingChildren())
    {
      return;
    }

    UpdateSVGRendererForElementChange();
  }

  RenderPtr<RenderElement> SVGMaskElement::CreateElementRenderer(RenderStyle &&style) noexcept
  {
    return CreateRenderer<LegacyRenderSVGResourceMasker>(*this, krys::move(style));
  }
}
