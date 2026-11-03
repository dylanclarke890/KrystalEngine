#pragma once

#include "Krystal.Booey/SVG/SVGAnimateElementBase.hpp"
#include "Krystal.Booey/SVG/SVGTransformValue.hpp"

namespace krys::boo::svg
{
  /// @brief The `<animateTransform>` element: animates one kind of transform on its target's transform list.
  /// @see https://svgwg.org/specs/animations/#AnimateTransformElement
  class SVGAnimateTransformElement final : public SVGAnimateElementBase
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGAnimateTransformElement);

  private:
    SVGTransformValue::SVGTransformType _type {SVGTransformValue::SVG_TRANSFORM_TRANSLATE};

  public:
    explicit SVGAnimateTransformElement(dom::Document &) noexcept;

    KRYS_NODISCARD SVGTransformValue::SVGTransformType TransformType() const noexcept
    {
      return _type;
    }

  private:
    KRYS_NODISCARD bool HasValidAttributeType() const noexcept final;
    void AttributeChanged(const dom::QualifiedName &, Maybe<dom::DOMStringView> oldValue,
                          Maybe<dom::DOMStringView> value) noexcept final;
    KRYS_NODISCARD dom::DOMString AnimateRangeString(dom::DOMStringView) const noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::svg::SVGAnimateTransformElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::svg::SVGElement &element) noexcept
  {
    return element.Interface() == krys::boo::svg::SVGElementInterface::AnimateTransform;
  }
  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Node &node) noexcept
  {
    auto *svgElement = krys::DynamicDowncast<krys::boo::svg::SVGElement>(node);
    return svgElement && IsType(*svgElement);
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
