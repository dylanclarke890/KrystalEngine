/*
 * Copyright (C) 2024 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/CSS/Properties/Consumers/MetaConsumerDefinitions.hpp"

namespace krys::boo::css
{
  struct IntegerValidator
  {
    KRYS_NODISCARD constexpr static Maybe<IntegerUnit> Validate(UnitType unitType, PropertyParserState &,
                                                                PropertyParserOptions) noexcept
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

    KRYS_NODISCARD static Maybe<typename Primitive::Raw> Consume(TokenRange &tokens, PropertyParserState &,
                                                                 AllowedSymbols,
                                                                 PropertyParserOptions options) noexcept
    {
      krys_debug_assert(tokens.Peek().Type() == TokenType::Number);

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

  template <Range R, typename IntType>
  struct ConsumerDefinition<Integer<R, IntType>>
  {
    using FunctionToken = FunctionConsumerForCalcValues<Integer<R, IntType>>;
    using NumberToken = NumberConsumerForIntegerValues<Integer<R, IntType>, IntegerValidator>;
  };
}