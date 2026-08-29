#pragma once

#include "Krystal.HTML/CSS/Calc/CSSCalcAllowedSymbols.hpp"
#include "Krystal.HTML/CSS/Calc/CSSCalcValue.hpp"
#include "Krystal.HTML/CSS/Parser/CSSToken.hpp"
#include "Krystal.HTML/CSS/Parser/CSSTokenRange.hpp"
#include "Krystal.HTML/CSS/Properties/CSSPropertyParserOptions.hpp"
#include "Krystal.HTML/CSS/Properties/CSSPropertyParserState.hpp"
#include "Krystal.HTML/CSS/Values/Primitives/CSSPrimitiveNumeric.hpp"
#include "Krystal.HTML/CSS/Values/Primitives/CSSPrimitiveNumericRange.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Types/Maybe.hpp"
#include <cassert>
#include <cmath>

namespace Krys::HTML
{
  template <typename>
  struct ConsumerDefinition;

  /// @brief Used to check that a specialization of ConsumerDefinition exists.
  struct HasConsumerDefinition
  {
  private:
    template <typename T, typename U = decltype(ConsumerDefinition<T> {})>
    KRYS_NODISCARD constexpr static bool Exists(int) noexcept
    {
      return true;
    }

    template <typename T>
    KRYS_NODISCARD constexpr static bool Exists(char) noexcept
    {
      return false;
    }

  public:
    template <typename T>
    KRYS_NODISCARD constexpr static bool Check() noexcept
    {
      return Exists<T>(0);
    }
  };

  // FIXME: Bailing on infinity during validation does not seem to match the intent of the spec,
  // though due to the use of "implementation-defined" it may still be conforming. The spec states:
  //
  //   "When a value cannot be explicitly supported due to range/precision limitations, it must
  //    be converted to the closest value supported by the implementation, but how the implementation
  //    defines "closest" is implementation-defined as well."
  //
  // Angles have the additional restriction that:
  //
  //   "If an <angle> must be converted due to exceeding the implementation-defined range of supported
  //    values, it must be clamped to the nearest supported multiple of 360deg."
  //
  // (https://drafts.csswg.org/css-values-4/#numeric-types)
  //
  // The infinity here is produced by the parser when a parsed number is no representable in
  // as a double. A potentially more appropriate behavior would be to have the parser use
  // std::numeric_limits<double>::max() instead. For angles, this would require further integration
  // with the fast_float library (or whatever is currently being used to parse the number) to
  // extract the correct modulo 360deg value.

  /// @brief Shared validator for types dimensional types that need to canonicalize to support range
  /// constraints other than 0 and +/-∞.
  template <typename Raw, typename F>
  KRYS_NODISCARD bool IsValidDimensionValue(Raw raw, F &&functor) noexcept
  {
    if (std::isinf(raw.Value))
    {
      return false;
    }

    if constexpr (raw.Range.Min == -CSSRange::Inf && raw.Range.Max == CSSRange::Inf)
    {
      return true;
    }
    else if constexpr (raw.Range.Min == 0 && raw.Range.Max == CSSRange::Inf)
    {
      return raw.Value >= 0;
    }
    else if constexpr (raw.Range.Min == -CSSRange::Inf && raw.Range.Max == 0)
    {
      return raw.Value <= 0;
    }
    else
    {
      return functor();
    }
  }

  /// @brief Shared validator for types that only support 0 and +/-∞ as valid range constraints.
  template <typename Raw>
  KRYS_NODISCARD bool IsValidNonCanonicalizableDimensionValue(Raw raw) noexcept
  {
    if (std::isinf(raw.Value))
    {
      return false;
    }

    if constexpr (raw.Range.Min == -CSSRange::Inf && raw.Range.Max == CSSRange::Inf)
    {
      return true;
    }
    else if constexpr (raw.Range.Min == 0 && raw.Range.Max == CSSRange::Inf)
    {
      return raw.Value >= 0;
    }
    else if constexpr (raw.Range.Min == -CSSRange::Inf && raw.Range.Max == 0)
    {
      return raw.Value <= 0;
    }
  }

  /// @brief Shared validator for types that always have their value in canonical units (number, percentage,
  /// flex).
  template <typename Raw>
  KRYS_NODISCARD bool IsValidCanonicalValue(Raw raw) noexcept
  {
    if (std::isinf(raw.Value))
    {
      return false;
    }

    if constexpr (raw.Range.Min == -CSSRange::Inf && raw.Range.Max == CSSRange::Inf)
    {
      return true;
    }
    else if constexpr (raw.Range.Max == CSSRange::Inf)
    {
      return raw.Value >= raw.Range.Min;
    }
    else if constexpr (raw.Range.Min == -CSSRange::Inf)
    {
      return raw.Value <= raw.Range.Max;
    }
    else
    {
      return raw.Value >= raw.Range.Min && raw.Value <= raw.Range.Max;
    }
  }

  /// @brief Shared clamping utility.
  template <typename Raw>
  KRYS_NODISCARD Raw PerformParseTimeClamp(Raw raw) noexcept
  {
    static_assert(raw.Range.ClampOptions != RangeClampOptions::Default);

    if constexpr (raw.Range.ClampOptions == RangeClampOptions::ClampLower)
    {
      return {std::max<typename Raw::ResolvedValueType>(raw.Value, raw.Range.Min)};
    }
    else if constexpr (raw.Range.ClampOptions == RangeClampOptions::ClampUpper)
    {
      return {std::min<typename Raw::ResolvedValueType>(raw.Value, raw.Range.Max)};
    }
    else if constexpr (raw.Range.ClampOptions == RangeClampOptions::ClampBoth)
    {
      return {std::clamp<typename Raw::ResolvedValueType>(raw.Value, raw.Range.Min, raw.Range.Max)};
    }
  }

