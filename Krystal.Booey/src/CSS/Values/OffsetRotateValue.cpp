#include "Krystal.Booey/CSS/Values/OffsetRotateValue.hpp"
#include "Krystal.Booey/CSS/Values/ValueId.hpp"
#include "Krystal.Core/Assert.hpp"

namespace krys::boo::css
{
  namespace
  {
    // FIXME: Use Downcast<PrimitiveValue>(RefPtr&&) once Krystal.Core's IntrusivePtr overloads are fixed:
    // they can't be called with an explicit target type, and they assert that the pointer is non-null (WTF's
    // allow null).
    KRYS_NODISCARD RefPtr<PrimitiveValue> DowncastToPrimitive(RefPtr<Value> &&value) noexcept
    {
      if (!value)
      {
        return nullptr;
      }
      return RefPtr<PrimitiveValue>::NoRef(Downcast<PrimitiveValue>(value.release()));
    }
  }

  OffsetRotateValue::OffsetRotateValue(RefPtr<PrimitiveValue> &&modifier,
                                       RefPtr<PrimitiveValue> &&angle) noexcept
      : Value(ValueType::OffsetRotate), _modifier(krys::move(modifier)), _angle(krys::move(angle))
  {
    krys_debug_assert(_modifier || _angle);

    if (_modifier)
    {
      krys_debug_assert(_modifier->IsValueId());
    }

    if (_angle)
    {
      krys_debug_assert(_angle->IsAngle());
    }
  }

  Ref<OffsetRotateValue> OffsetRotateValue::Create(RefPtr<Value> modifier, RefPtr<Value> angle) noexcept
  {
    return AdoptRef(*new OffsetRotateValue(DowncastToPrimitive(krys::move(modifier)),
                                           DowncastToPrimitive(krys::move(angle))));
  }

  CSSOMString OffsetRotateValue::CustomCSSText(const SerialisationContext &context) const noexcept
  {
    CSSOMString result;

    if (_modifier)
    {
      result.append(_modifier->CssText(context));
    }

    if (_angle)
    {
      if (!result.empty())
      {
        result.push_back(u8' ');
      }

      result.append(_angle->CssText(context));
    }

    return result;
  }

  bool OffsetRotateValue::IsInitialValue() const noexcept
  {
    return _modifier && _modifier->ValueId() == css::ValueId::Auto
           && (!_angle || _angle->IsZero().value_or(false));
  }

  bool OffsetRotateValue::Equals(const OffsetRotateValue &other) const noexcept
  {
    return CompareValuePtr(_modifier, other._modifier) && CompareValuePtr(_angle, other._angle);
  }
}
