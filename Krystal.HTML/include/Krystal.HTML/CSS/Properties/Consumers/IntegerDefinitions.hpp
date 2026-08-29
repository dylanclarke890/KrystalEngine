#pragma once

#include "Krystal.HTML/CSS/Calc/CSSCalcAllowedSymbols.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/MetaConsumerDefinitions.hpp"

namespace Krys::HTML
{
  struct IntegerValidator
  {
    KRYS_NODISCARD constexpr static Maybe<IntegerUnit>
      Validate(CSSUnitType unitType, CSSPropertyParserState &, CSSPropertyParserOptions) noexcept
    {
      return UnitTraits<IntegerUnit>::Validate(unitType);
    }

    template <auto R, typename V>
    KRYS_NODISCARD static bool IsValid(IntegerRaw<R, V> raw, CSSPropertyParserOptions) noexcept
    {
      return IsValidCanonicalValue(raw);
    }
  };

  template <typename Primitive, typename Validator>
  struct NumberConsumerForIntegerValues
  {
    constexpr static CSSTokenType TokenType = CSSTokenType::Number;

    KRYS_NODISCARD static Maybe<typename Primitive::Raw> Consume(CSSTokenRange &tokens,
                                                                 CSSPropertyParserState &,
                                                                 CSSCalcAllowedSymbols,
                                                                 CSSPropertyParserOptions options) noexcept
    {
      assert(tokens.Peek().Type() == CSSTokenType::Number);

      if (tokens.Peek().NumericValueType() != NumericValueType::Integer)
      {
        return Null;
      }

      auto rawValue = typename Primitive::Raw {IntegerUnit::Integer, tokens.Peek().NumericValue()};

      if constexpr (rawValue.Range.ClampOptions != RangeClampOptions::Default)
      {
        rawValue = PerformParseTimeClamp(rawValue);
      }

      if (!Validator::IsValid(rawValue, options))
      {
        return Null;
      }

      tokens.Discard();
      tokens.DiscardWhitespace();

      return rawValue;
    }
  };

  template <CSSRange R, typename IntType>
  struct ConsumerDefinition<Integer<R, IntType>>
  {
    using FunctionToken = FunctionConsumerForCalcValues<Integer<R, IntType>>;
    using NumberToken = NumberConsumerForIntegerValues<Integer<R, IntType>, IntegerValidator>;
  };
}