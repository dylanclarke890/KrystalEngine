#include "Krystal.Booey/Style/Values/Lists/ListStyleType.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/Style/BuilderChecking.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo::style
{
  bool ListStyleType::IsCircle() const noexcept
  {
    return _type == Type::CounterStyle
           && _identifier == css::CSSOMStringAtom {css::ToString(css::ValueId::Circle)};
  }

  bool ListStyleType::IsDecimal() const noexcept
  {
    return _type == Type::CounterStyle
           && _identifier == css::CSSOMStringAtom {css::ToString(css::ValueId::Decimal)};
  }

  bool ListStyleType::IsDisc() const noexcept
  {
    return _type == Type::CounterStyle
           && _identifier == css::CSSOMStringAtom {css::ToString(css::ValueId::Disc)};
  }

  bool ListStyleType::IsSquare() const noexcept
  {
    return _type == Type::CounterStyle
           && _identifier == css::CSSOMStringAtom {css::ToString(css::ValueId::Square)};
  }

  // MARK: - Conversion

  ListStyleType CSSValueConversion<ListStyleType>::operator()(BuilderState &state,
                                                              const css::Value &value) noexcept
  {
    auto *primitiveValue = RequiredDowncast<css::PrimitiveValue>(state, value);
    if (!primitiveValue)
    {
      return css::keywords::None {};
    }

    if (primitiveValue->IsValueId())
    {
      if (primitiveValue->ValueId() == css::ValueId::None)
      {
        return css::keywords::None {};
      }
      return CounterStyle {{css::CSSOMStringAtom {primitiveValue->StringValue()}}};
    }
    if (primitiveValue->IsCustomIdent())
    {
      return CounterStyle {{css::CSSOMStringAtom {primitiveValue->StringValue()}}};
    }

    return css::CSSOMStringAtom {primitiveValue->StringValue()};
  }

  Ref<css::Value> CSSValueCreation<ListStyleType>::operator()(css::ValuePool &pool, const RenderStyle &style,
                                                              const ListStyleType &value) noexcept
  {
    return krys::SwitchOn(
      value,
      [&](const css::keywords::None &none) { return style::CreateCSSValue(pool, style, none); }, // none
      [&](const CounterStyle &counterStyle)
      { return style::CreateCSSValue(pool, style, counterStyle.Identifier); }, // custom-ident
      [&](const css::CSSOMStringAtom &identifier)
      { return style::CreateCSSValue(pool, style, identifier); }); // string
  }
}
