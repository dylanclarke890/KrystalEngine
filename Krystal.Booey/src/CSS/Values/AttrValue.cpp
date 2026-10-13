#include "Krystal.Booey/CSS/Values/AttrValue.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"

namespace krys::boo::css
{
  Ref<AttrValue> AttrValue::Create(CSSOMString attributeName, RefPtr<Value> fallback) noexcept
  {
    return AdoptRef(*new AttrValue(krys::move(attributeName), krys::move(fallback)));
  }

  bool AttrValue::Equals(const AttrValue &other) const noexcept
  {
    auto *fallback = DynamicDowncast<PrimitiveValue>(_fallback.get());
    auto *otherFallback = DynamicDowncast<PrimitiveValue>(other._fallback.get());

    if (fallback && otherFallback)
    {
      return _attributeName == other._attributeName
             && fallback->StringValue() == otherFallback->StringValue();
    }
    if (fallback || otherFallback)
    {
      return false;
    }
    return _attributeName == other._attributeName;
  }

  CSSOMString AttrValue::CustomCSSText(const SerialisationContext &context) const noexcept
  {
    auto *fallback = DynamicDowncast<PrimitiveValue>(_fallback.get());

    CSSOMString result {u8"attr("};
    result.append(_attributeName);
    if (fallback && !fallback->StringValue().empty())
    {
      result.append(u8", ");
      result.append(fallback->CssText(context));
    }
    result.push_back(u8')');
    return result;
  }
}
