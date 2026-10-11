#include "Krystal.Booey/CSS/Values/AttrValue.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/Types.hpp"

namespace krys::boo::css
{
  Ref<AttrValue> AttrValue::Create(CSSOMString attr, RefPtr<Value> fallback) noexcept
  {
    return AdoptRef(*new AttrValue(krys::move(attr), krys::move(fallback)));
  }

  bool AttrValue::Equals(const AttrValue &other) const noexcept
  {
    auto *fallback = DynamicDowncast<PrimitiveValue>(_fallback.get());
    auto *otherFallback = DynamicDowncast<PrimitiveValue>(other._fallback.get());

    if (fallback && otherFallback)
    {
      return _attr == other._attr && fallback->StringValue() == otherFallback->StringValue();
    }

    if (fallback || otherFallback)
    {
      return false;
    }

    return _attr == other._attr;
  }

  CSSOMString AttrValue::CustomCSSText(const SerialisationContext &context) const noexcept
  {
    auto *fallback = DynamicDowncast<PrimitiveValue>(_fallback.get());

    CSSOMString result = u8"attr(" + _attr;
    if (fallback && !fallback->StringValue().empty())
    {
      result += u8", " + fallback->CssText(context);
    }
    result += u8")";

    return result;
  }
}