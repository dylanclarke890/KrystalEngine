#pragma once

#include "Krystal.Booey/SVG/SVGTextPositioningElement.hpp"

namespace krys::boo::svg
{
  /// @brief The `<text>` element: a block of SVG text.
  /// @see https://svgwg.org/svg2-draft/text.html#InterfaceSVGTextElement
  class SVGTextElement final : public SVGTextPositioningElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGTextElement);

  public:
    using PropertyRegistry = SVGPropertyOwnerRegistry<SVGTextElement, SVGTextPositioningElement>;

    explicit SVGTextElement(dom::Document &) noexcept;

  private:
    KRYS_NODISCARD RenderPtr<RenderElement> CreateElementRenderer(RenderStyle &&) noexcept final;
    KRYS_NODISCARD bool ChildShouldCreateRenderer(const dom::Node &) const noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::svg::SVGTextElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::svg::SVGElement &target) noexcept
  {
    return target.IsSVGTextElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
