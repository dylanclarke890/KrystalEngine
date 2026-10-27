#include "Krystal.Booey/SVG/SVGEllipseElement.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGEllipse.hpp"
#include "Krystal.Booey/SVG/Properties/SVGPropertyOwnerRegistry.hpp"
#include "Krystal.Booey/SVG/SVGNames.hpp"

namespace krys::boo::svg
{
  SVGEllipseElement::SVGEllipseElement(dom::Document &document) noexcept
      : SVGGeometryElement(document, CreateUnique<PropertyRegistry>(*this), SVGElementInterface::Ellipse),
        _cx(SVGAnimatedLength::Create(this, SVGLengthMode::Width)),
        _cy(SVGAnimatedLength::Create(this, SVGLengthMode::Height)),
        _rx(SVGAnimatedLength::Create(this, SVGLengthMode::Width)),
        _ry(SVGAnimatedLength::Create(this, SVGLengthMode::Height))
  {
    static bool didRegistration = false;
    if (!didRegistration) KRYS_UNLIKELY
    {
      didRegistration = true;
      PropertyRegistry::RegisterProperty<&SVGEllipseElement::_cx>(SVGNames::CxAttr());
      PropertyRegistry::RegisterProperty<&SVGEllipseElement::_cy>(SVGNames::CyAttr());
      PropertyRegistry::RegisterProperty<&SVGEllipseElement::_rx>(SVGNames::RxAttr());
      PropertyRegistry::RegisterProperty<&SVGEllipseElement::_ry>(SVGNames::RyAttr());
    }
  }

  void SVGEllipseElement::AttributeChanged(const dom::QualifiedName &name, Maybe<dom::DOMStringView> oldValue,
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
    else if (name == SVGNames::RxAttr())
    {
      _rx->SetBaseValInternal(SVGLengthValue::Construct(SVGLengthMode::Width, string, parseError,
                                                        SVGLengthNegativeValuesMode::Forbid));
    }
    else if (name == SVGNames::RyAttr())
    {
      _ry->SetBaseValInternal(SVGLengthValue::Construct(SVGLengthMode::Height, string, parseError,
                                                        SVGLengthNegativeValuesMode::Forbid));
    }
    ReportAttributeParsingError(parseError, name, string);

    SVGGeometryElement::AttributeChanged(name, oldValue, value);
  }

  void SVGEllipseElement::SVGAttributeChanged(const dom::QualifiedName &attributeName) noexcept
  {
    if (PropertyRegistry::IsKnownAttribute(attributeName))
    {
      InstanceInvalidationGuard guard(*this);
      SetPresentationalHintStyleIsDirty();
      return;
    }

    SVGGeometryElement::SVGAttributeChanged(attributeName);
  }

  RenderPtr<RenderElement> SVGEllipseElement::CreateElementRenderer(RenderStyle &&style) noexcept
  {
    return CreateRenderer<LegacyRenderSVGEllipse>(*this, krys::move(style));
  }
}
