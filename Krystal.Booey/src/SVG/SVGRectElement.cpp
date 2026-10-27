#include "Krystal.Booey/SVG/SVGRectElement.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGRect.hpp"
#include "Krystal.Booey/SVG/Properties/SVGPropertyOwnerRegistry.hpp"
#include "Krystal.Booey/SVG/SVGNames.hpp"

namespace krys::boo::svg
{
  SVGRectElement::SVGRectElement(dom::Document &document) noexcept
      : SVGGeometryElement(document, CreateUnique<PropertyRegistry>(*this), SVGElementInterface::Rect),
        _x(SVGAnimatedLength::Create(this, SVGLengthMode::Width)),
        _y(SVGAnimatedLength::Create(this, SVGLengthMode::Height)),
        _width(SVGAnimatedLength::Create(this, SVGLengthMode::Width)),
        _height(SVGAnimatedLength::Create(this, SVGLengthMode::Height)),
        _rx(SVGAnimatedLength::Create(this, SVGLengthMode::Width)),
        _ry(SVGAnimatedLength::Create(this, SVGLengthMode::Height))
  {
    static bool didRegistration = false;
    if (!didRegistration) KRYS_UNLIKELY
    {
      didRegistration = true;
      PropertyRegistry::RegisterProperty<&SVGRectElement::_x>(SVGNames::XAttr());
      PropertyRegistry::RegisterProperty<&SVGRectElement::_y>(SVGNames::YAttr());
      PropertyRegistry::RegisterProperty<&SVGRectElement::_width>(SVGNames::WidthAttr());
      PropertyRegistry::RegisterProperty<&SVGRectElement::_height>(SVGNames::HeightAttr());
      PropertyRegistry::RegisterProperty<&SVGRectElement::_rx>(SVGNames::RxAttr());
      PropertyRegistry::RegisterProperty<&SVGRectElement::_ry>(SVGNames::RyAttr());
    }
  }

  SVGAnimatedProperty *SVGRectElement::PropertyForAttribute(const dom::QualifiedName &name) const noexcept
  {
    if (name == SVGNames::XAttr())
    {
      return _x.get();
    }
    if (name == SVGNames::YAttr())
    {
      return _y.get();
    }
    if (name == SVGNames::WidthAttr())
    {
      return _width.get();
    }
    if (name == SVGNames::HeightAttr())
    {
      return _height.get();
    }
    if (name == SVGNames::RxAttr())
    {
      return _rx.get();
    }
    if (name == SVGNames::RyAttr())
    {
      return _ry.get();
    }
    return nullptr;
  }

  void SVGRectElement::AttributeChanged(const dom::QualifiedName &name, Maybe<dom::DOMStringView> oldValue,
                                        Maybe<dom::DOMStringView> value) noexcept
  {
    auto parseError = SVGParsingError::None;
    auto string = value.value_or(dom::DOMStringView {});

    if (name == SVGNames::XAttr())
    {
      _x->SetBaseValInternal(SVGLengthValue::Construct(SVGLengthMode::Width, string, parseError));
    }
    else if (name == SVGNames::YAttr())
    {
      _y->SetBaseValInternal(SVGLengthValue::Construct(SVGLengthMode::Height, string, parseError));
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
    SVGGeometryElement::AttributeChanged(name, oldValue, value);
  }

  void SVGRectElement::SVGAttributeChanged(const dom::QualifiedName &attributeName) noexcept
  {
    if (PropertyRegistry::IsKnownAttribute(attributeName))
    {
      InstanceInvalidationGuard guard(*this);
      SetPresentationalHintStyleIsDirty();
      return;
    }

    SVGGeometryElement::SVGAttributeChanged(attributeName);
  }

  RenderPtr<RenderElement> SVGRectElement::CreateElementRenderer(RenderStyle &&style) noexcept
  {
    return CreateRenderer<LegacyRenderSVGRect>(*this, krys::move(style));
  }
}
