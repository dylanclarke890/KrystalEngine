#include "Krystal.Booey/SVG/SVGFESpecularLightingElement.hpp"
#include "Krystal.Booey/Graphics/Filters/FESpecularLighting.hpp"
#include "Krystal.Booey/Graphics/Filters/LightSource.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/SVG/Internals/SVGParserUtilities.hpp"
#include "Krystal.Booey/SVG/Properties/SVGPropertyOwnerRegistry.hpp"
#include "Krystal.Booey/SVG/SVGFELightElement.hpp"
#include "Krystal.Booey/SVG/SVGNames.hpp"

namespace krys::boo::svg
{
  SVGFESpecularLightingElement::SVGFESpecularLightingElement(dom::Document &document) noexcept
      : SVGFilterPrimitiveStandardAttributes(document, CreateUnique<PropertyRegistry>(*this),
                                             SVGElementInterface::FESpecularLighting),
        _in1(SVGAnimatedString::Create(this)), _specularConstant(SVGAnimatedNumber::Create(this, 1)),
        _specularExponent(SVGAnimatedNumber::Create(this, 1)),
        _surfaceScale(SVGAnimatedNumber::Create(this, 1)),
        _kernelUnitLengthX(SVGAnimatedNumber::Create(this)),
        _kernelUnitLengthY(SVGAnimatedNumber::Create(this))
  {
    static bool didRegistration = false;
    if (!didRegistration) KRYS_UNLIKELY
    {
      didRegistration = true;
      PropertyRegistry::RegisterProperty<&SVGFESpecularLightingElement::_in1>(SVGNames::InAttr());
      PropertyRegistry::RegisterProperty<&SVGFESpecularLightingElement::_specularConstant>(
        SVGNames::SpecularConstantAttr());
      PropertyRegistry::RegisterProperty<&SVGFESpecularLightingElement::_specularExponent>(
        SVGNames::SpecularExponentAttr());
      PropertyRegistry::RegisterProperty<&SVGFESpecularLightingElement::_surfaceScale>(
        SVGNames::SurfaceScaleAttr());
      PropertyRegistry::RegisterProperty<&SVGFESpecularLightingElement::_kernelUnitLengthX,
                                         &SVGFESpecularLightingElement::_kernelUnitLengthY>(
        SVGNames::KernelUnitLengthAttr());
    }
  }

  void SVGFESpecularLightingElement::AttributeChanged(const dom::QualifiedName &name,
                                                      Maybe<dom::DOMStringView> oldValue,
                                                      Maybe<dom::DOMStringView> value) noexcept
  {
    auto string = value.value_or(dom::DOMStringView {});

    if (name == SVGNames::InAttr())
    {
      _in1->SetBaseValInternal(dom::DOMString(string));
    }
    else if (name == SVGNames::SurfaceScaleAttr())
    {
      _surfaceScale->SetBaseValInternal(ToFloatOrZero(string));
    }
    else if (name == SVGNames::SpecularConstantAttr())
    {
      _specularConstant->SetBaseValInternal(ToFloatOrZero(string));
    }
    else if (name == SVGNames::SpecularExponentAttr())
    {
      _specularExponent->SetBaseValInternal(ToFloatOrZero(string));
    }
    else if (name == SVGNames::KernelUnitLengthAttr())
    {
      if (auto result = ParseNumberOptionalNumber(string))
      {
        _kernelUnitLengthX->SetBaseValInternal(result->first);
        _kernelUnitLengthY->SetBaseValInternal(result->second);
      }
    }

    SVGFilterPrimitiveStandardAttributes::AttributeChanged(name, oldValue, value);
  }

