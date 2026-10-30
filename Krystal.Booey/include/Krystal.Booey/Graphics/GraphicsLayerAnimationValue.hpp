#pragma once

#include "Krystal.Booey/Graphics/Animation/TimingFunction.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo
{
  /// @brief Base class for animation values (also used for transitions). Here to represent values for
  /// properties being animated via the GraphicsLayer, without pulling in style-related data from outside of
  /// the platform directory.
  class GraphicsLayerAnimationValue
  {
  private:
    double _keyTime;
    const RefPtr<TimingFunction> _timingFunction;

  public:
    virtual ~GraphicsLayerAnimationValue() noexcept = default;

    KRYS_NODISCARD double KeyTime() const noexcept
    {
      return _keyTime;
    }
    KRYS_NODISCARD const TimingFunction *GetTimingFunction() const noexcept
    {
      return _timingFunction.get();
    }

    KRYS_NODISCARD virtual UniquePtr<GraphicsLayerAnimationValue> Clone() const noexcept = 0;

    void operator=(const GraphicsLayerAnimationValue &) = delete;

  protected:
    GraphicsLayerAnimationValue(double keyTime, TimingFunction *timingFunction = nullptr) noexcept
        : _keyTime(keyTime), _timingFunction(ShareRefPtr(timingFunction))
    {
    }

    explicit GraphicsLayerAnimationValue(const GraphicsLayerAnimationValue &other) noexcept
        : _keyTime(other._keyTime),
          _timingFunction(other._timingFunction ? RefPtr<TimingFunction> {other._timingFunction->Clone()}
                                                : nullptr)
    {
    }
    GraphicsLayerAnimationValue(GraphicsLayerAnimationValue &&) noexcept = default;
  };
}
