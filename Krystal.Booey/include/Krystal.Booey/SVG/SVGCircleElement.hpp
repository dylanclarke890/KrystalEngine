#pragma once

#include "Krystal.Booey/SVG/SVGGeometryElement.hpp"

namespace krys::boo::svg
{
  /// @brief The `<circle>` element: a circle about a centre.
  /// @see https://svgwg.org/svg2-draft/shapes.html#InterfaceSVGCircleElement
  class SVGCircleElement final : public SVGGeometryElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGCircleElement);

  private:
    Ref<SVGAnimatedLength> _cx;
    Ref<SVGAnimatedLength> _cy;
    Ref<SVGAnimatedLength> _r;

  public:
    explicit SVGCircleElement(dom::Document &document) noexcept;

    KRYS_NODISCARD const SVGLengthValue &Cx() const noexcept
    {
      return _cx->CurrentValue();
    }
    KRYS_NODISCARD const SVGLengthValue &Cy() const noexcept
    {
      return _cy->CurrentValue();
    }
    KRYS_NODISCARD const SVGLengthValue &R() const noexcept
    {
      return _r->CurrentValue();
    }

    KRYS_NODISCARD SVGAnimatedLength &CxAnimated() noexcept
    {
      return *_cx;
    }
    KRYS_NODISCARD SVGAnimatedLength &CyAnimated() noexcept
    {
      return *_cy;
    }
    KRYS_NODISCARD SVGAnimatedLength &RAnimated() noexcept
    {
      return *_r;
    }

    using PropertyRegistry = SVGPropertyOwnerRegistry<SVGCircleElement, SVGGeometryElement>;

    KRYS_NODISCARD SVGAnimatedProperty *PropertyForAttribute(const dom::QualifiedName &) const noexcept;

  private:
    void AttributeChanged(const dom::QualifiedName &name, Maybe<dom::DOMStringView> oldValue,
                          Maybe<dom::DOMStringView> value) noexcept final;
    void SVGAttributeChanged(const dom::QualifiedName &attributeName) noexcept final;

    KRYS_NODISCARD bool IsValid() const noexcept final
    {
      return SVGTests::IsValid();
    }
    KRYS_NODISCARD bool SelfHasRelativeLengths() const noexcept final
    {
      return true;
    }

    KRYS_NODISCARD RenderPtr<RenderElement> CreateElementRenderer(RenderStyle &&) noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::svg::SVGCircleElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::svg::SVGElement &target) noexcept
  {
    return target.IsSVGCircleElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
