#include "Krystal.Booey/CSS/TypedOM/CSSStyleValue.hpp"
#include "Krystal.Booey/CSS/Values/SerialisationContext.hpp"
#include "Krystal.Booey/CSS/Values/Value.hpp"

namespace krys::boo::css
{
  Ref<CSSStyleValue> CSSStyleValue::Create(RefPtr<Value> &&cssValue, CSSOMString &&property) noexcept
  {
    return AdoptRef(*new CSSStyleValue(krys::move(cssValue), krys::move(property)));
  }

  Ref<CSSStyleValue> CSSStyleValue::Create() noexcept
  {
    return AdoptRef(*new CSSStyleValue());
  }

  CSSStyleValue::CSSStyleValue(RefPtr<Value> &&cssValue, CSSOMString &&property) noexcept
      : _customPropertyName(krys::move(property)), _propertyValue(krys::move(cssValue))
  {
  }

  CSSStyleValue::CSSStyleValue() noexcept = default;

  CSSStyleValue::~CSSStyleValue() noexcept = default;

  CSSOMString CSSStyleValue::ToString() const noexcept
  {
    StringBuilder builder;
    Serialise(builder);
    return krys::move(builder).ToString();
  }

  void CSSStyleValue::Serialise(StringBuilder &builder, EnumSet<SerialisationArguments>) const noexcept
  {
    if (_propertyValue)
    {
      builder.Append(_propertyValue->CssText(DefaultSerialisationContext()));
    }
  }

  RefPtr<Value> CSSStyleValue::ToCSSValue() const noexcept
  {
    return _propertyValue;
  }
}