  bool
    SVGFESpecularLightingElement::SetFilterEffectAttribute(FilterEffect &filterEffect,
                                                           const dom::QualifiedName &attributeName) noexcept
  {
    auto &effect = Downcast<FESpecularLighting>(filterEffect);
    auto lightElement = [this]
    {
      return SVGFELightElement::FindLightElement(this);
    };

    if (attributeName == SVGNames::LightingColorAttr())
    {
      const auto &style = Renderer()->Style();
      auto color = style.ColorWithColorFilter(style.LightingColor());
      return effect.SetLightingColor(color);
    }
    if (attributeName == SVGNames::SurfaceScaleAttr())
    {
      return effect.SetSurfaceScale(SurfaceScale());
    }
    if (attributeName == SVGNames::SpecularConstantAttr())
    {
      return effect.SetSpecularConstant(SpecularConstant());
    }
    if (attributeName == SVGNames::SpecularExponentAttr())
    {
      return effect.SetSpecularExponent(SpecularExponent());
    }
    if (attributeName == SVGNames::AzimuthAttr())
    {
      return effect.GetLightSource().SetAzimuth(lightElement()->Azimuth());
    }
    if (attributeName == SVGNames::ElevationAttr())
    {
      return effect.GetLightSource().SetElevation(lightElement()->Elevation());
    }
    if (attributeName == SVGNames::XAttr())
    {
      return effect.GetLightSource().SetX(lightElement()->X());
    }
    if (attributeName == SVGNames::YAttr())
    {
      return effect.GetLightSource().SetY(lightElement()->Y());
    }
    if (attributeName == SVGNames::ZAttr())
    {
      return effect.GetLightSource().SetZ(lightElement()->Z());
    }
    if (attributeName == SVGNames::PointsAtXAttr())
    {
      return effect.GetLightSource().SetPointsAtX(lightElement()->PointsAtX());
    }
    if (attributeName == SVGNames::PointsAtYAttr())
    {
      return effect.GetLightSource().SetPointsAtY(lightElement()->PointsAtY());
    }
    if (attributeName == SVGNames::PointsAtZAttr())
    {
      return effect.GetLightSource().SetPointsAtZ(lightElement()->PointsAtZ());
    }
    if (attributeName == SVGNames::LimitingConeAngleAttr())
    {
      return effect.GetLightSource().SetLimitingConeAngle(lightElement()->LimitingConeAngle());
    }

    krys_debug_assert_msg(false, "Not reached");
    return false;
  }

  void SVGFESpecularLightingElement::SVGAttributeChanged(const dom::QualifiedName &attributeName) noexcept
  {
    if (PropertyRegistry::IsKnownAttribute(attributeName))
    {
      InstanceInvalidationGuard guard(*this);
      if (attributeName == SVGNames::InAttr())
      {
        UpdateSVGRendererForElementChange();
      }
      else
      {
        krys_debug_assert(attributeName == SVGNames::SpecularConstantAttr()
                          || attributeName == SVGNames::SpecularExponentAttr()
                          || attributeName == SVGNames::SurfaceScaleAttr()
                          || attributeName == SVGNames::KernelUnitLengthAttr());
        PrimitiveAttributeChanged(attributeName);
      }
      return;
    }

    SVGFilterPrimitiveStandardAttributes::SVGAttributeChanged(attributeName);
  }

  void SVGFESpecularLightingElement::LightElementAttributeChanged(
    const SVGFELightElement *lightElement, const dom::QualifiedName &attributeName) noexcept
  {
    if (SVGFELightElement::FindLightElement(this) != lightElement)
    {
      return;
    }

    // The light element has different attribute names so attrName can identify the requested attribute.
    PrimitiveAttributeChanged(attributeName);
  }

  RefPtr<FilterEffect>
    SVGFESpecularLightingElement::CreateFilterEffect(const FilterEffectVector &,
                                                     const GraphicsContext &) const noexcept
  {
    auto *lightElement = SVGFELightElement::FindLightElement(this);
    if (!lightElement)
    {
      return nullptr;
    }

    auto *renderer = Renderer();
    if (!renderer)
    {
      return nullptr;
    }

    auto lightSource = lightElement->GetLightSource();
    const auto &style = renderer->Style();

    auto color = style.ColorWithColorFilter(style.LightingColor());

    return FESpecularLighting::Create(color, SurfaceScale(), SpecularConstant(), SpecularExponent(),
                                      KernelUnitLengthX(), KernelUnitLengthY(), krys::move(lightSource));
  }
}
