#pragma once

#include "Krystal.Booey/Graphics/GraphicsLayerAnimationValue.hpp"
#include "Krystal.Booey/Graphics/Transforms/TransformOperations.hpp"

namespace krys::boo
{
  /// @brief Used to store one transform value in a keyframe list.
  class GraphicsLayerTransformAnimationValue : public GraphicsLayerAnimationValue
  {
  private:
    TransformOperations _value;

  public:
    GraphicsLayerTransformAnimationValue(double keyTime, const TransformOperations &value,
                                         TimingFunction *timingFunction = nullptr) noexcept
        : GraphicsLayerAnimationValue(keyTime, timingFunction), _value(value)
    {
    }

    GraphicsLayerTransformAnimationValue(double keyTime, TransformOperation *value,
                                         TimingFunction *timingFunction = nullptr) noexcept
        : GraphicsLayerAnimationValue(keyTime, timingFunction),
          _value(value ? TransformOperations {ShareRef(*value)} : TransformOperations {})
    {
    }

    explicit GraphicsLayerTransformAnimationValue(const GraphicsLayerTransformAnimationValue &other) noexcept
        : GraphicsLayerAnimationValue(other), _value(other._value.Clone())
    {
    }
    GraphicsLayerTransformAnimationValue(GraphicsLayerTransformAnimationValue &&) noexcept = default;

    KRYS_NODISCARD UniquePtr<GraphicsLayerAnimationValue> Clone() const noexcept override
    {
      return CreateUnique<GraphicsLayerTransformAnimationValue>(*this);
    }

    KRYS_NODISCARD const TransformOperations &Value() const noexcept
    {
      return _value;
    }
  };
}
