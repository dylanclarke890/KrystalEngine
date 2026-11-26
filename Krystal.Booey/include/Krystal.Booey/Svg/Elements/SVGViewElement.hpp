#pragma once

#include "Krystal.Booey/Svg/Elements/SVGElement.hpp"
#include "Krystal.Booey/Svg/SVGFitToViewBox.hpp"
#include "Krystal.Core/Memory/WeakPtr.hpp"

namespace krys::boo
{
  class SVGSvgElement;

  /// @brief The `<view>` element: a view of the document's `<svg>` that a fragment identifier can pick.
  /// @see https://svgwg.org/svg2-draft/linking.html#InterfaceSVGViewElement
  class SVGViewElement final : public SVGElement, public SVGFitToViewBox
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGViewElement);

  private:
    WeakPtr<SVGSvgElement> _targetElement;

  public:
    explicit SVGViewElement(Document &document) noexcept;

    using PropertyRegistry = SVGPropertyOwnerRegistry<SVGViewElement, SVGElement, SVGFitToViewBox>;

    KRYS_NODISCARD const SVGSvgElement *TargetElement() const noexcept;

    void SetTargetElement(const SVGSvgElement &targetElement) noexcept;

    void ResetTargetElement() noexcept;

  private:
    void AttributeChanged(const QualifiedName &name, Maybe<DOMStringView> oldValue,
                          Maybe<DOMStringView> value) noexcept final;

    void SVGAttributeChanged(const QualifiedName &attributeName) noexcept override;

    KRYS_NODISCARD bool RendererIsNeeded(const RenderStyle &) noexcept final
    {
      return false;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::SVGViewElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::SVGElement &target) noexcept
  {
    return target.IsSVGViewElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
