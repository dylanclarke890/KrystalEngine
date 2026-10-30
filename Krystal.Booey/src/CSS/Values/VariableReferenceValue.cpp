#include "Krystal.Booey/CSS/Values/VariableReferenceValue.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyParser.hpp"
#include "Krystal.Booey/CSS/Properties/RegisteredCustomProperty.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Style/Builder.hpp"
#include "Krystal.Booey/Style/CustomProperty.hpp"
#include "Krystal.Booey/Style/CustomPropertyRegistry.hpp"
#include "Krystal.Booey/Style/Scope.hpp"

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

  auto VariableReferenceValue::ResolveVariableFallback(const CSSOMStringAtom &variableName,
                                                       css::TokenRange range, css::ValueId functionId,
                                                       style::Builder &builder) const noexcept
    -> Pair<FallbackResult, List<Token>>
  {
    krys_debug_assert(range.IsAtEnd() || range.Peek().Type() == TokenType::Comma);

    if (range.IsAtEnd())
    {
      return {FallbackResult::None, {}};
    }

    std::ignore = range.ConsumeIncludingWhitespace();

    auto tokens = ResolveTokenRange(range, builder);

    if (functionId == css::ValueId::Var)
    {
      const auto *registered =
        builder.State().GetDocument().StyleScope().GetCustomPropertyRegistry().Get(variableName);
      if (registered && !registered->Syntax.IsUniversal())
      {
        // https://drafts.css-houdini.org/css-properties-values-api/#fallbacks-in-var-references
        // The fallback value must match the syntax definition of the custom property being referenced,
        // otherwise the declaration is invalid at computed-value time
        if (!tokens
            || !PropertyParser::IsValidCustomPropertyValueForSyntax(registered->Syntax,
                                                                    css::TokenRange(*tokens), Context()))
        {
          return {FallbackResult::Invalid, {}};
        }

        return {FallbackResult::Valid, krys::move(*tokens)};
      }
    }

    if (!tokens)
    {
      return {FallbackResult::None, {}};
    }

    return {FallbackResult::Valid, krys::move(*tokens)};
  }

  namespace
  {
    KRYS_NODISCARD const style::CustomProperty *
      PropertyValueForVariableName(const CSSOMStringAtom &variableName, css::ValueId functionId,
                                   style::Builder &builder) noexcept
    {
      if (functionId == css::ValueId::Env)
      {
        // FIXME: The document's constant properties (`ConstantPropertyMap`: the safe area and fullscreen
        // insets) come with the page.
        return nullptr;
      }

      // Apply this variable first, in case it is still unresolved
      builder.ApplyCustomProperty(variableName);

      return builder.State().Style().CustomPropertyValue(variableName);
    }
  }

  bool VariableReferenceValue::ResolveVariableReference(css::TokenRange range, css::ValueId functionId,
                                                        List<Token> &tokens,
                                                        style::Builder &builder) const noexcept
  {
    krys_debug_assert(functionId == css::ValueId::Var || functionId == css::ValueId::Env);

    range.DiscardWhitespace();
    krys_debug_assert(range.Peek().Type() == TokenType::Ident);
    auto variableName = CSSOMStringAtom(range.ConsumeIncludingWhitespace().IdentCodePoints());

    // Fallback has to be resolved even when not used to detect cycles and invalid syntax.
    auto [fallbackResult, fallbackTokens] = ResolveVariableFallback(variableName, range, functionId, builder);
    if (fallbackResult == FallbackResult::Invalid)
    {
      return false;
    }

    RefPtr property = ShareRefPtr(PropertyValueForVariableName(variableName, functionId, builder));

    if (!property || property->IsGuaranteedInvalid())
    {
      if (fallbackTokens.size() > MaxSubstitutionTokens)
      {
        return false;
      }

      if (fallbackResult == FallbackResult::Valid)
      {
        tokens.insert(tokens.end(), fallbackTokens.begin(), fallbackTokens.end());
        return true;
      }
      return false;
    }

    if (property->Tokens().size() > MaxSubstitutionTokens)
    {
      return false;
    }

    const auto &propertyTokens = property->Tokens();
    tokens.insert(tokens.end(), propertyTokens.begin(), propertyTokens.end());
    return true;
  }

  Maybe<List<Token>> VariableReferenceValue::ResolveTokenRange(css::TokenRange range,
                                                               style::Builder &builder) const noexcept
  {
    List<Token> tokens;
    bool success = true;
    while (!range.IsAtEnd())
    {
      auto functionId = range.Peek().FunctionId();
      if (functionId == css::ValueId::Var || functionId == css::ValueId::Env)
      {
        if (!ResolveVariableReference(range.ConsumeBlock(), functionId, tokens, builder))
        {
          success = false;
        }
        continue;
      }
      tokens.push_back(range.Consume());
    }
    if (!success)
    {
      return std::nullopt;
    }

    return tokens;
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

  RefPtr<VariableData>
    VariableReferenceValue::TryResolveSimpleReference(style::Builder &builder) const noexcept
  {
    if (!_simpleReference)
    {
      return nullptr;
    }

    // Shortcut for the simple common case of property:var(--foo)

    RefPtr property = ShareRefPtr(
      PropertyValueForVariableName(_simpleReference->Name, _simpleReference->FunctionId, builder));
    if (!property || !std::holds_alternative<Ref<VariableData>>(property->GetValue()))
    {
      return nullptr;
    }

    return std::get<Ref<VariableData>>(property->GetValue());
  }

  RefPtr<VariableData>
    VariableReferenceValue::ResolveVariableReferences(style::Builder &builder) const noexcept
  {
    if (auto data = TryResolveSimpleReference(builder))
    {
      return data;
    }

    auto resolvedTokens = ResolveTokenRange(_data->TokenRange(), builder);
    if (!resolvedTokens)
    {
      return nullptr;
    }

    return VariableData::Create(css::TokenRange(*resolvedTokens), Context());
  }

  RefPtr<Value> VariableReferenceValue::ResolveSingleValue(style::Builder &builder,
                                                           css::PropertyId propertyId) const noexcept
  {
    if (!ResolveAndCacheValue(builder,
                              [this, propertyId](const auto &data)
                              {
                                _cachedValue = PropertyParser::ParseStylePropertyLonghand(
                                  propertyId, data->TokenRange(), Context());
#if KRYS_DEBUG
                                _cachePropertyId = propertyId;
#endif
                              }))
    {
      return nullptr;
    }

#if KRYS_DEBUG
    krys_debug_assert(_cachePropertyId == propertyId);
#endif
    return _cachedValue;
  }
}
