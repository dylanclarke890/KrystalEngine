#pragma once

#include "Krystal.Booey/Graphics/GraphicsLayerAnimationValue.hpp"

namespace krys::boo
{
  /// @brief Used to store one float value of an animation.
  class GraphicsLayerFloatAnimationValue : public GraphicsLayerAnimationValue
  {
  private:
    float _value;

  public:
    GraphicsLayerFloatAnimationValue(double keyTime, float value,
                                     TimingFunction *timingFunction = nullptr) noexcept
        : GraphicsLayerAnimationValue(keyTime, timingFunction), _value(value)
    {
    }

    KRYS_NODISCARD UniquePtr<GraphicsLayerAnimationValue> Clone() const noexcept override
    {
      return CreateUnique<GraphicsLayerFloatAnimationValue>(*this);
    }

    KRYS_NODISCARD float Value() const noexcept
    {
      return _value;
    }
  };
}
