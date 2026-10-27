#pragma once

#include "Krystal.Booey/SVG/Properties/SVGAttributeAnimator.hpp"

namespace krys::boo::svg
{
  class SVGElement;

  /// @brief Computes an animated value from an animation's `from`, `to` and `by` values and its progress.
  class SVGAnimationFunction
  {
  protected:
    AnimationMode _animationMode;

  public:
    virtual ~SVGAnimationFunction() noexcept = default;

    KRYS_NODISCARD virtual bool IsDiscrete() const noexcept
    {
      return false;
    }

    virtual void SetFromAndToValues(SVGElement &targetElement, dom::DOMStringView,
                                    dom::DOMStringView) noexcept = 0;
    virtual void SetFromAndByValues(SVGElement &targetElement, dom::DOMStringView,
                                    dom::DOMStringView) noexcept = 0;
    virtual void SetToAtEndOfDurationValue(SVGElement &targetElement, dom::DOMStringView) noexcept = 0;

    KRYS_NODISCARD virtual Maybe<float> CalculateDistance(SVGElement &, dom::DOMStringView,
                                                          dom::DOMStringView) const noexcept
    {
      return {};
    }

  protected:
    explicit SVGAnimationFunction(AnimationMode animationMode) noexcept : _animationMode(animationMode)
    {
    }

    virtual void AddFromAndToValues(SVGElement &) noexcept
    {
    }
  };
}
