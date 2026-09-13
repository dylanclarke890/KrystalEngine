#include "Krystal.Booey/CSS/Properties/Consumers/CounterStyles.hpp"
#include "Krystal.Booey/CSS/Parser/TokenRange.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/CSSPrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Ident.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/IntegerDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/List.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyParserState.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyParsing.hpp"
#include "Krystal.Booey/CSS/Values/CSSPrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/CSSValueList.hpp"
#include "Krystal.Booey/CSS/Values/CSSValuePair.hpp"

namespace krys::boo::css::PropertyParserHelpers
{
  KRYS_NODISCARD static bool IsPredefinedCounterStyle(ValueId valueId) noexcept
  {
    // https://drafts.csswg.org/css-counter-styles-3/#predefined-counters

    return valueId >= ValueId::Disc && valueId <= ValueId::EthiopicNumeric;
  }

  RefPtr<CSSValue> ConsumeCounterStyle(TokenRange &tokens, PropertyParserState &state) noexcept
  {
    // <counter-style> = <counter-style-name excluding=none> | <symbols()>
    // https://drafts.csswg.org/css-counter-styles-3/#typedef-counter-style

    // FIXME: Implement support for `symbols()`.

    if (tokens.Peek().ValueId() == ValueId::None)
    {
      return nullptr;
    }

    if (auto predefinedValues = ConsumeIdent(tokens, IsPredefinedCounterStyle))
    {
      return predefinedValues;
    }

    return ConsumeCustomIdent(tokens);
  }

  CSSOMStringAtom ConsumeCounterStyleNameInPrelude(TokenRange &prelude, ParserMode mode) noexcept
  {
    // In the context of the prelude of an @counter-style rule, a <counter-style-name> must not be an
    // ASCII case-insensitive match for "decimal", "disc", "square", "circle", "disclosure-open" and
    // "disclosure-closed". No <counter-style-name>, prelude or not, may be an ASCII case-insensitive
    // match for "none".
    // https://drafts.csswg.org/css-counter-styles-3/#typedef-counter-style-name

    auto &nameToken = prelude.ConsumeIncludingWhitespace();
    if (!prelude.IsAtEnd())
    {
      return CSSOMStringAtom::Null();
    }

    // Ensure this token is a valid <custom-ident>.
    if (nameToken.Type() != TokenType::Ident || !IsValidCustomIdentifier(nameToken.ValueId()))
    {
      return CSSOMStringAtom::Null();
    }

    auto id = nameToken.ValueId();
    if (IdentMatches<ValueId::None>(id)
        || (!IsUASheetBehavior(mode)
            && IdentMatches<ValueId::Decimal, ValueId::Disc, ValueId::Circle, ValueId::Square,
                            ValueId::DisclosureOpen, ValueId::DisclosureClosed>(id)))
    {
      return CSSOMStringAtom::Null();
    }

    auto name = nameToken.IdentCodePoints();
    return IsPredefinedCounterStyle(nameToken.ValueId()) ? krys::text::ToASCIILower(name) : name;
  }

  RefPtr<CSSValue> ConsumeCounterStyleName(TokenRange &tokens, PropertyParserState &) noexcept
  {
    // <counter-style-name> is a <custom-ident> that is not an ASCII case-insensitive match for "none".
    // https://drafts.csswg.org/css-counter-styles-3/#typedef-counter-style-name

    auto valueId = tokens.Peek().ValueId();
    if (valueId == ValueId::None)
    {
      return nullptr;
    }

    // If the value is an ASCII case-insensitive match for any of the predefined counter styles, lowercase it.
    if (auto name = ConsumeCustomIdent(tokens, IsPredefinedCounterStyle(valueId)))
    {
      return name;
    }

    return nullptr;
  }

