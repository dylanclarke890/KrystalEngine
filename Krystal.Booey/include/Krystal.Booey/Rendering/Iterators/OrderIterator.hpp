#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/List.hpp"

namespace krys::boo
{
  class RenderBox;
  class RenderObject;

  /// @brief Iterates over a flex or grid container's child boxes in `order`-modified document order (or its
  /// reverse).
  class OrderIterator
  {
    friend class OrderIteratorPopulator;

  private:
    RenderBox *_containerBox;
    RenderBox *_currentChild {nullptr};

    // The distinct order values of the children, ascending.
    List<int32> _orderValues;
    // The position in _orderValues, counted from the end when the order is reversed.
    size_t _orderValuesIndex {0};
    bool _isReset {false};
    bool _reversedOrder {false};

  public:
    explicit OrderIterator(RenderBox &containerBox) noexcept;

    KRYS_NODISCARD RenderBox *CurrentChild() const noexcept
    {
      return _currentChild;
    }
    RenderBox *First() noexcept;
    RenderBox *Next() noexcept;
    KRYS_NODISCARD OrderIterator Reverse() const noexcept;
    KRYS_NODISCARD bool ShouldSkipChild(const RenderObject &child) const noexcept;

  private:
    void Reset() noexcept;
    KRYS_NODISCARD int32 CurrentOrderValue() const noexcept;
  };

  /// @brief Collects the order values of a container's children into an @ref OrderIterator, and resets the
  /// iterator when it goes out of scope.
  class OrderIteratorPopulator
  {
  private:
    OrderIterator &_iterator;

  public:
    explicit OrderIteratorPopulator(OrderIterator &iterator) noexcept;
    ~OrderIteratorPopulator() noexcept;

    /// @return Whether @p child takes part in the container's layout (it isn't skipped).
    bool CollectChild(const RenderBox &child) noexcept;
  };
}
