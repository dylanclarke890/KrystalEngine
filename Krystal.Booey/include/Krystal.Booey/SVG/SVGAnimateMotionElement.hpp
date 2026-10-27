#pragma once

#include "Krystal.Booey/Graphics/FloatPoint.hpp"
#include "Krystal.Booey/Graphics/Path.hpp"
#include "Krystal.Booey/SVG/SVGAnimationElement.hpp"

namespace krys::boo
{
  class AffineTransform;
}

namespace krys::boo::svg
{
  /// @brief The `<animateMotion>` element: moves its target along a motion path.
  /// @see https://svgwg.org/specs/animations/#AnimateMotionElement
  class SVGAnimateMotionElement final : public SVGAnimationElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGAnimateMotionElement);

    enum RotateMode : uint8
    {
      RotateAngle,
      RotateAuto,
      RotateAutoReverse
    };

  private:
    // Note: we do not support percentage values for to/from coords as the spec implies we should (opera
    // doesn't either)
    FloatPoint _fromPoint;
    FloatPoint _toPoint;
    Maybe<FloatPoint> _toPointAtEndOfDuration;

    Path _path;
    Path _animationPath;

  public:
    explicit SVGAnimateMotionElement(dom::Document &) noexcept;

    void UpdateAnimationPath() noexcept;

  private:
    KRYS_NODISCARD bool HasValidAttributeType() const noexcept override;
    KRYS_NODISCARD bool HasValidAttributeName() const noexcept override;

    void AttributeChanged(const dom::QualifiedName &, Maybe<dom::DOMStringView> oldValue,
                          Maybe<dom::DOMStringView> value) noexcept override;

    void StartAnimation() noexcept override;
    void StopAnimation(SVGElement *targetElement) noexcept override;
    bool SetFromAndToValues(dom::DOMStringView fromString, dom::DOMStringView toString) noexcept override;
    bool SetFromAndByValues(dom::DOMStringView fromString, dom::DOMStringView byString) noexcept override;
    bool SetToAtEndOfDurationValue(dom::DOMStringView toAtEndOfDurationString) noexcept override;
    void CalculateAnimatedValue(float percentage, uint32 repeatCount) noexcept override;
    void ApplyResultsToTarget() noexcept override;
    KRYS_NODISCARD Maybe<float> CalculateDistance(dom::DOMStringView fromString,
                                                  dom::DOMStringView toString) noexcept override;

    KRYS_NODISCARD RotateMode GetRotateMode() const noexcept;
    void BuildTransformForProgress(AffineTransform *, float percentage) const noexcept;

    void UpdateAnimationMode() noexcept override;
    void OnChildrenChanged() noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::svg::SVGAnimateMotionElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::svg::SVGElement &element) noexcept
  {
    return element.Interface() == krys::boo::svg::SVGElementInterface::AnimateMotion;
  }
  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Node &node) noexcept
  {
    auto *svgElement = krys::DynamicDowncast<krys::boo::svg::SVGElement>(node);
    return svgElement && IsType(*svgElement);
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
