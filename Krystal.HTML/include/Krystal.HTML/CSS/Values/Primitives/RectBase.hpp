#pragma once

#include "Krystal.HTML/CSS/Values/CSSValue.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"

namespace Krys::HTML
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
        : _top(value), _right(value), _bottom(value), _left(Krys::Move(value))
    {
    }

    RectBase(Ref<CSSValue> top, Ref<CSSValue> right, Ref<CSSValue> bottom, Ref<CSSValue> left) noexcept
        : _top(Krys::Move(top)), _right(Krys::Move(right)), _bottom(Krys::Move(bottom)),
          _left(Krys::Move(left))
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