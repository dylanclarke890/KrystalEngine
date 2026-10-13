#include "Krystal.Booey/CSS/Values/VariableReferenceValue.hpp"

namespace krys::boo::css
{
  VariableReferenceValue::VariableReferenceValue(Ref<VariableData> &&data) noexcept
      : Value(ValueType::VariableReference), _data(krys::move(data))
  {
    CacheSimpleReference();
  }

  Ref<VariableReferenceValue> VariableReferenceValue::Create(const css::TokenRange &range,
                                                             const ParserContext &context) noexcept
  {
    return AdoptRef(*new VariableReferenceValue(VariableData::Create(range, context)));
  }

  Ref<VariableReferenceValue> VariableReferenceValue::Create(Ref<VariableData> &&data) noexcept
  {
    return AdoptRef(*new VariableReferenceValue(krys::move(data)));
  }

  bool VariableReferenceValue::Equals(const VariableReferenceValue &other) const noexcept
  {
    return _data.get() == other._data.get() || *_data == *other._data;
  }

  CSSOMString VariableReferenceValue::CustomCSSText(const SerialisationContext &) const noexcept
  {
    if (_stringValue.empty())
    {
      _stringValue = _data->Serialise();
    }
    return _stringValue;
  }

  const ParserContext &VariableReferenceValue::Context() const noexcept
  {
    return _data->Context();
  }

  void VariableReferenceValue::CacheSimpleReference() noexcept
  {
    krys_debug_assert(!_simpleReference);

    auto range = _data->TokenRange();

    auto functionId = range.Peek().FunctionId();
    if (functionId != css::ValueId::Var && functionId != css::ValueId::Env)
    {
      return;
    }

    auto variableRange = range.ConsumeBlock();
    if (!range.IsAtEnd())
    {
      return;
    }

    variableRange.DiscardWhitespace();

    const Token &nameToken = variableRange.ConsumeIncludingWhitespace();
    if (nameToken.Type() != TokenType::Ident)
    {
      return;
    }

    auto variableName = CSSOMStringAtom(nameToken.IdentCodePoints());

    // No fallback support on this path.
    if (!variableRange.IsAtEnd())
    {
      return;
    }

    _simpleReference = SimpleReference {variableName, functionId};
  }
}
