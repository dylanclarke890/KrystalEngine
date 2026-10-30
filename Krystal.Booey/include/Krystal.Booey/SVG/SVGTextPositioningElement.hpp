#pragma once

#include "Krystal.Booey/SVG/SVGLengthList.hpp"
#include "Krystal.Booey/SVG/SVGNumberList.hpp"
#include "Krystal.Booey/SVG/SVGTextContentElement.hpp"

namespace krys::boo
{
  class RenderBoxModelObject;
}

namespace krys::boo::svg
{
  /// @brief The base of `<text>` and `<tspan>`: their characters' absolute and relative positions and
  /// rotations.
  /// @see https://svgwg.org/svg2-draft/text.html#InterfaceSVGTextPositioningElement
  class SVGTextPositioningElement : public SVGTextContentElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGTextPositioningElement);

  private:
    Ref<SVGAnimatedLengthList> _x;
    Ref<SVGAnimatedLengthList> _y;
    Ref<SVGAnimatedLengthList> _dx;
    Ref<SVGAnimatedLengthList> _dy;
    Ref<SVGAnimatedNumberList> _rotate;

  public:
    KRYS_NODISCARD static RefPtr<SVGTextPositioningElement>
      ElementFromRenderer(RenderBoxModelObject &) noexcept;

    using PropertyRegistry = SVGPropertyOwnerRegistry<SVGTextPositioningElement, SVGTextContentElement>;

    KRYS_NODISCARD const SVGLengthList &X() const noexcept
    {
      return _x->CurrentValue();
    }
    KRYS_NODISCARD const SVGLengthList &Y() const noexcept
    {
      return _y->CurrentValue();
    }
    KRYS_NODISCARD const SVGLengthList &Dx() const noexcept
    {
      return _dx->CurrentValue();
    }
    KRYS_NODISCARD const SVGLengthList &Dy() const noexcept
    {
      return _dy->CurrentValue();
    }
    KRYS_NODISCARD const SVGNumberList &Rotate() const noexcept
    {
      return _rotate->CurrentValue();
    }

    KRYS_NODISCARD SVGAnimatedLengthList &XAnimated() noexcept
    {
      return *_x;
    }
    KRYS_NODISCARD SVGAnimatedLengthList &YAnimated() noexcept
    {
      return *_y;
    }
    KRYS_NODISCARD SVGAnimatedLengthList &DxAnimated() noexcept
    {
      return *_dx;
    }
    KRYS_NODISCARD SVGAnimatedLengthList &DyAnimated() noexcept
    {
      return *_dy;
    }
    KRYS_NODISCARD SVGAnimatedNumberList &RotateAnimated() noexcept
    {
      return *_rotate;
    }

  protected:
    SVGTextPositioningElement(dom::Document &, UniquePtr<SVGPropertyRegistry> &&,
                              SVGElementInterface) noexcept;

    void AttributeChanged(const dom::QualifiedName &name, Maybe<dom::DOMStringView> oldValue,
                          Maybe<dom::DOMStringView> value) noexcept override;
    void SVGAttributeChanged(const dom::QualifiedName &attributeName) noexcept override;

  private:
    KRYS_NODISCARD bool
      HasPresentationalHintsForAttribute(dom::DOMStringAtom localName,
                                         dom::DOMStringAtom namespaceURI) const noexcept final;
    void CollectPresentationalHintsForAttribute(dom::DOMStringAtom localName, dom::DOMStringAtom namespaceURI,
                                                dom::DOMStringView value,
                                                css::MutableStyleProperties &style) noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::svg::SVGTextPositioningElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::svg::SVGElement &target) noexcept
  {
    return target.IsSVGTextPositioningElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
