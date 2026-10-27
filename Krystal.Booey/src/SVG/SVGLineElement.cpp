#include "Krystal.Booey/SVG/SVGLineElement.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGShape.hpp"
#include "Krystal.Booey/SVG/Properties/SVGPropertyOwnerRegistry.hpp"
#include "Krystal.Booey/SVG/SVGNames.hpp"

namespace krys::boo::svg
{
  SVGLineElement::SVGLineElement(dom::Document &document) noexcept
      : SVGGeometryElement(document, CreateUnique<PropertyRegistry>(*this), SVGElementInterface::Line),
        _x1(SVGAnimatedLength::Create(this, SVGLengthMode::Width)),
        _y1(SVGAnimatedLength::Create(this, SVGLengthMode::Height)),
        _x2(SVGAnimatedLength::Create(this, SVGLengthMode::Width)),
        _y2(SVGAnimatedLength::Create(this, SVGLengthMode::Height))
  {
    static bool didRegistration = false;
    if (!didRegistration) KRYS_UNLIKELY
    {
      didRegistration = true;
      PropertyRegistry::RegisterProperty<&SVGLineElement::_x1>(SVGNames::X1Attr());
      PropertyRegistry::RegisterProperty<&SVGLineElement::_y1>(SVGNames::Y1Attr());
      PropertyRegistry::RegisterProperty<&SVGLineElement::_x2>(SVGNames::X2Attr());
      PropertyRegistry::RegisterProperty<&SVGLineElement::_y2>(SVGNames::Y2Attr());
    }
  }

  void SVGLineElement::AttributeChanged(const dom::QualifiedName &name, Maybe<dom::DOMStringView> oldValue,
                                        Maybe<dom::DOMStringView> value) noexcept
  {
    auto parseError = SVGParsingError::None;
    auto string = value.value_or(dom::DOMStringView {});

    if (name == SVGNames::X1Attr())
    {
      _x1->SetBaseValInternal(SVGLengthValue::Construct(SVGLengthMode::Width, string, parseError));
    }
    else if (name == SVGNames::Y1Attr())
    {
      _y1->SetBaseValInternal(SVGLengthValue::Construct(SVGLengthMode::Height, string, parseError));
    }
    else if (name == SVGNames::X2Attr())
    {
      _x2->SetBaseValInternal(SVGLengthValue::Construct(SVGLengthMode::Width, string, parseError));
    }
    else if (name == SVGNames::Y2Attr())
    {
      _y2->SetBaseValInternal(SVGLengthValue::Construct(SVGLengthMode::Height, string, parseError));
    }
    ReportAttributeParsingError(parseError, name, string);

    SVGGeometryElement::AttributeChanged(name, oldValue, value);
  }

  void SVGLineElement::SVGAttributeChanged(const dom::QualifiedName &attributeName) noexcept
  {
    if (PropertyRegistry::IsKnownAttribute(attributeName))
    {
      InstanceInvalidationGuard guard(*this);
      UpdateRelativeLengthsInformation();

      if (auto *shape = DynamicDowncast<LegacyRenderSVGShape>(Renderer()))
      {
        shape->SetNeedsShapeUpdate();
      }

      UpdateSVGRendererForElementChange();
      return;
    }

    SVGGeometryElement::SVGAttributeChanged(attributeName);
  }

  bool SVGLineElement::SelfHasRelativeLengths() const noexcept
  {
    return X1().IsRelative() || Y1().IsRelative() || X2().IsRelative() || Y2().IsRelative();
  }
}
