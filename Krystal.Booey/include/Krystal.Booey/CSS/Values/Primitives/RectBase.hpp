#pragma once

#include "Krystal.Booey/CSS/Values/CSSValue.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class RectBase
  {
  private:
    const Ref<const CSSValue> _top;
    const Ref<const CSSValue> _right;
    const Ref<const CSSValue> _bottom;
    const Ref<const CSSValue> _left;

  protected:
    explicit RectBase(Ref<CSSValue> value) noexcept
        : _top(value), _right(value), _bottom(value), _left(krys::move(value))
    {
    }

    RectBase(Ref<CSSValue> top, Ref<CSSValue> right, Ref<CSSValue> bottom, Ref<CSSValue> left) noexcept
        : _top(krys::move(top)), _right(krys::move(right)), _bottom(krys::move(bottom)),
          _left(krys::move(left))
    {
    }

    ~RectBase() noexcept = default;

  public:
    KRYS_NODISCARD const CSSValue &Top() const noexcept
    {
      return *_top;
    }
    KRYS_NODISCARD const CSSValue &Right() const noexcept
    {
      return *_right;
    }
    KRYS_NODISCARD const CSSValue &Bottom() const noexcept
    {
      return *_bottom;
    }
    KRYS_NODISCARD const CSSValue &Left() const noexcept
    {
      return *_left;
    }

    KRYS_NODISCARD bool Equals(const RectBase &other) const noexcept
    {
      return *_top == *other._top && *_right == *other._right && *_left == *other._left
             && *_bottom == *other._bottom;
    }
  };
}