  // Shared consumer for `Dimension` tokens.
  template <typename Primitive, typename Validator>
  struct DimensionConsumer
  {
    constexpr static CSSTokenType TokenType = CSSTokenType::Dimension;

    KRYS_NODISCARD static Maybe<typename Primitive::Raw> Consume(CSSTokenRange &tokens,
                                                                 CSSPropertyParserState &state,
                                                                 CSSCalcAllowedSymbols,
                                                                 CSSPropertyParserOptions options) noexcept
    {
      assert(tokens.Peek().Type() == CSSTokenType::Dimension);

      auto &token = tokens.Peek();

      auto validatedUnit = Validator::Validate(ParseCSSUnitType(token.Unit()), state, options);
      if (!validatedUnit)
      {
        return Null;
      }

      auto rawValue = typename Primitive::Raw {*validatedUnit, token.NumericValue()};

      if constexpr (Primitive::Raw::Range.ClampOptions != RangeClampOptions::Default)
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

  // Shared consumer for `Percentage` tokens.
  template <typename Primitive, typename Validator, auto Unit>
  struct PercentageConsumer
  {
    constexpr static CSSTokenType TokenType = CSSTokenType::Percentage;

    KRYS_NODISCARD static Maybe<typename Primitive::Raw> Consume(CSSTokenRange &tokens,
                                                                 CSSPropertyParserState &,
                                                                 CSSCalcAllowedSymbols,
                                                                 CSSPropertyParserOptions options) noexcept
    {
      assert(tokens.Peek().Type() == CSSTokenType::Percentage);

      auto rawValue = typename Primitive::Raw {Unit, tokens.Peek().NumericValue()};

      if constexpr (Primitive::Raw::Range.ClampOptions != RangeClampOptions::Default)
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

  // Shared consumer for `Number` tokens.
  template <typename Primitive, typename Validator>
  struct NumberConsumer
  {
    constexpr static CSSTokenType TokenType = CSSTokenType::Number;

    KRYS_NODISCARD static Maybe<typename Primitive::Raw> Consume(CSSTokenRange &tokens,
                                                                 CSSPropertyParserState &,
                                                                 CSSCalcAllowedSymbols,
                                                                 CSSPropertyParserOptions options) noexcept
    {
      assert(tokens.Peek().Type() == CSSTokenType::Number);

      auto rawValue = typename Primitive::Raw {NumberUnit::Number, tokens.Peek().NumericValue()};

      if constexpr (Primitive::Raw::Range.ClampOptions != RangeClampOptions::Default)
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

  // Shared consumer for `Number` tokens for use by dimensional primitives that support "unitless" values.
  template <typename Primitive, typename Validator, auto Unit>
  struct NumberConsumerForUnitlessValues
  {
    constexpr static CSSTokenType TokenType = CSSTokenType::Number;

    KRYS_NODISCARD static Maybe<typename Primitive::Raw> Consume(CSSTokenRange &tokens,
                                                                 CSSPropertyParserState &state,
                                                                 CSSCalcAllowedSymbols,
                                                                 CSSPropertyParserOptions options) noexcept
    {
      assert(tokens.Peek().Type() == CSSTokenType::Number);

      auto numericValue = tokens.Peek().NumericValue();
      if (!Validator::ShouldAcceptUnitlessValue(numericValue, state, options))
      {
        return Null;
      }

      auto rawValue = typename Primitive::Raw {Unit, numericValue};

      if constexpr (Primitive::Raw::Range.ClampOptions != RangeClampOptions::Default)
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

  // Shared consumer for `Function` tokens that processes `calc()` for the provided primitive.
  template <typename Primitive>
  struct FunctionConsumerForCalcValues
  {
    constexpr static CSSTokenType TokenType = CSSTokenType::Function;

    KRYS_NODISCARD static Maybe<typename Primitive::Calc> Consume(CSSTokenRange &tokens,
                                                                  CSSPropertyParserState &state,
                                                                  CSSCalcAllowedSymbols symbolsAllowed,
                                                                  CSSPropertyParserOptions options) noexcept
    {
      assert(tokens.Peek().Type() == CSSTokenType::Function);

      auto rangeCopy = tokens;
      if (auto value = CSSCalcValue::Parse(rangeCopy, state, Primitive::Category, Primitive::Range,
                                           Krys::Move(symbolsAllowed), options))
      {
        tokens = rangeCopy;
        // TODO: fix this return type
        return Null;
      }

      return Null;
    }
  };

  template <typename T>
  struct KeywordConsumer
  {
    constexpr static CSSTokenType TokenType = CSSTokenType::Ident;

    KRYS_NODISCARD static Maybe<T> Consume(CSSTokenRange &tokens, CSSPropertyParserState &,
                                           CSSCalcAllowedSymbols, CSSPropertyParserOptions) noexcept
    {
      assert(tokens.Peek().Type() == CSSTokenType::Ident);

      if (tokens.Peek().ValueId() == T::value)
      {
        tokens.Discard();
        tokens.DiscardWhitespace();

        return T {};
      }

      return Null;
    }
  };
}