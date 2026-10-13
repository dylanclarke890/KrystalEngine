#pragma once

#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  /// @brief The value of a `view()` function, as used in `animation-timeline`.
  /// @note https://drafts.csswg.org/scroll-animations-1/#funcdef-view
  class ViewValue final : public Value
  {
  private:
    RefPtr<Value> _axis;
    RefPtr<Value> _startInset;
    RefPtr<Value> _endInset;

    ViewValue(RefPtr<Value> &&axis, RefPtr<Value> &&startInset, RefPtr<Value> &&endInset) noexcept;

  public:
    KRYS_NODISCARD static Ref<ViewValue> Create() noexcept;
    KRYS_NODISCARD static Ref<ViewValue> Create(RefPtr<Value> &&axis, RefPtr<Value> &&startInset,
                                                RefPtr<Value> &&endInset) noexcept;

    KRYS_NODISCARD CSSOMString CustomCSSText(const SerialisationContext &context) const noexcept;

    KRYS_NODISCARD const RefPtr<Value> &Axis() const noexcept
    {
      return _axis;
    }

    KRYS_NODISCARD const RefPtr<Value> &StartInset() const noexcept
    {
      return _startInset;
    }

    KRYS_NODISCARD const RefPtr<Value> &EndInset() const noexcept
    {
      return _endInset;
    }

    KRYS_NODISCARD bool Equals(const ViewValue &other) const noexcept;

    KRYS_NODISCARD IterationStatus
      CustomVisitChildren(KRYS_NOESCAPE const Func<IterationStatus(Value &)> &func) const noexcept
    {
      if (_axis)
      {
        if (func(*_axis) == IterationStatus::Done)
        {
          return IterationStatus::Done;
        }
      }
      if (_startInset)
      {
        if (func(*_startInset) == IterationStatus::Done)
        {
          return IterationStatus::Done;
        }
      }
      if (_endInset)
      {
        if (func(*_endInset) == IterationStatus::Done)
        {
          return IterationStatus::Done;
        }
      }
      return IterationStatus::Continue;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::ViewValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsViewValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
