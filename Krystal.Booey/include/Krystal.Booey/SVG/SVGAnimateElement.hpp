#pragma once

#include "Krystal.Booey/SVG/SVGAnimateElementBase.hpp"

namespace krys::boo::svg
{
  /// @brief The `<animate>` element: animates an attribute or property over time.
  /// @see https://svgwg.org/specs/animations/#AnimateElement
  class SVGAnimateElement final : public SVGAnimateElementBase
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGAnimateElement);

  public:
    explicit SVGAnimateElement(dom::Document &document) noexcept
        : SVGAnimateElementBase(document, SVGElementInterface::Animate)
    {
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::svg::SVGAnimateElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::svg::SVGElement &element) noexcept
  {
    return element.Interface() == krys::boo::svg::SVGElementInterface::Animate;
  }
  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Node &node) noexcept
  {
    auto *svgElement = krys::DynamicDowncast<krys::boo::svg::SVGElement>(node);
    return svgElement && IsType(*svgElement);
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
