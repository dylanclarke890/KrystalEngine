#pragma once

#include "Krystal.Booey/SVG/Properties/SVGAnimationFunction.hpp"

namespace krys::boo::svg
{
  /// @brief An animation function that jumps from its `from` value to its `to` value.
  template <typename TValue>
  class SVGAnimationDiscreteFunction : public SVGAnimationFunction
  {
  protected:
    TValue _from {};
    TValue _to {};

  public:
    SVGAnimationDiscreteFunction(AnimationMode animationMode, CalcMode, bool, bool) noexcept
        : SVGAnimationFunction(animationMode)
    {
    }

    KRYS_NODISCARD bool IsDiscrete() const noexcept override
    {
      return true;
    }

    void SetToAtEndOfDurationValue(SVGElement &, dom::DOMStringView) noexcept override
    {
      krys_debug_assert_msg(false, "Discrete functions don't accumulate");
    }

    void SetFromAndByValues(SVGElement &, dom::DOMStringView, dom::DOMStringView) noexcept override
    {
      krys_debug_assert_msg(false, "Discrete functions don't take `by` values");
    }

    void Animate(SVGElement &, float progress, uint32, TValue &animated) noexcept
    {
      if (((_animationMode == AnimationMode::FromTo || _animationMode == AnimationMode::To) && progress > 0.5)
          || progress == 1)
      {
        animated = _to;
      }
      else
      {
        animated = _from;
      }
    }
  };
}
