#pragma once

#include "Krystal.Booey/SVG/SVGGradientElement.hpp"

namespace krys::boo::svg
{
  struct LinearGradientAttributes;

  /// @brief The `<linearGradient>` element: a gradient along a line.
  /// @see https://svgwg.org/svg2-draft/pservers.html#InterfaceSVGLinearGradientElement
  class SVGLinearGradientElement final : public SVGGradientElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGLinearGradientElement);

  private:
    Ref<SVGAnimatedLength> _x1;
    Ref<SVGAnimatedLength> _y1;
    Ref<SVGAnimatedLength> _x2;
    Ref<SVGAnimatedLength> _y2;

  public:
    explicit SVGLinearGradientElement(dom::Document &document) noexcept;

    bool CollectGradientAttributes(LinearGradientAttributes &) noexcept;

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

    using PropertyRegistry = SVGPropertyOwnerRegistry<SVGLinearGradientElement, SVGGradientElement>;

  private:
    void AttributeChanged(const dom::QualifiedName &name, Maybe<dom::DOMStringView> oldValue,
                          Maybe<dom::DOMStringView> value) noexcept final;
    void SVGAttributeChanged(const dom::QualifiedName &attributeName) noexcept final;

    KRYS_NODISCARD RenderPtr<RenderElement> CreateElementRenderer(RenderStyle &&) noexcept final;

    KRYS_NODISCARD bool SelfHasRelativeLengths() const noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::svg::SVGLinearGradientElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::svg::SVGElement &target) noexcept
  {
    return target.IsSVGLinearGradientElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
