#pragma once

#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  /// @brief Four values for the top, right, bottom and left sides; shared by `Quad` and `Rect`.
  class RectBase
  {
  private:
    const Ref<const Value> _top;
    const Ref<const Value> _right;
    const Ref<const Value> _bottom;
    const Ref<const Value> _left;

  protected:
    explicit RectBase(Ref<Value> value) noexcept
        : _top(value), _right(value), _bottom(value), _left(krys::move(value))
    {
    }

    RectBase(Ref<Value> top, Ref<Value> right, Ref<Value> bottom, Ref<Value> left) noexcept
        : _top(krys::move(top)), _right(krys::move(right)), _bottom(krys::move(bottom)),
          _left(krys::move(left))
    {
    }

    ~RectBase() noexcept = default;

  public:
    KRYS_NODISCARD const Value &Top() const noexcept
    {
      return *_top;
    }

    KRYS_NODISCARD const Value &Right() const noexcept
    {
      return *_right;
    }

    KRYS_NODISCARD const Value &Bottom() const noexcept
    {
      return *_bottom;
    }

    KRYS_NODISCARD const Value &Left() const noexcept
    {
      return *_left;
    }

    KRYS_NODISCARD bool Equals(const RectBase &other) const noexcept
    {
      return CompareValue(_top, other._top) && CompareValue(_right, other._right)
             && CompareValue(_left, other._left) && CompareValue(_bottom, other._bottom);
    }
  };
}
