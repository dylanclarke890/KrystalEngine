#include "Krystal.Booey/CSS/Values/ScrollValue.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/ValueId.hpp"

namespace krys::boo::css
{
  ScrollValue::ScrollValue(RefPtr<Value> &&scroller, RefPtr<Value> &&axis) noexcept
      : Value(ValueType::Scroll), _scroller(krys::move(scroller)), _axis(krys::move(axis))
  {
  }

  Ref<ScrollValue> ScrollValue::Create() noexcept
  {
    return AdoptRef(*new ScrollValue(nullptr, nullptr));
  }

  Ref<ScrollValue> ScrollValue::Create(RefPtr<Value> &&scroller, RefPtr<Value> &&axis) noexcept
  {
    return AdoptRef(*new ScrollValue(krys::move(scroller), krys::move(axis)));
  }

  CSSOMString ScrollValue::CustomCSSText(const SerialisationContext &context) const noexcept
  {
    bool hasScroller = _scroller && _scroller->ValueId() != ValueId::Nearest;
    bool hasAxis = _axis && _axis->ValueId() != ValueId::Block;

    CSSOMString result {u8"scroll("};
    if (hasScroller)
    {
      result.append(_scroller->CssText(context));
    }
    if (hasScroller && hasAxis)
    {
      result.push_back(u8' ');
    }
    if (hasAxis)
    {
      result.append(_axis->CssText(context));
    }
    result.push_back(u8')');
    return result;
  }

  bool ScrollValue::Equals(const ScrollValue &other) const noexcept
  {
    return CompareValuePtr(_scroller, other._scroller) && CompareValuePtr(_axis, other._axis);
  }
}
