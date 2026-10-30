#pragma once

#include "Krystal.Booey/SVG/SVGGeometryElement.hpp"

namespace krys::boo::svg
{
  /// @brief The `<line>` element: a line between two points.
  /// @see https://svgwg.org/svg2-draft/shapes.html#InterfaceSVGLineElement
  class SVGLineElement final : public SVGGeometryElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGLineElement);

  private:
    Ref<SVGAnimatedLength> _x1;
    Ref<SVGAnimatedLength> _y1;
    Ref<SVGAnimatedLength> _x2;
    Ref<SVGAnimatedLength> _y2;

  public:
    explicit SVGLineElement(dom::Document &document) noexcept;

    KRYS_NODISCARD const SVGLengthValue &X1() const noexcept
    {
      return _x1->CurrentValue();
    }
    KRYS_NODISCARD const SVGLengthValue &Y1() const noexcept
    {
      return _y1->CurrentValue();
    }
    KRYS_NODISCARD const SVGLengthValue &X2() const noexcept
    {
      return _x2->CurrentValue();
    }
    KRYS_NODISCARD const SVGLengthValue &Y2() const noexcept
    {
      return _y2->CurrentValue();
    }

    KRYS_NODISCARD SVGAnimatedLength &X1Animated() noexcept
    {
      return *_x1;
    }
    KRYS_NODISCARD SVGAnimatedLength &Y1Animated() noexcept
    {
      return *_y1;
    }
    KRYS_NODISCARD SVGAnimatedLength &X2Animated() noexcept
    {
      return *_x2;
    }
    KRYS_NODISCARD SVGAnimatedLength &Y2Animated() noexcept
    {
      return *_y2;
    }

  private:
    using PropertyRegistry = SVGPropertyOwnerRegistry<SVGLineElement, SVGGeometryElement>;

    void AttributeChanged(const dom::QualifiedName &name, Maybe<dom::DOMStringView> oldValue,
                          Maybe<dom::DOMStringView> value) noexcept final;
    void SVGAttributeChanged(const dom::QualifiedName &attributeName) noexcept final;

    KRYS_NODISCARD bool IsValid() const noexcept final
    {
      return SVGTests::IsValid();
    }
    KRYS_NODISCARD bool SupportsMarkers() const noexcept final
    {
      return true;
    }
    KRYS_NODISCARD bool SelfHasRelativeLengths() const noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::svg::SVGLineElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::svg::SVGElement &target) noexcept
  {
    return target.IsSVGLineElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
