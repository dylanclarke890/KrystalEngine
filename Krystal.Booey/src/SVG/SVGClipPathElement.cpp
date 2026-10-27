#include "Krystal.Booey/SVG/SVGClipPathElement.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGResourceClipper.hpp"
#include "Krystal.Booey/SVG/Properties/SVGPropertyOwnerRegistry.hpp"
#include "Krystal.Booey/SVG/SVGNames.hpp"

namespace krys::boo::svg
{
  SVGClipPathElement::SVGClipPathElement(dom::Document &document) noexcept
      : SVGGraphicsElement(document, CreateUnique<PropertyRegistry>(*this), SVGElementInterface::ClipPath),
        _clipPathUnits(SVGAnimatedEnumeration::Create(this, SVGUnitTypes::SVG_UNIT_TYPE_USERSPACEONUSE))
  {
    static bool didRegistration = false;
    if (!didRegistration) KRYS_UNLIKELY
    {
      didRegistration = true;
      PropertyRegistry::RegisterProperty<SVGUnitTypes::SVGUnitType, &SVGClipPathElement::_clipPathUnits>(
        SVGNames::ClipPathUnitsAttr());
    }
  }

  void SVGClipPathElement::AttributeChanged(const dom::QualifiedName &name,
                                            Maybe<dom::DOMStringView> oldValue,
                                            Maybe<dom::DOMStringView> value) noexcept
  {
    if (name == SVGNames::ClipPathUnitsAttr())
    {
      auto propertyValue = SVGPropertyTraits<SVGUnitTypes::SVGUnitType>::FromString(
        *this, value.value_or(dom::DOMStringView {}));
      if (propertyValue > 0)
      {
        _clipPathUnits->SetBaseValInternal<SVGUnitTypes::SVGUnitType>(propertyValue);
      }
    }

    SVGGraphicsElement::AttributeChanged(name, oldValue, value);
  }

  void SVGClipPathElement::SVGAttributeChanged(const dom::QualifiedName &attributeName) noexcept
  {
    if (PropertyRegistry::IsKnownAttribute(attributeName))
    {
      InstanceInvalidationGuard guard(*this);
      UpdateSVGRendererForElementChange();
      return;
    }

    SVGGraphicsElement::SVGAttributeChanged(attributeName);
  }

  void SVGClipPathElement::OnChildrenChanged() noexcept
  {
    SVGGraphicsElement::OnChildrenChanged();

    if (!IsFinishedParsingChildren())
    {
      return;
    }

    UpdateSVGRendererForElementChange();
  }

  RenderPtr<RenderElement> SVGClipPathElement::CreateElementRenderer(RenderStyle &&style) noexcept
  {
    return CreateRenderer<LegacyRenderSVGResourceClipper>(*this, krys::move(style));
  }
}
