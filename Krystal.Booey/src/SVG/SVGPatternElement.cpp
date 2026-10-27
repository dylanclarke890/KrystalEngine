#include "Krystal.Booey/SVG/SVGPatternElement.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGResourcePattern.hpp"
#include "Krystal.Booey/SVG/PatternAttributes.hpp"
#include "Krystal.Booey/SVG/Properties/SVGPropertyOwnerRegistry.hpp"
#include "Krystal.Booey/SVG/SVGNames.hpp"

namespace krys::boo::svg
{
  SVGPatternElement::SVGPatternElement(dom::Document &document) noexcept
      : SVGElement(document, CreateUnique<PropertyRegistry>(*this), SVGElementInterface::Pattern),
        SVGFitToViewBox(this), SVGTests(this), SVGURIReference(this),
        _x(SVGAnimatedLength::Create(this, SVGLengthMode::Width)),
        _y(SVGAnimatedLength::Create(this, SVGLengthMode::Height)),
        _width(SVGAnimatedLength::Create(this, SVGLengthMode::Width)),
        _height(SVGAnimatedLength::Create(this, SVGLengthMode::Height)),
        _patternUnits(SVGAnimatedEnumeration::Create(this, SVGUnitTypes::SVG_UNIT_TYPE_OBJECTBOUNDINGBOX)),
        _patternContentUnits(
          SVGAnimatedEnumeration::Create(this, SVGUnitTypes::SVG_UNIT_TYPE_USERSPACEONUSE)),
        _patternTransform(SVGAnimatedTransformList::Create(this))
  {
    static bool didRegistration = false;
    if (!didRegistration) KRYS_UNLIKELY
    {
      didRegistration = true;
      PropertyRegistry::RegisterProperty<&SVGPatternElement::_x>(SVGNames::XAttr());
      PropertyRegistry::RegisterProperty<&SVGPatternElement::_y>(SVGNames::YAttr());
      PropertyRegistry::RegisterProperty<&SVGPatternElement::_width>(SVGNames::WidthAttr());
      PropertyRegistry::RegisterProperty<&SVGPatternElement::_height>(SVGNames::HeightAttr());
      PropertyRegistry::RegisterProperty<SVGUnitTypes::SVGUnitType, &SVGPatternElement::_patternUnits>(
        SVGNames::PatternUnitsAttr());
      PropertyRegistry::RegisterProperty<SVGUnitTypes::SVGUnitType, &SVGPatternElement::_patternContentUnits>(
        SVGNames::PatternContentUnitsAttr());
      PropertyRegistry::RegisterProperty<&SVGPatternElement::_patternTransform>(
        SVGNames::PatternTransformAttr());
    }
  }

  void SVGPatternElement::AttributeChanged(const dom::QualifiedName &name, Maybe<dom::DOMStringView> oldValue,
                                           Maybe<dom::DOMStringView> value) noexcept
  {
    auto parseError = SVGParsingError::None;
    auto string = value.value_or(dom::DOMStringView {});

    if (name == SVGNames::PatternUnitsAttr())
    {
      auto propertyValue = SVGPropertyTraits<SVGUnitTypes::SVGUnitType>::FromString(*this, string);
      if (propertyValue > 0)
      {
        _patternUnits->SetBaseValInternal<SVGUnitTypes::SVGUnitType>(propertyValue);
      }
    }
    else if (name == SVGNames::PatternContentUnitsAttr())
    {
      auto propertyValue = SVGPropertyTraits<SVGUnitTypes::SVGUnitType>::FromString(*this, string);
      if (propertyValue > 0)
      {
        _patternContentUnits->SetBaseValInternal<SVGUnitTypes::SVGUnitType>(propertyValue);
      }
    }
    else if (name == SVGNames::PatternTransformAttr())
    {
      _patternTransform->BaseVal()->Parse(string);
    }
    else if (name == SVGNames::XAttr())
    {
      _x->SetBaseValInternal(SVGLengthValue::Construct(SVGLengthMode::Width, string, parseError));
    }
    else if (name == SVGNames::YAttr())
    {
      _y->SetBaseValInternal(SVGLengthValue::Construct(SVGLengthMode::Height, string, parseError));
    }
    else if (name == SVGNames::WidthAttr())
    {
      _width->SetBaseValInternal(SVGLengthValue::Construct(SVGLengthMode::Width, string, parseError,
                                                           SVGLengthNegativeValuesMode::Forbid));
    }
    else if (name == SVGNames::HeightAttr())
    {
      _height->SetBaseValInternal(SVGLengthValue::Construct(SVGLengthMode::Height, string, parseError,
                                                            SVGLengthNegativeValuesMode::Forbid));
    }
    ReportAttributeParsingError(parseError, name, string);

    SVGURIReference::ParseAttribute(name, value);
    SVGTests::ParseAttribute(name, value);
    SVGFitToViewBox::ParseAttribute(name, value);
    SVGElement::AttributeChanged(name, oldValue, value);
  }

