#pragma once

#include "Krystal.Booey/Graphics/GraphicsLayerAnimationValue.hpp"
#include "Krystal.Booey/Graphics/GraphicsLayerClient.hpp"
#include "Krystal.Core/Types/List.hpp"

namespace krys::boo
{
  /// @brief Used to store a series of values in a keyframe list. Values will all be of the same type, which
  /// can be inferred from the property.
  class GraphicsLayerKeyframeValueList
  {
  protected:
    List<UniquePtr<const GraphicsLayerAnimationValue>> _values;
    AnimatedProperty _property;

  public:
    explicit GraphicsLayerKeyframeValueList(AnimatedProperty property) noexcept : _property(property)
    {
    }

    explicit GraphicsLayerKeyframeValueList(const GraphicsLayerKeyframeValueList &other) noexcept
        : _property(other.Property())
    {
      _values.reserve(other._values.size());
      for (const auto &value : other._values)
      {
        _values.push_back(value->Clone());
      }
    }
    GraphicsLayerKeyframeValueList(GraphicsLayerKeyframeValueList &&) noexcept = default;

    GraphicsLayerKeyframeValueList &operator=(const GraphicsLayerKeyframeValueList &other) noexcept
    {
      GraphicsLayerKeyframeValueList copy(other);
      Swap(copy);
      return *this;
    }
    GraphicsLayerKeyframeValueList &operator=(GraphicsLayerKeyframeValueList &&) noexcept = default;

    void Swap(GraphicsLayerKeyframeValueList &other) noexcept
    {
      std::swap(_property, other._property);
      _values.swap(other._values);
    }

    KRYS_NODISCARD AnimatedProperty Property() const noexcept
    {
      return _property;
    }

    KRYS_NODISCARD size_t Size() const noexcept
    {
      return _values.size();
    }
    KRYS_NODISCARD const GraphicsLayerAnimationValue &At(size_t i) const noexcept
    {
      return *_values.at(i);
    }

    // Insert, sorted by keyTime.
    void Insert(UniquePtr<const GraphicsLayerAnimationValue> value) noexcept;
  };
}
