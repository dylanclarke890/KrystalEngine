#pragma once

#include "Krystal.Booey/Graphics/Filters/FilterOperations.hpp"
#include "Krystal.Booey/Graphics/GraphicsLayerAnimationValue.hpp"

namespace krys::boo
{
  /// @brief Used to store one filter value in a keyframe list.
  class GraphicsLayerFilterAnimationValue : public GraphicsLayerAnimationValue
  {
  private:
    FilterOperations _value;

  public:
    GraphicsLayerFilterAnimationValue(double keyTime, const FilterOperations &value,
                                      TimingFunction *timingFunction = nullptr) noexcept
        : GraphicsLayerAnimationValue(keyTime, timingFunction), _value(value)
    {
    }

    explicit GraphicsLayerFilterAnimationValue(const GraphicsLayerFilterAnimationValue &other) noexcept
        : GraphicsLayerAnimationValue(other), _value(other._value.Clone())
    {
    }
    GraphicsLayerFilterAnimationValue(GraphicsLayerFilterAnimationValue &&) noexcept = default;

    KRYS_NODISCARD UniquePtr<GraphicsLayerAnimationValue> Clone() const noexcept override
    {
      return CreateUnique<GraphicsLayerFilterAnimationValue>(*this);
    }

    KRYS_NODISCARD const FilterOperations &Value() const noexcept
    {
      return _value;
    }
  };
}
