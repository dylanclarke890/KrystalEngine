#include "Krystal.Booey/CSS/Values/CounterValue.hpp"
#include "Krystal.Booey/CSS/Markup.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/ValueId.hpp"
#include "Krystal.Core/Assert.hpp"

namespace krys::boo::css
{
  CounterValue::CounterValue(CSSOMStringAtom &&identifier, CSSOMStringAtom &&separator,
                             Ref<Value> &&counterStyle) noexcept
      : Value(ValueType::Counter), _identifier(krys::move(identifier)), _separator(krys::move(separator)),
        _counterStyle(krys::move(counterStyle))
  {
  }

  Ref<CounterValue> CounterValue::Create(CSSOMStringAtom identifier, CSSOMStringAtom separator,
                                         Ref<Value> counterStyle) noexcept
  {
    return AdoptRef(
      *new CounterValue(krys::move(identifier), krys::move(separator), krys::move(counterStyle)));
  }

  bool CounterValue::Equals(const CounterValue &other) const noexcept
  {
    return _identifier == other._identifier && _separator == other._separator
           && CompareValue(_counterStyle, other._counterStyle);
  }

  CSSOMString CounterValue::CustomCSSText(const SerialisationContext &) const noexcept
  {
    bool isDecimal = _counterStyle->ValueId() == css::ValueId::Decimal
                     || (_counterStyle->IsCustomIdent() && _counterStyle->CustomIdent() == u8"decimal");
    CSSOMStringView styleSeparator = isDecimal ? u8"" : u8", ";
    CSSOMString styleLiteral = isDecimal ? CSSOMString() : CounterStyleCSSText();

    CSSOMString result;
    if (_separator == CSSOMStringAtom::Null() || _separator.View().empty())
    {
      result.append(u8"counter(");
      result.append(_identifier.View());
    }
    else
    {
      result.append(u8"counters(");
      result.append(_identifier.View());
      result.append(u8", ");
      SerialiseString(CSSOMString(_separator.View()), result);
    }
    result.append(styleSeparator);
    result.append(styleLiteral);
    result.push_back(u8')');
    return result;
  }

  CSSOMString CounterValue::CounterStyleCSSText() const noexcept
  {
    if (_counterStyle->IsValueId())
    {
      return CSSOMString(ToString(_counterStyle->ValueId()));
    }
    if (_counterStyle->IsCustomIdent())
    {
      return _counterStyle->CustomIdent();
    }

    krys_debug_assert_msg(false, "A counter style is a keyword or a custom ident.");
    return CSSOMString();
  }
}
