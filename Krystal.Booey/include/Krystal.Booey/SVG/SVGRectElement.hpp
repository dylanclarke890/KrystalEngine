#pragma once

#include "Krystal.Booey/SVG/SVGGeometryElement.hpp"

namespace krys::boo::svg
{
  /// @brief The `<rect>` element: a rectangle, with optionally rounded corners.
  /// @see https://svgwg.org/svg2-draft/shapes.html#InterfaceSVGRectElement
  class SVGRectElement final : public SVGGeometryElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGRectElement);

  private:
    Ref<SVGAnimatedLength> _x;
    Ref<SVGAnimatedLength> _y;
    Ref<SVGAnimatedLength> _width;
    Ref<SVGAnimatedLength> _height;
    Ref<SVGAnimatedLength> _rx;
    Ref<SVGAnimatedLength> _ry;

  public:
    explicit SVGRectElement(dom::Document &document) noexcept;

    KRYS_NODISCARD const SVGLengthValue &X() const noexcept
    {
      return _x->CurrentValue();
    }
    KRYS_NODISCARD const SVGLengthValue &Y() const noexcept
    {
      return _y->CurrentValue();
    }
    KRYS_NODISCARD const SVGLengthValue &Width() const noexcept
    {
      return _width->CurrentValue();
    }
    KRYS_NODISCARD const SVGLengthValue &Height() const noexcept
    {
      return _height->CurrentValue();
    }
    KRYS_NODISCARD const SVGLengthValue &Rx() const noexcept
    {
      return _rx->CurrentValue();
    }
    KRYS_NODISCARD const SVGLengthValue &Ry() const noexcept
    {
      return _ry->CurrentValue();
    }

    KRYS_NODISCARD SVGAnimatedLength &XAnimated() noexcept
    {
      return *_x;
    }
    KRYS_NODISCARD SVGAnimatedLength &YAnimated() noexcept
    {
      return *_y;
    }
    KRYS_NODISCARD SVGAnimatedLength &WidthAnimated() noexcept
    {
      return *_width;
    }
    KRYS_NODISCARD SVGAnimatedLength &HeightAnimated() noexcept
    {
      return *_height;
    }
    KRYS_NODISCARD SVGAnimatedLength &RxAnimated() noexcept
    {
      return *_rx;
    }
    KRYS_NODISCARD SVGAnimatedLength &RyAnimated() noexcept
    {
      return *_ry;
    }

    using PropertyRegistry = SVGPropertyOwnerRegistry<SVGRectElement, SVGGeometryElement>;

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

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::svg::SVGRectElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::svg::SVGElement &target) noexcept
  {
    return target.IsSVGRectElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
