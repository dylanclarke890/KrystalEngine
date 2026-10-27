#pragma once

#include "Krystal.Booey/SVG/Properties/SVGAnimationAdditiveFunction.hpp"

namespace krys::boo::svg
{
  /// @brief An additive animation function over a value type.
  template <typename TValue>
  class SVGAnimationAdditiveValueFunction : public SVGAnimationAdditiveFunction
  {
  public:
    using SVGAnimationAdditiveFunction::SVGAnimationAdditiveFunction;

  protected:
    TValue _from {};
    TValue _to {};
    Maybe<TValue> _toAtEndOfDuration;

    KRYS_NODISCARD TValue ToAtEndOfDuration() const noexcept
    {
      return _toAtEndOfDuration ? *_toAtEndOfDuration : _to;
    }
  };
}
