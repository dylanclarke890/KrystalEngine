#pragma once

#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  /// @brief The value of a `scroll()` function, as used in `animation-timeline`.
  /// @note https://drafts.csswg.org/scroll-animations-1/#funcdef-scroll
  class ScrollValue final : public Value
  {
  private:
    RefPtr<Value> _scroller;
    RefPtr<Value> _axis;

    ScrollValue(RefPtr<Value> &&scroller, RefPtr<Value> &&axis) noexcept;

  public:
    KRYS_NODISCARD static Ref<ScrollValue> Create() noexcept;
    KRYS_NODISCARD static Ref<ScrollValue> Create(RefPtr<Value> &&scroller, RefPtr<Value> &&axis) noexcept;

    KRYS_NODISCARD CSSOMString CustomCSSText(const SerialisationContext &context) const noexcept;

    KRYS_NODISCARD const RefPtr<Value> &Scroller() const noexcept
    {
      return _scroller;
    }

    KRYS_NODISCARD const RefPtr<Value> &Axis() const noexcept
    {
      return _axis;
    }

    KRYS_NODISCARD bool Equals(const ScrollValue &other) const noexcept;

    KRYS_NODISCARD IterationStatus
      CustomVisitChildren(KRYS_NOESCAPE const Func<IterationStatus(Value &)> &func) const noexcept
    {
      if (_scroller)
      {
        if (func(*_scroller) == IterationStatus::Done)
        {
          return IterationStatus::Done;
        }
      }
      if (_axis)
      {
        if (func(*_axis) == IterationStatus::Done)
        {
          return IterationStatus::Done;
        }
      }
      return IterationStatus::Continue;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::ScrollValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsScrollValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