  void SVGPatternElement::SVGAttributeChanged(const dom::QualifiedName &attributeName) noexcept
  {
    if (PropertyRegistry::IsKnownAttribute(attributeName) || SVGFitToViewBox::IsKnownAttribute(attributeName)
        || SVGURIReference::IsKnownAttribute(attributeName))
    {
      InstanceInvalidationGuard guard(*this);
      if (PropertyRegistry::IsAnimatedLengthAttribute(attributeName))
      {
        SetPresentationalHintStyleIsDirty();
      }

      UpdateSVGRendererForElementChange();
      return;
    }

    SVGElement::SVGAttributeChanged(attributeName);
  }

  void SVGPatternElement::OnChildrenChanged() noexcept
  {
    SVGElement::OnChildrenChanged();

    if (!IsFinishedParsingChildren())
    {
      return;
    }

    UpdateSVGRendererForElementChange();
  }

  RenderPtr<RenderElement> SVGPatternElement::CreateElementRenderer(RenderStyle &&style) noexcept
  {
    return CreateRenderer<LegacyRenderSVGResourcePattern>(*this, krys::move(style));
  }

  void SVGPatternElement::CollectPatternAttributes(PatternAttributes &attributes) const noexcept
  {
    if (!attributes.HasX() && HasAttribute(SVGNames::XAttr().LocalName()))
    {
      attributes.SetX(X());
    }

    if (!attributes.HasY() && HasAttribute(SVGNames::YAttr().LocalName()))
    {
      attributes.SetY(Y());
    }

    if (!attributes.HasWidth() && HasAttribute(SVGNames::WidthAttr().LocalName()))
    {
      attributes.SetWidth(Width());
    }

    if (!attributes.HasHeight() && HasAttribute(SVGNames::HeightAttr().LocalName()))
    {
      attributes.SetHeight(Height());
    }

    if (!attributes.HasViewBox() && HasAttribute(SVGNames::ViewBoxAttr().LocalName()) && HasValidViewBox())
    {
      attributes.SetViewBox(ViewBox());
    }

    if (!attributes.HasPreserveAspectRatio() && HasAttribute(SVGNames::PreserveAspectRatioAttr().LocalName()))
    {
      attributes.SetPreserveAspectRatio(PreserveAspectRatio());
    }

    if (!attributes.HasPatternUnits() && HasAttribute(SVGNames::PatternUnitsAttr().LocalName()))
    {
      attributes.SetPatternUnits(PatternUnits());
    }

    if (!attributes.HasPatternContentUnits() && HasAttribute(SVGNames::PatternContentUnitsAttr().LocalName()))
    {
      attributes.SetPatternContentUnits(PatternContentUnits());
    }

    if (!attributes.HasPatternTransform() && HasAttribute(SVGNames::PatternTransformAttr().LocalName()))
    {
      attributes.SetPatternTransform(PatternTransform().Concatenate());
    }

    if (!attributes.HasPatternContentElement() && ChildElementCount())
    {
      attributes.SetPatternContentElement(this);
    }
  }

  AffineTransform SVGPatternElement::LocalCoordinateSpaceTransform(CTMScope) const noexcept
  {
    return PatternTransform().Concatenate();
  }
}
