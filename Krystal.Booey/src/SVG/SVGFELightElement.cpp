#include "Krystal.Booey/SVG/SVGFELightElement.hpp"
#include "Krystal.Booey/Graphics/Filters/LightSource.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/SVG/Internals/SVGParserUtilities.hpp"
#include "Krystal.Booey/SVG/Properties/SVGPropertyOwnerRegistry.hpp"
#include "Krystal.Booey/SVG/SVGFEDiffuseLightingElement.hpp"
#include "Krystal.Booey/SVG/SVGFESpecularLightingElement.hpp"
#include "Krystal.Booey/SVG/SVGFilterPrimitiveStandardAttributes.hpp"
#include "Krystal.Booey/SVG/SVGNames.hpp"

namespace krys::boo::svg
{
  SVGFELightElement::SVGFELightElement(dom::Document &document, SVGElementInterface interface) noexcept
      : SVGElement(document, CreateUnique<PropertyRegistry>(*this), interface),
        _azimuth(SVGAnimatedNumber::Create(this)), _elevation(SVGAnimatedNumber::Create(this)),
        _x(SVGAnimatedNumber::Create(this)), _y(SVGAnimatedNumber::Create(this)),
        _z(SVGAnimatedNumber::Create(this)), _pointsAtX(SVGAnimatedNumber::Create(this)),
        _pointsAtY(SVGAnimatedNumber::Create(this)), _pointsAtZ(SVGAnimatedNumber::Create(this)),
        _specularExponent(SVGAnimatedNumber::Create(this, 1)),
        _limitingConeAngle(SVGAnimatedNumber::Create(this))
  {
    static bool didRegistration = false;
    if (!didRegistration) KRYS_UNLIKELY
    {
      didRegistration = true;
      PropertyRegistry::RegisterProperty<&SVGFELightElement::_azimuth>(SVGNames::AzimuthAttr());
      PropertyRegistry::RegisterProperty<&SVGFELightElement::_elevation>(SVGNames::ElevationAttr());
      PropertyRegistry::RegisterProperty<&SVGFELightElement::_x>(SVGNames::XAttr());
      PropertyRegistry::RegisterProperty<&SVGFELightElement::_y>(SVGNames::YAttr());
      PropertyRegistry::RegisterProperty<&SVGFELightElement::_z>(SVGNames::ZAttr());
      PropertyRegistry::RegisterProperty<&SVGFELightElement::_pointsAtX>(SVGNames::PointsAtXAttr());
      PropertyRegistry::RegisterProperty<&SVGFELightElement::_pointsAtY>(SVGNames::PointsAtYAttr());
      PropertyRegistry::RegisterProperty<&SVGFELightElement::_pointsAtZ>(SVGNames::PointsAtZAttr());
      PropertyRegistry::RegisterProperty<&SVGFELightElement::_specularExponent>(
        SVGNames::SpecularExponentAttr());
      PropertyRegistry::RegisterProperty<&SVGFELightElement::_limitingConeAngle>(
        SVGNames::LimitingConeAngleAttr());
    }
  }

  SVGFELightElement *SVGFELightElement::FindLightElement(const SVGElement *svgElement) noexcept
  {
    for (auto *child = svgElement->FirstChild(); child; child = child->NextSibling())
    {
      if (auto *element = DynamicDowncast<SVGFELightElement>(*child))
      {
        return element;
      }
    }
    return nullptr;
  }

  void SVGFELightElement::AttributeChanged(const dom::QualifiedName &name, Maybe<dom::DOMStringView> oldValue,
                                           Maybe<dom::DOMStringView> value) noexcept
  {
    auto string = value.value_or(dom::DOMStringView {});

    if (name == SVGNames::AzimuthAttr())
    {
      _azimuth->SetBaseValInternal(ToFloatOrZero(string));
    }
    else if (name == SVGNames::ElevationAttr())
    {
      _elevation->SetBaseValInternal(ToFloatOrZero(string));
    }
    else if (name == SVGNames::XAttr())
    {
      _x->SetBaseValInternal(ToFloatOrZero(string));
    }
    else if (name == SVGNames::YAttr())
    {
      _y->SetBaseValInternal(ToFloatOrZero(string));
    }
    else if (name == SVGNames::ZAttr())
    {
      _z->SetBaseValInternal(ToFloatOrZero(string));
    }
    else if (name == SVGNames::PointsAtXAttr())
    {
      _pointsAtX->SetBaseValInternal(ToFloatOrZero(string));
    }
    else if (name == SVGNames::PointsAtYAttr())
    {
      _pointsAtY->SetBaseValInternal(ToFloatOrZero(string));
    }
    else if (name == SVGNames::PointsAtZAttr())
    {
      _pointsAtZ->SetBaseValInternal(ToFloatOrZero(string));
    }
    else if (name == SVGNames::SpecularExponentAttr())
    {
      _specularExponent->SetBaseValInternal(ToFloatOrZero(string));
    }
    else if (name == SVGNames::LimitingConeAngleAttr())
    {
      _limitingConeAngle->SetBaseValInternal(ToFloatOrZero(string));
    }

    SVGElement::AttributeChanged(name, oldValue, value);
  }

  void SVGFELightElement::SVGAttributeChanged(const dom::QualifiedName &attributeName) noexcept
  {
    if (PropertyRegistry::IsKnownAttribute(attributeName))
    {
      auto *parent = DynamicDowncast<SVGElement>(ParentElement());
      if (!parent)
      {
        return;
      }

      auto *renderer = parent->Renderer();
      if (!renderer || !renderer->IsLegacyRenderSVGResourceFilterPrimitive())
      {
        return;
      }

      if (auto *diffuseLighting = DynamicDowncast<SVGFEDiffuseLightingElement>(*parent))
      {
        InstanceInvalidationGuard guard(*this);
        diffuseLighting->LightElementAttributeChanged(this, attributeName);
      }
      else if (auto *specularLighting = DynamicDowncast<SVGFESpecularLightingElement>(*parent))
      {
        InstanceInvalidationGuard guard(*this);
        specularLighting->LightElementAttributeChanged(this, attributeName);
      }

      return;
    }

    SVGElement::SVGAttributeChanged(attributeName);
  }

  void SVGFELightElement::OnChildrenChanged() noexcept
  {
    SVGElement::OnChildrenChanged();

    if (HasNodeFlag(dom::NodeFlags::IsParsingChildren))
    {
      return;
    }

    SVGFilterPrimitiveStandardAttributes::InvalidateFilterPrimitiveParent(this);
  }
}
