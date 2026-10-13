#include "Krystal.Booey/CSS/Values/CustomPropertyValue.hpp"
#include "Krystal.Booey/CSS/Values/ValueId.hpp"
#include "Krystal.Core/Utils/NeverDestroyed.hpp"
#include "Krystal.Core/Visitor.hpp"

namespace krys::boo::css
{
  Ref<CustomPropertyValue> CustomPropertyValue::CreateEmpty(const CSSOMStringAtom &name) noexcept
  {
    static NeverDestroyed<Ref<VariableData>> empty {
      VariableData::Create(css::TokenRange(Span<const Token> {}))};
    return CreateSyntaxAll(name, Ref {empty.get()});
  }

  Ref<CustomPropertyValue> CustomPropertyValue::CreateUnresolved(const CSSOMStringAtom &name,
                                                                 Ref<VariableReferenceValue> &&value) noexcept
  {
    return AdoptRef(*new CustomPropertyValue(
      name, VariantValue {std::in_place_type<Ref<VariableReferenceValue>>, krys::move(value)}));
  }

  Ref<CustomPropertyValue> CustomPropertyValue::CreateSyntaxAll(const CSSOMStringAtom &name,
                                                                Ref<VariableData> &&value) noexcept
  {
    return AdoptRef(*new CustomPropertyValue(
      name, VariantValue {std::in_place_type<Ref<VariableData>>, krys::move(value)}));
  }

  Ref<CustomPropertyValue> CustomPropertyValue::CreateWithGlobalKeyword(const CSSOMStringAtom &name,
                                                                        GlobalKeyword keyword) noexcept
  {
    return AdoptRef(*new CustomPropertyValue(name, VariantValue {keyword}));
  }

  bool CustomPropertyValue::IsVariableReference() const noexcept
  {
    return std::holds_alternative<Ref<VariableReferenceValue>>(_value);
  }

  bool CustomPropertyValue::IsVariableData() const noexcept
  {
    return std::holds_alternative<Ref<VariableData>>(_value);
  }

  bool CustomPropertyValue::IsGlobalKeyword() const noexcept
  {
    return std::holds_alternative<GlobalKeyword>(_value);
  }

  Maybe<GlobalKeyword> CustomPropertyValue::TryGlobalKeyword() const noexcept
  {
    if (auto *keyword = std::get_if<GlobalKeyword>(&_value))
    {
      return *keyword;
    }
    return {};
  }

  bool CustomPropertyValue::Equals(const CustomPropertyValue &other) const noexcept
  {
    if (_name != other._name || _value.index() != other._value.index())
    {
      return false;
    }

    return krys::SwitchOn(
      _value,
      [&](const Ref<VariableReferenceValue> &value)
      {
        const auto &otherValue = std::get<Ref<VariableReferenceValue>>(other._value);
        return value.get() == otherValue.get() || value->Equals(*otherValue);
      },
      [&](const Ref<VariableData> &value)
      {
        const auto &otherValue = std::get<Ref<VariableData>>(other._value);
        return value.get() == otherValue.get() || *value == *otherValue;
      },
      [&](const GlobalKeyword &keyword) { return keyword == std::get<GlobalKeyword>(other._value); });
  }

  CSSOMString CustomPropertyValue::CustomCSSText(const SerialisationContext &context) const noexcept
  {
    auto serialise = [&]
    {
      return krys::SwitchOn(
        _value, [&](const Ref<VariableReferenceValue> &value) { return value->CssText(context); },
        [&](const Ref<VariableData> &value) { return value->Serialise(); },
        [&](const GlobalKeyword &value) { return CSSOMString {ToLower(ToValueId(value))}; });
    };

    if (_cachedCSSText.empty())
    {
      _cachedCSSText = serialise();
    }

    return _cachedCSSText;
  }

  const List<Token> &CustomPropertyValue::Tokens() const noexcept
  {
    static NeverDestroyed<List<Token>> emptyTokens;

    // Not a SwitchOn: it returns by value, and this returns a reference.
    if (auto *value = std::get_if<Ref<VariableData>>(&_value))
    {
      return (*value)->Tokens();
    }

    krys_debug_assert(!IsVariableReference());
    return emptyTokens.get();
  }

  Ref<const VariableData> CustomPropertyValue::AsVariableData() const noexcept
  {
    return krys::SwitchOn(
      _value, [&](const Ref<VariableReferenceValue> &value) -> Ref<const VariableData>
      { return ShareRef(value->Data()); }, [&](const Ref<VariableData> &value) -> Ref<const VariableData>
      { return value; }, [&](const GlobalKeyword &) -> Ref<const VariableData>
      { return VariableData::Create(css::TokenRange(Tokens())); });
  }

  bool CustomPropertyValue::IsCurrentColor() const noexcept
  {
    // FIXME: Registered properties?
    auto tokenRange = krys::SwitchOn(
      _value, [&](const Ref<VariableReferenceValue> &variableReferenceValue)
      { return variableReferenceValue->Data().TokenRange(); },
      [&](const Ref<VariableData> &data) { return data->TokenRange(); },
      [&](const GlobalKeyword &) { return css::TokenRange(Span<const Token> {}); });

    if (tokenRange.IsAtEnd())
    {
      return false;
    }

    const Token &token = tokenRange.ConsumeIncludingWhitespace();
    if (!tokenRange.IsAtEnd())
    {
      return false;
    }

    // FIXME: This should probably check all tokens.
    return token.ValueId() == css::ValueId::Currentcolor;
  }

  IterationStatus CustomPropertyValue::CustomVisitChildren(
    KRYS_NOESCAPE const Func<IterationStatus(css::Value &)> &func) const noexcept
  {
    if (auto *value = std::get_if<Ref<VariableReferenceValue>>(&_value))
    {
      if (func(*value->get()) == IterationStatus::Done)
      {
        return IterationStatus::Done;
      }
    }
    return IterationStatus::Continue;
  }
}
