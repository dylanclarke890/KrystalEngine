#pragma once

#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  /// @brief An `offset-rotate` value: `auto`/`reverse`, an angle, or both, e.g. `auto 45deg`.
  class OffsetRotateValue final : public Value
  {
  private:
    RefPtr<PrimitiveValue> _modifier;
    RefPtr<PrimitiveValue> _angle;

    OffsetRotateValue(RefPtr<PrimitiveValue> &&modifier, RefPtr<PrimitiveValue> &&angle) noexcept;

  public:
    /// @param modifier A keyword primitive (`auto` or `reverse`), or null.
    /// @param angle An angle primitive, or null. At least one of the two must be set.
    KRYS_NODISCARD static Ref<OffsetRotateValue> Create(RefPtr<Value> modifier, RefPtr<Value> angle) noexcept;

    KRYS_NODISCARD CSSOMString CustomCSSText(const SerialisationContext &context) const noexcept;

    KRYS_NODISCARD PrimitiveValue *Modifier() const noexcept
    {
      return _modifier.get();
    }

    KRYS_NODISCARD PrimitiveValue *GetAngle() const noexcept
    {
      return _angle.get();
    }

    /// @brief Whether this is `auto` (optionally with a zero angle), the initial value.
    KRYS_NODISCARD bool IsInitialValue() const noexcept;

    KRYS_NODISCARD bool Equals(const OffsetRotateValue &other) const noexcept;

    KRYS_NODISCARD IterationStatus
      CustomVisitChildren(KRYS_NOESCAPE const Func<IterationStatus(Value &)> &func) const noexcept
    {
      if (_modifier)
      {
        if (func(*_modifier) == IterationStatus::Done)
        {
          return IterationStatus::Done;
        }
      }
      if (_angle)
      {
        if (func(*_angle) == IterationStatus::Done)
        {
          return IterationStatus::Done;
        }
      }
      return IterationStatus::Continue;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::OffsetRotateValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsOffsetRotateValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
