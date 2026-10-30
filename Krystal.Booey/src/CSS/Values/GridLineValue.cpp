#include "Krystal.Booey/CSS/Values/GridLineValue.hpp"

namespace krys::boo::css
{
  CSSOMString GridLineValue::CustomCSSText(const SerialisationContext &context) const noexcept
  {
    CSSOMString result;
    auto append = [&](const PrimitiveValue &value)
    {
      if (!result.empty())
      {
        result.push_back(u8' ');
      }
      result.append(value.CssText(context));
    };

    if (_spanValue)
    {
      append(*_spanValue);
    }
    // Only return the numeric value if not 1, or if it provided without a span value.
    // https://drafts.csswg.org/css-grid-2/#grid-placement-span-int
    if (_numericValue)
    {
      if (_numericValue->IsOne() != true || !_spanValue || !_gridLineName)
      {
        append(*_numericValue);
      }
    }
    if (_gridLineName)
    {
      append(*_gridLineName);
    }
    return result;
  }

  GridLineValue::GridLineValue(RefPtr<PrimitiveValue> spanValue, RefPtr<PrimitiveValue> numericValue,
                               RefPtr<PrimitiveValue> gridLineName) noexcept
      : Value(ValueType::GridLine), _spanValue(krys::move(spanValue)),
        _numericValue(krys::move(numericValue)), _gridLineName(krys::move(gridLineName))
  {
  }

  Ref<GridLineValue> GridLineValue::Create(RefPtr<PrimitiveValue> spanValue,
                                           RefPtr<PrimitiveValue> numericValue,
                                           RefPtr<PrimitiveValue> gridLineName) noexcept
  {
    return AdoptRef(
      *new GridLineValue(krys::move(spanValue), krys::move(numericValue), krys::move(gridLineName)));
  }

  bool GridLineValue::Equals(const GridLineValue &other) const noexcept
  {
    auto equals = [](PrimitiveValue *value, PrimitiveValue *otherValue)
    {
      if ((!value && otherValue) || (value && !otherValue))
      {
        return false;
      }
      return (!value && !otherValue) || value->Equals(*otherValue);
    };

    if (!equals(ProtectedSpanValue().get(), other.ProtectedSpanValue().get()))
    {
      return false;
    }
    if (!equals(ProtectedNumericValue().get(), other.ProtectedNumericValue().get()))
    {
      return false;
    }
    if (!equals(ProtectedGridLineName().get(), other.ProtectedGridLineName().get()))
    {
      return false;
    }

    return true;
  }
}