  RefPtr<CSSValue> ConsumeCounterStyleSystem(TokenRange &tokens, PropertyParserState &state) noexcept
  {
    // <'system'> = cyclic | numeric | alphabetic | symbolic | additive | [fixed <integer>?] | [ extends
    // <counter-style-name> ] https://drafts.csswg.org/css-counter-styles-3/#counter-style-system

    if (auto ident = ConsumeIdent<ValueId::Cyclic, ValueId::Numeric, ValueId::Alphabetic,
                                  ValueId::Symbolic, ValueId::Additive>(tokens))
    {
      return ident;
    }

    if (IsUASheetBehavior(state.Context.Mode))
    {
      auto internalKeyword =
        ConsumeIdent<ValueId::InternalDisclosureClosed, ValueId::InternalDisclosureOpen,
                     ValueId::InternalSimplifiedChineseInformal,
                     ValueId::InternalSimplifiedChineseFormal,
                     ValueId::InternalTraditionalChineseInformal,
                     ValueId::InternalTraditionalChineseFormal, ValueId::InternalEthiopicNumeric>(
          tokens);
      if (internalKeyword)
      {
        return internalKeyword;
      }
    }

    if (auto ident = ConsumeIdent<ValueId::Fixed>(tokens))
    {
      if (tokens.IsAtEnd())
      {
        return ident;
      }

      // If we have the `fixed` keyword but the tokens is not at the end, the next token must be a integer.
      // If it's not, this value is invalid.
      auto firstSymbolValue = CSSPrimitiveValueResolver<Integer<>>::ConsumeAndResolve(tokens, state);
      if (!firstSymbolValue)
      {
        return nullptr;
      }

      return CSSValuePair::Create(krys::move(ident), krys::move(firstSymbolValue));
    }

    if (auto ident = ConsumeIdent<ValueId::Extends>(tokens))
    {
      // There must be a `<counter-style-name>` following the `extends` keyword. If there isn't, this value is
      // invalid.
      auto parsedCounterStyleName = ConsumeCounterStyleName(tokens, state);
      if (!parsedCounterStyleName)
      {
        return nullptr;
      }

      return CSSValuePair::Create(krys::move(ident), krys::move(parsedCounterStyleName));
    }
    return nullptr;
  }

  RefPtr<CSSValue> ConsumeCounterStyleRange(TokenRange &tokens, PropertyParserState &state) noexcept
  {
    // <'tokens'> = [ [ <integer> | infinite ]{2} ]# | auto
    // https://drafts.csswg.org/css-counter-styles-3/#counter-style-tokens

    auto ConsumeCounterStyleRangeBound = [&](TokenRange &tokens) -> RefPtr<CSSPrimitiveValue>
    {
      if (auto infinite = ConsumeIdent<ValueId::Infinite>(tokens))
      {
        return infinite;
      }

      if (auto integer = CSSPrimitiveValueResolver<Integer<>>::ConsumeAndResolve(tokens, state))
      {
        return integer;
      }

      return nullptr;
    };

    if (auto autoValue = ConsumeIdent<ValueId::Auto>(tokens))
    {
      return autoValue;
    }

    auto tokensList = ConsumeListSeparatedBy<',', OneOrMore>(
      tokens,
      [&](auto &tokens) -> RefPtr<CSSValue>
      {
        auto lowerBound = ConsumeCounterStyleRangeBound(tokens);
        if (!lowerBound)
        {
          return nullptr;
        }

        auto upperBound = ConsumeCounterStyleRangeBound(tokens);
        if (!upperBound)
        {
          return nullptr;
        }

        // If the lower bound of any tokens is higher than the upper bound, the entire descriptor is invalid
        // and must be ignored.
        if (lowerBound->IsInteger() && upperBound->IsInteger()
            && lowerBound->ResolveAsIntegerDeprecated() > upperBound->ResolveAsIntegerDeprecated())
        {
          return nullptr;
        }

        return CSSValuePair::CreateNonCoalescing(krys::move(lowerBound), krys::move(upperBound));
      });

    if (!tokens.IsAtEnd() || !tokensList || !tokensList->Size())
    {
      return nullptr;
    }

    return tokensList;
  }

  RefPtr<CSSValue> ConsumeCounterStyleAdditiveSymbols(TokenRange &tokens,
                                                      PropertyParserState &state) noexcept
  {
    // <'additive-symbols'> = [ <integer [0,∞]> && <symbol> ]#
    // https://drafts.csswg.org/css-counter-styles-3/#descdef-counter-style-additive-symbols

    Maybe<int64> lastWeight;
    auto values = ConsumeListSeparatedBy<',', OneOrMore>(
      tokens,
      [&lastWeight](auto &tokens, auto &state) -> RefPtr<CSSValue>
      {
        auto integer = CSSPrimitiveValueResolver<Integer<NonNegative>>::ConsumeAndResolve(tokens, state);
        auto symbol = PropertyParsing::ConsumeSymbol(tokens, state);
        if (!integer)
        {
          if (!symbol)
          {
            return nullptr;
          }

          integer = CSSPrimitiveValueResolver<Integer<NonNegative>>::ConsumeAndResolve(tokens, state);
          if (!integer)
          {
            return nullptr;
          }
        }

        if (!symbol)
        {
          return nullptr;
        }

        // Additive tuples must be specified in order of strictly descending weight.
        auto weight = integer->ResolveAsIntegerDeprecated();
        if (lastWeight && !(weight < lastWeight))
        {
          return nullptr;
        }

        lastWeight = weight;

        return CSSValuePair::Create(krys::move(integer), krys::move(symbol));
      },
      state);

    if (!tokens.IsAtEnd() || !values || !values->Size())
    {
      return nullptr;
    }

    return values;
  }

}