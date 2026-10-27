#pragma once

#include "Krystal.Booey/SVG/SVGFitToViewBox.hpp"
#include "Krystal.Booey/SVG/SVGGraphicsElement.hpp"

namespace krys::boo::svg
{
  /// @brief The `<symbol>` element: graphics drawn only where `<use>` instantiates them.
  /// @see https://svgwg.org/svg2-draft/struct.html#InterfaceSVGSymbolElement
  class SVGSymbolElement final : public SVGGraphicsElement, public SVGFitToViewBox
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGSymbolElement);

  public:
    explicit SVGSymbolElement(dom::Document &document) noexcept;

    using PropertyRegistry = SVGPropertyOwnerRegistry<SVGSymbolElement, SVGGraphicsElement, SVGFitToViewBox>;

  private:
    void AttributeChanged(const dom::QualifiedName &name, Maybe<dom::DOMStringView> oldValue,
                          Maybe<dom::DOMStringView> value) noexcept override;
    KRYS_NODISCARD RenderPtr<RenderElement> CreateElementRenderer(RenderStyle &&) noexcept override;
    KRYS_NODISCARD bool SelfHasRelativeLengths() const noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::svg::SVGSymbolElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::svg::SVGElement &target) noexcept
  {
    return target.IsSVGSymbolElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
