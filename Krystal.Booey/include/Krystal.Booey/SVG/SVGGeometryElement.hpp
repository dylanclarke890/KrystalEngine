#pragma once

#include "Krystal.Booey/DOM/Dicts/DOMPointInit.hpp"
#include "Krystal.Booey/SVG/SVGGraphicsElement.hpp"

namespace krys::boo::svg
{
  class SVGPoint;

  /// @brief A shape element: its geometry is a path that can be measured and hit tested.
  /// @see https://svgwg.org/svg2-draft/types.html#InterfaceSVGGeometryElement
  class SVGGeometryElement : public SVGGraphicsElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGGeometryElement);

  private:
    Ref<SVGAnimatedNumber> _pathLength;

  public:
    /// @see https://svgwg.org/svg2-draft/types.html#__svg__SVGGeometryElement__getTotalLength
    KRYS_NODISCARD virtual float GetTotalLength() const noexcept;
    /// @see https://svgwg.org/svg2-draft/types.html#__svg__SVGGeometryElement__getPointAtLength
    KRYS_NODISCARD virtual dom::ExceptionOr<Ref<SVGPoint>> GetPointAtLength(float distance) const noexcept;

    /// @see https://svgwg.org/svg2-draft/types.html#__svg__SVGGeometryElement__isPointInFill
    KRYS_NODISCARD bool IsPointInFill(dom::DOMPointInit &&) noexcept;
    /// @see https://svgwg.org/svg2-draft/types.html#__svg__SVGGeometryElement__isPointInStroke
    KRYS_NODISCARD bool IsPointInStroke(dom::DOMPointInit &&) noexcept;

    using PropertyRegistry = SVGPropertyOwnerRegistry<SVGGeometryElement, SVGGraphicsElement>;

    KRYS_NODISCARD float PathLength() const noexcept
    {
      return _pathLength->CurrentValue();
    }
    /// @see https://svgwg.org/svg2-draft/types.html#__svg__SVGGeometryElement__pathLength
    KRYS_NODISCARD SVGAnimatedNumber &PathLengthAnimated() noexcept
    {
      return *_pathLength;
    }

  protected:
    SVGGeometryElement(dom::Document &document, UniquePtr<SVGPropertyRegistry> &&propertyRegistry,
                       SVGElementInterface interface) noexcept;

    void AttributeChanged(const dom::QualifiedName &name, Maybe<dom::DOMStringView> oldValue,
                          Maybe<dom::DOMStringView> value) noexcept override;
    void SVGAttributeChanged(const dom::QualifiedName &attributeName) noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::svg::SVGGeometryElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::svg::SVGElement &target) noexcept
  {
    return target.IsSVGGeometryElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
