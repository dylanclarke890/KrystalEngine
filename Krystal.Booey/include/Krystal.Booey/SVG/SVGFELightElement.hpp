#pragma once

#include "Krystal.Booey/SVG/Properties/SVGAnimatedPropertyImpl.hpp"
#include "Krystal.Booey/SVG/SVGElement.hpp"

namespace krys::boo
{
  class LightSource;
}

namespace krys::boo::svg
{
  /// @brief The base of the light source elements, which light their parent `<feDiffuseLighting>` or
  /// `<feSpecularLighting>`.
  class SVGFELightElement : public SVGElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGFELightElement);

  private:
    Ref<SVGAnimatedNumber> _azimuth;
    Ref<SVGAnimatedNumber> _elevation;
    Ref<SVGAnimatedNumber> _x;
    Ref<SVGAnimatedNumber> _y;
    Ref<SVGAnimatedNumber> _z;
    Ref<SVGAnimatedNumber> _pointsAtX;
    Ref<SVGAnimatedNumber> _pointsAtY;
    Ref<SVGAnimatedNumber> _pointsAtZ;
    Ref<SVGAnimatedNumber> _specularExponent;
    Ref<SVGAnimatedNumber> _limitingConeAngle;

  public:
    KRYS_NODISCARD virtual Ref<LightSource> GetLightSource() const noexcept = 0;
    KRYS_NODISCARD static SVGFELightElement *FindLightElement(const SVGElement *svgElement) noexcept;

    KRYS_NODISCARD float Azimuth() const noexcept
    {
      return _azimuth->CurrentValue();
    }
    KRYS_NODISCARD float Elevation() const noexcept
    {
      return _elevation->CurrentValue();
    }
    KRYS_NODISCARD float X() const noexcept
    {
      return _x->CurrentValue();
    }
    KRYS_NODISCARD float Y() const noexcept
    {
      return _y->CurrentValue();
    }
    KRYS_NODISCARD float Z() const noexcept
    {
      return _z->CurrentValue();
    }
    KRYS_NODISCARD float PointsAtX() const noexcept
    {
      return _pointsAtX->CurrentValue();
    }
    KRYS_NODISCARD float PointsAtY() const noexcept
    {
      return _pointsAtY->CurrentValue();
    }
    KRYS_NODISCARD float PointsAtZ() const noexcept
    {
      return _pointsAtZ->CurrentValue();
    }
    KRYS_NODISCARD float SpecularExponent() const noexcept
    {
      return _specularExponent->CurrentValue();
    }
    KRYS_NODISCARD float LimitingConeAngle() const noexcept
    {
      return _limitingConeAngle->CurrentValue();
    }

    KRYS_NODISCARD SVGAnimatedNumber &AzimuthAnimated() noexcept
    {
      return *_azimuth;
    }
    KRYS_NODISCARD SVGAnimatedNumber &ElevationAnimated() noexcept
    {
      return *_elevation;
    }
    KRYS_NODISCARD SVGAnimatedNumber &XAnimated() noexcept
    {
      return *_x;
    }
    KRYS_NODISCARD SVGAnimatedNumber &YAnimated() noexcept
    {
      return *_y;
    }
    KRYS_NODISCARD SVGAnimatedNumber &ZAnimated() noexcept
    {
      return *_z;
    }
    KRYS_NODISCARD SVGAnimatedNumber &PointsAtXAnimated() noexcept
    {
      return *_pointsAtX;
    }
    KRYS_NODISCARD SVGAnimatedNumber &PointsAtYAnimated() noexcept
    {
      return *_pointsAtY;
    }
    KRYS_NODISCARD SVGAnimatedNumber &PointsAtZAnimated() noexcept
    {
      return *_pointsAtZ;
    }
    KRYS_NODISCARD SVGAnimatedNumber &SpecularExponentAnimated() noexcept
    {
      return *_specularExponent;
    }
    KRYS_NODISCARD SVGAnimatedNumber &LimitingConeAngleAnimated() noexcept
    {
      return *_limitingConeAngle;
    }

    using PropertyRegistry = SVGPropertyOwnerRegistry<SVGFELightElement, SVGElement>;

  protected:
    SVGFELightElement(dom::Document &document, SVGElementInterface interface) noexcept;

    KRYS_NODISCARD bool RendererIsNeeded(const RenderStyle &) noexcept override
    {
      return false;
    }

  private:
    void AttributeChanged(const dom::QualifiedName &name, Maybe<dom::DOMStringView> oldValue,
                          Maybe<dom::DOMStringView> value) noexcept override;
    void SVGAttributeChanged(const dom::QualifiedName &attributeName) noexcept override;
    void OnChildrenChanged() noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::svg::SVGFELightElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::svg::SVGElement &target) noexcept
  {
    return target.IsSVGFEDistantLightElement() || target.IsSVGFEPointLightElement()
           || target.IsSVGFESpotLightElement();
  }
  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Node &node) noexcept
  {
    auto *svgElement = krys::DynamicDowncast<krys::boo::svg::SVGElement>(node);
    return svgElement && IsType(*svgElement);
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
