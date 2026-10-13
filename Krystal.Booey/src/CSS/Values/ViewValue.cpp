#include "Krystal.Booey/CSS/Values/ViewValue.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/ValueId.hpp"

namespace krys::boo::css
{
  ViewValue::ViewValue(RefPtr<Value> &&axis, RefPtr<Value> &&startInset, RefPtr<Value> &&endInset) noexcept
      : Value(ValueType::View), _axis(krys::move(axis)), _startInset(krys::move(startInset)),
        _endInset(krys::move(endInset))
  {
  }

  Ref<ViewValue> ViewValue::Create() noexcept
  {
    return AdoptRef(*new ViewValue(nullptr, nullptr, nullptr));
  }

  Ref<ViewValue> ViewValue::Create(RefPtr<Value> &&axis, RefPtr<Value> &&startInset,
                                   RefPtr<Value> &&endInset) noexcept
  {
    return AdoptRef(*new ViewValue(krys::move(axis), krys::move(startInset), krys::move(endInset)));
  }

  CSSOMString ViewValue::CustomCSSText(const SerialisationContext &context) const noexcept
  {
    bool hasAxis = _axis && _axis->ValueId() != ValueId::Block;
    bool hasEndInset = _endInset && _endInset != _startInset;
    bool hasStartInset = (_startInset && _startInset->ValueId() != ValueId::Auto)
                         || (_startInset && _startInset->ValueId() == ValueId::Auto && hasEndInset);

    CSSOMString result {u8"view("};
    if (hasAxis)
    {
      result.append(_axis->CssText(context));
    }
    if (hasAxis && hasStartInset)
    {
      result.push_back(u8' ');
    }
    if (hasStartInset)
    {
      result.append(_startInset->CssText(context));
    }
    if (hasStartInset && hasEndInset)
    {
      result.push_back(u8' ');
    }
    if (hasEndInset)
    {
      result.append(_endInset->CssText(context));
    }
    result.push_back(u8')');
    return result;
  }

  bool ViewValue::Equals(const ViewValue &other) const noexcept
  {
    return CompareValuePtr(_axis, other._axis) && CompareValuePtr(_startInset, other._startInset)
           && CompareValuePtr(_endInset, other._endInset);
  }
}
