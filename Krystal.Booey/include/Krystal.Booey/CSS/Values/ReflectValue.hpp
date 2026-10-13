#pragma once

#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  enum class ValueId : uint16;

  /// @brief A `-webkit-box-reflect` value: a direction, an offset and an optional mask.
  class ReflectValue final : public Value
  {
  private:
    css::ValueId _direction;
    const Ref<Value> _offset;
    const RefPtr<Value> _mask;

    ReflectValue(css::ValueId direction, Ref<Value> offset, RefPtr<Value> mask) noexcept;

  public:
    KRYS_NODISCARD static Ref<ReflectValue> Create(css::ValueId direction, Ref<Value> offset,
                                                   RefPtr<Value> mask) noexcept;

    KRYS_NODISCARD css::ValueId Direction() const noexcept
    {
      return _direction;
    }

    KRYS_NODISCARD const Value &Offset() const noexcept
    {
      return *_offset;
    }

    KRYS_NODISCARD const Value *Mask() const noexcept
    {
      return _mask.get();
    }

    KRYS_NODISCARD CSSOMString CustomCSSText(const SerialisationContext &context) const noexcept;
    KRYS_NODISCARD bool Equals(const ReflectValue &other) const noexcept;

    KRYS_NODISCARD IterationStatus
      CustomVisitChildren(KRYS_NOESCAPE const Func<IterationStatus(Value &)> &func) const noexcept
    {
      if (func(*_offset) == IterationStatus::Done)
      {
        return IterationStatus::Done;
      }
      if (_mask)
      {
        if (func(*_mask) == IterationStatus::Done)
        {
          return IterationStatus::Done;
        }
      }
      return IterationStatus::Continue;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::ReflectValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsReflectValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
