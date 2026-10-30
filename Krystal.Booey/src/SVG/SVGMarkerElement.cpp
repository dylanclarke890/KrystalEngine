#include "Krystal.Booey/SVG/SVGMarkerElement.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGResourceMarker.hpp"
#include "Krystal.Booey/SVG/Properties/SVGPropertyOwnerRegistry.hpp"
#include "Krystal.Booey/SVG/SVGAngle.hpp"
#include "Krystal.Booey/SVG/SVGNames.hpp"

namespace krys::boo::svg
{
  SVGMarkerElement::SVGMarkerElement(dom::Document &document) noexcept
      : SVGElement(document, CreateUnique<PropertyRegistry>(*this), SVGElementInterface::Marker),
        SVGFitToViewBox(this), _refX(SVGAnimatedLength::Create(this, SVGLengthMode::Width)),
        _refY(SVGAnimatedLength::Create(this, SVGLengthMode::Height)),
        _markerWidth(SVGAnimatedLength::Create(this, SVGLengthMode::Width, u8"3")),
        _markerHeight(SVGAnimatedLength::Create(this, SVGLengthMode::Height, u8"3")),
        _markerUnits(SVGAnimatedEnumeration::Create(this, SVGMarkerUnitsType::StrokeWidth)),
        _orientAngle(SVGAnimatedAngle::Create(this)),
        _orientType(SVGAnimatedOrientType::Create(this, SVGMarkerOrientAngle))
  {
    // Spec: If the markerWidth/markerHeight attribute is not specified, the effect is as if a value of "3"
    // were specified.
    static bool didRegistration = false;
    if (!didRegistration) KRYS_UNLIKELY
    {
      didRegistration = true;
      PropertyRegistry::RegisterProperty<&SVGMarkerElement::_refX>(SVGNames::RefXAttr());
      PropertyRegistry::RegisterProperty<&SVGMarkerElement::_refY>(SVGNames::RefYAttr());
      PropertyRegistry::RegisterProperty<&SVGMarkerElement::_markerWidth>(SVGNames::MarkerWidthAttr());
      PropertyRegistry::RegisterProperty<&SVGMarkerElement::_markerHeight>(SVGNames::MarkerHeightAttr());
      PropertyRegistry::RegisterProperty<SVGMarkerUnitsType, &SVGMarkerElement::_markerUnits>(
        SVGNames::MarkerUnitsAttr());
      PropertyRegistry::RegisterProperty<&SVGMarkerElement::_orientAngle, &SVGMarkerElement::_orientType>(
        SVGNames::OrientAttr());
    }
  }

  AffineTransform SVGMarkerElement::ViewBoxToViewTransform(float viewWidth, float viewHeight) const noexcept
  {
    return SVGFitToViewBox::ViewBoxToViewTransform(ViewBox(), PreserveAspectRatio(), viewWidth, viewHeight);
  }

  void SVGMarkerElement::AttributeChanged(const dom::QualifiedName &name, Maybe<dom::DOMStringView> oldValue,
                                          Maybe<dom::DOMStringView> value) noexcept
  {
    auto parseError = SVGParsingError::None;
    auto string = value.value_or(dom::DOMStringView {});

    if (name == SVGNames::MarkerUnitsAttr())
    {
      auto propertyValue = SVGPropertyTraits<SVGMarkerUnitsType>::FromString(*this, string);
      if (propertyValue != SVGMarkerUnitsType::Unknown)
      {
        _markerUnits->SetBaseValInternal<SVGMarkerUnitsType>(propertyValue);
      }
      return;
    }
    if (name == SVGNames::OrientAttr())
    {
      auto pair = SVGPropertyTraits<std::pair<SVGAngleValue, SVGMarkerOrientType>>::FromString(*this, string);
      _orientAngle->SetBaseValInternal(pair.first);
      _orientType->SetBaseValInternal(pair.second);
      return;
    }
    if (name == SVGNames::RefXAttr())
    {
      _refX->SetBaseValInternal(SVGLengthValue::Construct(SVGLengthMode::Width, string, parseError));
    }
    else if (name == SVGNames::RefYAttr())
    {
      _refY->SetBaseValInternal(SVGLengthValue::Construct(SVGLengthMode::Height, string, parseError));
    }
    else if (name == SVGNames::MarkerWidthAttr())
    {
      _markerWidth->SetBaseValInternal(SVGLengthValue::Construct(SVGLengthMode::Width, string, parseError,
                                                                 SVGLengthNegativeValuesMode::Allow, u8"3"));
    }
    else if (name == SVGNames::MarkerHeightAttr())
    {
      _markerHeight->SetBaseValInternal(SVGLengthValue::Construct(SVGLengthMode::Height, string, parseError,
                                                                  SVGLengthNegativeValuesMode::Allow, u8"3"));
    }
    ReportAttributeParsingError(parseError, name, string);

    SVGFitToViewBox::ParseAttribute(name, value);
    SVGElement::AttributeChanged(name, oldValue, value);
  }

