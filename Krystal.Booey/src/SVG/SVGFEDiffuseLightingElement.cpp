#include "Krystal.Booey/SVG/SVGFEDiffuseLightingElement.hpp"
#include "Krystal.Booey/Graphics/Filters/FEDiffuseLighting.hpp"
#include "Krystal.Booey/Graphics/Filters/LightSource.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/SVG/Internals/SVGParserUtilities.hpp"
#include "Krystal.Booey/SVG/Properties/SVGPropertyOwnerRegistry.hpp"
#include "Krystal.Booey/SVG/SVGFELightElement.hpp"
#include "Krystal.Booey/SVG/SVGNames.hpp"

namespace krys::boo::svg
{
  SVGFEDiffuseLightingElement::SVGFEDiffuseLightingElement(dom::Document &document) noexcept
      : SVGFilterPrimitiveStandardAttributes(document, CreateUnique<PropertyRegistry>(*this),
                                             SVGElementInterface::FEDiffuseLighting),
        _in1(SVGAnimatedString::Create(this)), _diffuseConstant(SVGAnimatedNumber::Create(this, 1)),
        _surfaceScale(SVGAnimatedNumber::Create(this, 1)),
        _kernelUnitLengthX(SVGAnimatedNumber::Create(this)),
        _kernelUnitLengthY(SVGAnimatedNumber::Create(this))
  {
    static bool didRegistration = false;
    if (!didRegistration) KRYS_UNLIKELY
    {
      didRegistration = true;
      PropertyRegistry::RegisterProperty<&SVGFEDiffuseLightingElement::_in1>(SVGNames::InAttr());
      PropertyRegistry::RegisterProperty<&SVGFEDiffuseLightingElement::_diffuseConstant>(
        SVGNames::DiffuseConstantAttr());
      PropertyRegistry::RegisterProperty<&SVGFEDiffuseLightingElement::_surfaceScale>(
        SVGNames::SurfaceScaleAttr());
      PropertyRegistry::RegisterProperty<&SVGFEDiffuseLightingElement::_kernelUnitLengthX,
                                         &SVGFEDiffuseLightingElement::_kernelUnitLengthY>(
        SVGNames::KernelUnitLengthAttr());
    }
  }

  void SVGFEDiffuseLightingElement::AttributeChanged(const dom::QualifiedName &name,
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
    else if (name == SVGNames::DiffuseConstantAttr())
    {
      _diffuseConstant->SetBaseValInternal(ToFloatOrZero(string));
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

  bool SVGFEDiffuseLightingElement::SetFilterEffectAttribute(FilterEffect &filterEffect,
                                                             const dom::QualifiedName &attributeName) noexcept
  {
    auto &effect = Downcast<FEDiffuseLighting>(filterEffect);
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
    if (attributeName == SVGNames::DiffuseConstantAttr())
    {
      return effect.SetDiffuseConstant(DiffuseConstant());
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
    if (attributeName == SVGNames::SpecularExponentAttr())
    {
      return effect.GetLightSource().SetSpecularExponent(lightElement()->SpecularExponent());
    }
    if (attributeName == SVGNames::LimitingConeAngleAttr())
    {
      return effect.GetLightSource().SetLimitingConeAngle(lightElement()->LimitingConeAngle());
    }

    krys_debug_assert_msg(false, "Not reached");
    return false;
  }

  void SVGFEDiffuseLightingElement::SVGAttributeChanged(const dom::QualifiedName &attributeName) noexcept
  {
    if (attributeName == SVGNames::InAttr())
    {
      InstanceInvalidationGuard guard(*this);
      UpdateSVGRendererForElementChange();
      return;
    }

    if (attributeName == SVGNames::DiffuseConstantAttr() || attributeName == SVGNames::SurfaceScaleAttr()
        || attributeName == SVGNames::KernelUnitLengthAttr())
    {
      InstanceInvalidationGuard guard(*this);
      PrimitiveAttributeChanged(attributeName);
      return;
    }

    SVGFilterPrimitiveStandardAttributes::SVGAttributeChanged(attributeName);
  }

  void SVGFEDiffuseLightingElement::LightElementAttributeChanged(
    const SVGFELightElement *lightElement, const dom::QualifiedName &attributeName) noexcept
  {
    if (SVGFELightElement::FindLightElement(this) != lightElement)
    {
      return;
    }

    // The light element has different attribute names.
    PrimitiveAttributeChanged(attributeName);
  }

  RefPtr<FilterEffect> SVGFEDiffuseLightingElement::CreateFilterEffect(const FilterEffectVector &,
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

    Color color = style.ColorWithColorFilter(style.LightingColor());

    return FEDiffuseLighting::Create(color, SurfaceScale(), DiffuseConstant(), KernelUnitLengthX(),
                                     KernelUnitLengthY(), krys::move(lightSource));
  }
}
