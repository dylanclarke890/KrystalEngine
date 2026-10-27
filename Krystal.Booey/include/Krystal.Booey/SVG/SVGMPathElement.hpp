#pragma once

#include "Krystal.Booey/SVG/SVGElement.hpp"
#include "Krystal.Booey/SVG/SVGPathElement.hpp"
#include "Krystal.Booey/SVG/SVGURIReference.hpp"

namespace krys::boo::svg
{
  /// @brief The `<mpath>` element: references the `<path>` its parent `<animateMotion>` moves along.
  /// @see https://svgwg.org/specs/animations/#MPathElement
  class SVGMPathElement final : public SVGElement, public SVGURIReference
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGMPathElement);

  public:
    explicit SVGMPathElement(dom::Document &) noexcept;
    ~SVGMPathElement() noexcept override;

    KRYS_NODISCARD RefPtr<SVGPathElement> PathElement() noexcept;

    void TargetPathChanged() noexcept;

    using PropertyRegistry = SVGPropertyOwnerRegistry<SVGMPathElement, SVGElement, SVGURIReference>;

  private:
    void AttributeChanged(const dom::QualifiedName &, Maybe<dom::DOMStringView> oldValue,
                          Maybe<dom::DOMStringView> value) noexcept final;
    void SVGAttributeChanged(const dom::QualifiedName &) noexcept final;

    void BuildPendingResource() noexcept final;
    void ClearResourceReferences() noexcept;
    void OnPostConnection() noexcept final;
    void OnRemove(bool isSubtreeRoot, dom::ContainerNode &oldAncestor) noexcept final;

    KRYS_NODISCARD bool RendererIsNeeded(const RenderStyle &) noexcept final
    {
      return false;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::svg::SVGMPathElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::svg::SVGElement &element) noexcept
  {
    return element.Interface() == krys::boo::svg::SVGElementInterface::MPath;
  }
  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Node &node) noexcept
  {
    auto *svgElement = krys::DynamicDowncast<krys::boo::svg::SVGElement>(node);
    return svgElement && IsType(*svgElement);
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
