#pragma once

#include "Krystal.Booey/CSS/Calc/CalcAllowedSymbols.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/MetaConsumerDefinitions.hpp"

namespace krys::boo::css
{
  struct IntegerValidator
  {
    KRYS_NODISCARD constexpr static Maybe<IntegerUnit>
      Validate(CSSUnitType unitType, PropertyParserState &, PropertyParserOptions) noexcept
    {
      return UnitTraits<IntegerUnit>::Validate(unitType);
    }

    template <auto R, typename V>
    KRYS_NODISCARD static bool IsValid(IntegerRaw<R, V> raw, PropertyParserOptions) noexcept
    {
      return IsValidCanonicalValue(raw);
    }
  };

  template <typename Primitive, typename Validator>
  struct NumberConsumerForIntegerValues
  {
    constexpr static TokenType TokenType = TokenType::Number;

    KRYS_NODISCARD static Maybe<typename Primitive::Raw> Consume(TokenRange &tokens,
                                                                 PropertyParserState &,
                                                                 CalcAllowedSymbols,
                                                                 PropertyParserOptions options) noexcept
    {
      assert(tokens.Peek().Type() == TokenType::Number);

      if (tokens.Peek().NumericValueType() != NumericValueType::Integer)
      {
        return null;
      }

      auto rawValue = typename Primitive::Raw {IntegerUnit::Integer, tokens.Peek().NumericValue()};

      if constexpr (rawValue.Range.ClampOptions != RangeClampOptions::Default)
      {
        rawValue = PerformParseTimeClamp(rawValue);
      }

      if (!Validator::IsValid(rawValue, options))
      {
        return null;
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