  void SVGMarkerElement::InvalidateMarkerResource() noexcept
  {
    UpdateSVGRendererForElementChange();
  }

  void SVGMarkerElement::SVGAttributeChanged(const dom::QualifiedName &attributeName) noexcept
  {
    if (PropertyRegistry::IsKnownAttribute(attributeName))
    {
      InstanceInvalidationGuard guard(*this);
      if (PropertyRegistry::IsAnimatedLengthAttribute(attributeName))
      {
        UpdateRelativeLengthsInformation();
      }
      // These properties affect the layout of the RenderSVGResourceMarker itself (due to viewBox +
      // overflowClipRect handling).
      if (attributeName == SVGNames::MarkerWidthAttr() || attributeName == SVGNames::MarkerHeightAttr())
      {
        UpdateSVGRendererForElementChange();
      }
      else
      {
        InvalidateMarkerResource();
      }
      return;
    }

    if (SVGFitToViewBox::IsKnownAttribute(attributeName))
    {
      UpdateSVGRendererForElementChange();
      return;
    }

    SVGElement::SVGAttributeChanged(attributeName);
  }

  void SVGMarkerElement::OnChildrenChanged() noexcept
  {
    SVGElement::OnChildrenChanged();

    if (!IsFinishedParsingChildren())
    {
      return;
    }

    InvalidateMarkerResource();
  }

  dom::DOMStringAtom SVGMarkerElement::Orient() const noexcept
  {
    auto orient = GetAttribute(SVGNames::OrientAttr().LocalName());
    return orient ? dom::DOMStringAtom(*orient) : dom::DOMStringAtom::Null();
  }

  void SVGMarkerElement::SetOrient(const dom::DOMStringAtom &orient) noexcept
  {
    (void)SetAttribute(SVGNames::OrientAttr().LocalName(), dom::DOMString(orient.View()));
  }

  void SVGMarkerElement::SetOrientToAuto() noexcept
  {
    (void)_orientType->SetBaseVal(SVGMarkerOrientAuto);
    InvalidateMarkerResource();
  }

  void SVGMarkerElement::SetOrientToAngle(const SVGAngle &angle) noexcept
  {
    (void)_orientAngle->BaseVal()->NewValueSpecifiedUnits(krys::ToUnderlying(angle.UnitType()),
                                                          angle.ValueInSpecifiedUnits());
    InvalidateMarkerResource();
  }

  void SVGMarkerElement::SetOrientToAutoStartReverse() noexcept
  {
    (void)_orientType->SetBaseVal(SVGMarkerOrientAutoStartReverse);
    InvalidateMarkerResource();
  }

  RenderPtr<RenderElement> SVGMarkerElement::CreateElementRenderer(RenderStyle &&style) noexcept
  {
    return CreateRenderer<LegacyRenderSVGResourceMarker>(*this, krys::move(style));
  }

  bool SVGMarkerElement::SelfHasRelativeLengths() const noexcept
  {
    return RefX().IsRelative() || RefY().IsRelative() || MarkerWidth().IsRelative()
           || MarkerHeight().IsRelative();
  }
}
