#pragma once

#include "Krystal.Booey/SVG/Properties/SVGAnimationFunction.hpp"

namespace krys::boo::svg
{
  /// @brief An animation function that interpolates, and can add to the underlying value (`additive="sum"`)
  /// and accumulate over repeats (`accumulate="sum"`).
  class SVGAnimationAdditiveFunction : public SVGAnimationFunction
  {
  protected:
    CalcMode _calcMode;
    bool _isAccumulated;
    bool _isAdditive;

  public:
    SVGAnimationAdditiveFunction(AnimationMode animationMode, CalcMode calcMode, bool isAccumulated,
                                 bool isAdditive) noexcept
        : SVGAnimationFunction(animationMode), _calcMode(calcMode), _isAccumulated(isAccumulated),
          _isAdditive(isAdditive)
    {
    }

    void SetFromAndByValues(SVGElement &targetElement, dom::DOMStringView from,
                            dom::DOMStringView by) noexcept override
    {
      SetFromAndToValues(targetElement, from, by);
      AddFromAndToValues(targetElement);
    }

    void SetToAtEndOfDurationValue(SVGElement &, dom::DOMStringView) noexcept override
    {
      krys_debug_assert_msg(false, "Additive functions set the value at the end of the duration themselves");
    }

  protected:
    KRYS_NODISCARD float Animate(float progress, uint32 repeatCount, float from, float to,
                                 float toAtEndOfDuration, float animated) const noexcept
    {
      float number;
      if (_calcMode == CalcMode::Discrete)
      {
        number = progress < 0.5 ? from : to;
      }
      else
      {
        number = (to - from) * progress + from;
      }

      if (_isAccumulated && repeatCount)
      {
        number += toAtEndOfDuration * static_cast<float>(repeatCount);
      }

      if (_isAdditive && _animationMode != AnimationMode::To)
      {
        number += animated;
      }

      return number;
    }
  };
}
