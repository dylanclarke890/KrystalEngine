#pragma once

#include "Krystal.Booey/SVG/SVGElement.hpp"
#include "Krystal.Booey/SVG/SVGFitToViewBox.hpp"
#include "Krystal.Booey/SVG/SVGZoomAndPan.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::svg
{
  class SVGSvgElement;

  /// @brief The `<view>` element: a view of the document's `<svg>` that a fragment identifier can pick.
  /// @see https://svgwg.org/svg2-draft/linking.html#InterfaceSVGViewElement
  class SVGViewElement final : public SVGElement, public SVGFitToViewBox, public SVGZoomAndPan
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGViewElement);

  private:
    WeakPtr<SVGSvgElement> _targetElement;

  public:
    explicit SVGViewElement(dom::Document &document) noexcept;

    using PropertyRegistry = SVGPropertyOwnerRegistry<SVGViewElement, SVGElement, SVGFitToViewBox>;

    KRYS_NODISCARD const SVGSvgElement *TargetElement() const noexcept;
    void SetTargetElement(const SVGSvgElement &targetElement) noexcept;
    void ResetTargetElement() noexcept;

  private:
    void AttributeChanged(const dom::QualifiedName &name, Maybe<dom::DOMStringView> oldValue,
                          Maybe<dom::DOMStringView> value) noexcept final;
    void SVGAttributeChanged(const dom::QualifiedName &attributeName) noexcept override;

    KRYS_NODISCARD bool RendererIsNeeded(const RenderStyle &) noexcept final
    {
      return false;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::svg::SVGViewElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::svg::SVGElement &target) noexcept
  {
    return target.IsSVGViewElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
