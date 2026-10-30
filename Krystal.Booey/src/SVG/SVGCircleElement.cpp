#include "Krystal.Booey/SVG/SVGCircleElement.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGEllipse.hpp"
#include "Krystal.Booey/SVG/Properties/SVGPropertyOwnerRegistry.hpp"
#include "Krystal.Booey/SVG/SVGNames.hpp"

namespace krys::boo::svg
{
  SVGCircleElement::SVGCircleElement(dom::Document &document) noexcept
      : SVGGeometryElement(document, CreateUnique<PropertyRegistry>(*this), SVGElementInterface::Circle),
        _cx(SVGAnimatedLength::Create(this, SVGLengthMode::Width)),
        _cy(SVGAnimatedLength::Create(this, SVGLengthMode::Height)),
        _r(SVGAnimatedLength::Create(this, SVGLengthMode::Other))
  {
    static bool didRegistration = false;
    if (!didRegistration) KRYS_UNLIKELY
    {
      didRegistration = true;
      PropertyRegistry::RegisterProperty<&SVGCircleElement::_cx>(SVGNames::CxAttr());
      PropertyRegistry::RegisterProperty<&SVGCircleElement::_cy>(SVGNames::CyAttr());
      PropertyRegistry::RegisterProperty<&SVGCircleElement::_r>(SVGNames::RAttr());
    }
  }

  SVGAnimatedProperty *SVGCircleElement::PropertyForAttribute(const dom::QualifiedName &name) const noexcept
  {
    if (name == SVGNames::CxAttr())
    {
      return _cx.get();
    }
    if (name == SVGNames::CyAttr())
    {
      return _cy.get();
    }
    if (name == SVGNames::RAttr())
    {
      return _r.get();
    }
    return nullptr;
  }

  void SVGCircleElement::AttributeChanged(const dom::QualifiedName &name, Maybe<dom::DOMStringView> oldValue,
                                          Maybe<dom::DOMStringView> value) noexcept
  {
    auto parseError = SVGParsingError::None;
    auto string = value.value_or(dom::DOMStringView {});

    if (name == SVGNames::CxAttr())
    {
      _cx->SetBaseValInternal(SVGLengthValue::Construct(SVGLengthMode::Width, string, parseError));
    }
    else if (name == SVGNames::CyAttr())
    {
      _cy->SetBaseValInternal(SVGLengthValue::Construct(SVGLengthMode::Height, string, parseError));
    }
    else if (name == SVGNames::RAttr())
    {
      _r->SetBaseValInternal(SVGLengthValue::Construct(SVGLengthMode::Other, string, parseError,
                                                       SVGLengthNegativeValuesMode::Forbid));
    }
    ReportAttributeParsingError(parseError, name, string);

    SVGGeometryElement::AttributeChanged(name, oldValue, value);
  }

  void SVGCircleElement::SVGAttributeChanged(const dom::QualifiedName &attributeName) noexcept
  {
    if (PropertyRegistry::IsKnownAttribute(attributeName))
    {
      InstanceInvalidationGuard guard(*this);
      SetPresentationalHintStyleIsDirty();
      return;
    }

    SVGGeometryElement::SVGAttributeChanged(attributeName);
  }

  RenderPtr<RenderElement> SVGCircleElement::CreateElementRenderer(RenderStyle &&style) noexcept
  {
    return CreateRenderer<LegacyRenderSVGEllipse>(*this, krys::move(style));
  }
}
