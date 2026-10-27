#pragma once

#include "Krystal.Booey/SVG/SVGGeometryElement.hpp"
#include "Krystal.Booey/SVG/SVGPointList.hpp"

namespace krys::boo::svg
{
  /// @brief The base of `<polygon>` and `<polyline>`: a shape through a list of points.
  class SVGPolyElement : public SVGGeometryElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGPolyElement);

  private:
    Ref<SVGAnimatedPointList> _points;

  public:
    KRYS_NODISCARD const SVGPointList &Points() const noexcept
    {
      return _points->CurrentValue();
    }
    /// @see https://svgwg.org/svg2-draft/shapes.html#__svg__SVGAnimatedPoints__points
    KRYS_NODISCARD SVGPointList &Points() noexcept
    {
      return *_points->BaseVal();
    }
    /// @see https://svgwg.org/svg2-draft/shapes.html#__svg__SVGAnimatedPoints__animatedPoints
    KRYS_NODISCARD SVGPointList &AnimatedPoints() noexcept
    {
      return *_points->AnimVal();
    }

    using PropertyRegistry = SVGPropertyOwnerRegistry<SVGPolyElement, SVGGeometryElement>;

  protected:
    SVGPolyElement(dom::Document &document, SVGElementInterface interface) noexcept;

  private:
    void AttributeChanged(const dom::QualifiedName &name, Maybe<dom::DOMStringView> oldValue,
                          Maybe<dom::DOMStringView> value) noexcept override;
    void SVGAttributeChanged(const dom::QualifiedName &attributeName) noexcept override;

    KRYS_NODISCARD bool IsValid() const noexcept override
    {
      return SVGTests::IsValid();
    }
    KRYS_NODISCARD bool SupportsMarkers() const noexcept override
    {
      return true;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::svg::SVGPolyElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::svg::SVGElement &target) noexcept
  {
    return target.IsSVGPolygonElement() || target.IsSVGPolylineElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
