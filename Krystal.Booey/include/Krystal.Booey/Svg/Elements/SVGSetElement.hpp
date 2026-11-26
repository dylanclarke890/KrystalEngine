#pragma once

#include "Krystal.Booey/Svg/SVGAnimateElementBase.hpp"

namespace krys::boo
{
  /// @brief The `<set>` element: sets an attribute or property to a value for its interval.
  /// @note `SVGAnimateElement` implements a superset of the functionality.
  /// @see https://svgwg.org/specs/animations/#SetElement
  class SVGSetElement final : public SVGAnimateElementBase
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGSetElement);

  public:
    explicit SVGSetElement(Document &document) noexcept
        : SVGAnimateElementBase(document, SVGElementInterface::Set)
    {
      SetAnimationMode(svg::AnimationMode::To);
    }

  private:
    void UpdateAnimationMode() noexcept override
    {
      // No-op, as <set> has a constant animation mode of AnimationMode::To.
      // See: http://www.w3.org/TR/SVG/single-page.html#animate-SetElement
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::SVGSetElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::SVGElement &element) noexcept
  {
    return element.Interface() == krys::boo::SVGElementInterface::Set;
  }
  KRYS_NODISCARD static bool IsType(const krys::boo::Node &node) noexcept
  {
    auto *svgElement = krys::DynamicDowncast<krys::boo::SVGElement>(node);
    return svgElement && IsType(*svgElement);
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
