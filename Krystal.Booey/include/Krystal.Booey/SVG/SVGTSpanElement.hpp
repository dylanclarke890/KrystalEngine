#pragma once

#include "Krystal.Booey/SVG/SVGTextPositioningElement.hpp"

namespace krys::boo::svg
{
  /// @brief The `<tspan>` element: a span of SVG text with its own positions and style.
  /// @see https://svgwg.org/svg2-draft/text.html#InterfaceSVGTSpanElement
  class SVGTSpanElement final : public SVGTextPositioningElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGTSpanElement);

  public:
    using PropertyRegistry = SVGPropertyOwnerRegistry<SVGTSpanElement, SVGTextPositioningElement>;

    explicit SVGTSpanElement(dom::Document &) noexcept;

  private:
    KRYS_NODISCARD RenderPtr<RenderElement> CreateElementRenderer(RenderStyle &&) noexcept final;
    KRYS_NODISCARD bool ChildShouldCreateRenderer(const dom::Node &) const noexcept final;
    KRYS_NODISCARD bool RendererIsNeeded(const RenderStyle &) noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::svg::SVGTSpanElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::svg::SVGElement &target) noexcept
  {
    return target.IsSVGTSpanElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
