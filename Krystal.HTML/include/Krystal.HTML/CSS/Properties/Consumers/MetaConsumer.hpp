#pragma once

#include "Krystal.HTML/CSS/Calc/CSSCalcAllowedSymbols.hpp"
#include "Krystal.HTML/CSS/Parser/CSSToken.hpp"
#include "Krystal.HTML/CSS/Parser/CSSTokenRange.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/MetaConsumerDefinitions.hpp"
#include "Krystal.HTML/CSS/Properties/CSSPropertyParserOptions.hpp"
#include "Krystal.HTML/CSS/Properties/CSSPropertyParserState.hpp"
#include "Krystal.HTML/CSS/Values/Primitives/CSSPrimitiveNumeric.hpp"
#include "Krystal.HTML/CSS/Values/Primitives/CSSPrimitiveNumericRange.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Types/Maybe.hpp"
#include "Krystal.Lib/Types/Variant.hpp"
#include <cassert>
#include <cmath>

namespace Krys::HTML
{
#pragma region Meta Consumer Dispatchers

  template <CSSTokenType TokenType, typename Consumer, typename = void>
  struct MetaConsumerDispatcher
  {
    constexpr static bool Supported = false;
  };

  template <typename Consumer>
  struct MetaConsumerDispatcher<CSSTokenType::Function, Consumer,
                                typename void_t<typename Consumer::FunctionToken>>
  {
    constexpr static bool Supported = true;

    template <typename... Args>
    KRYS_NODISCARD static decltype(auto) Consume(Args &&...args) noexcept
    {
      return Consumer::FunctionToken::Consume(std::forward<Args>(args)...);
    }
  };

  template <typename Consumer>
  struct MetaConsumerDispatcher<CSSTokenType::Number, Consumer,
                                typename void_t<typename Consumer::NumberToken>>
  {
    constexpr static bool Supported = true;

    template <typename... Args>
    KRYS_NODISCARD static decltype(auto) Consume(Args &&...args) noexcept
    {
      return Consumer::NumberToken::Consume(std::forward<Args>(args)...);
    }
  };

  template <typename Consumer>
  struct MetaConsumerDispatcher<CSSTokenType::Percentage, Consumer,
                                typename void_t<typename Consumer::PercentageToken>>
  {
    constexpr static bool Supported = true;

    template <typename... Args>
    KRYS_NODISCARD static decltype(auto) Consume(Args &&...args) noexcept
    {
      return Consumer::PercentageToken::Consume(std::forward<Args>(args)...);
    }
  };

  template <typename Consumer>
  struct MetaConsumerDispatcher<CSSTokenType::Dimension, Consumer,
                                typename void_t<typename Consumer::DimensionToken>>
  {
    constexpr static bool Supported = true;

    template <typename... Args>
    KRYS_NODISCARD static decltype(auto) Consume(Args &&...args) noexcept
    {
      return Consumer::DimensionToken::Consume(std::forward<Args>(args)...);
    }
  };

  template <typename Consumer>
  struct MetaConsumerDispatcher<CSSTokenType::Ident, Consumer, typename void_t<typename Consumer::IdentToken>>
  {
    constexpr static bool Supported = true;

    template <typename... Args>
    KRYS_NODISCARD static decltype(auto) Consume(Args &&...args) noexcept
    {
      return Consumer::IdentToken::Consume(std::forward<Args>(args)...);
    }
  };

#pragma endregion

#pragma region Meta Consumer Unroller

  /// The `MetaConsumerUnroller` gives each type in the consumer list (`Ts...`) a chance to consume the token.
  /// It recursively peels off types from the type list, checks if the consumer supports this token type, and
  /// then calls to the MetaConsumerDispatcher to actually call right `consume` function.

  // Empty case, used to indicate no more types remain to try.
  template <typename... Ts>
  struct MetaConsumerUnroller
  {
    template <CSSTokenType, typename ResultType>
    KRYS_NODISCARD static std::nullopt_t Consume(CSSTokenRange &tokens, CSSPropertyParserState &state,
                                                 CSSCalcAllowedSymbols allowedSymbols,
                                                 CSSPropertyParserOptions options) noexcept
    {
      return Null;
    }

    template <CSSTokenType, typename ResultType, typename F>
    KRYS_NODISCARD static std::nullopt_t Consume(CSSTokenRange &tokens, CSSPropertyParserState &state,
                                                 CSSCalcAllowedSymbols allowedSymbols,
                                                 CSSPropertyParserOptions options, F &&func) noexcept
    {
      return Null;
    }
  };

  // Actionable case, checks if the `Consumer` defined for type `T` supports the
  // current token, trying to consume if it does, and in either case, falling
  // back to recursively trying the same on the remainder of the type list `Ts...`.
  template <typename T, typename... Ts>
  struct MetaConsumerUnroller<T, Ts...>
  {
    template <CSSTokenType TokenType, typename ResultType>
    KRYS_NODISCARD static Maybe<ResultType> Consume(CSSTokenRange &range, CSSPropertyParserState &state,
                                                    CSSCalcAllowedSymbols symbolsAllowed,
                                                    CSSPropertyParserOptions options) noexcept
    {
      using Consumer = MetaConsumerDispatcher<TokenType, ConsumerDefinition<T>>;
      if constexpr (Consumer::Supported)
      {
        if (auto result = Consumer::Consume(range, state, symbolsAllowed, options))
        {
          return {T {*result}};
        }
      }
      return MetaConsumerUnroller<Ts...>::template Consume<TokenType, ResultType>(range, state,
                                                                                  symbolsAllowed, options);
    }

    template <CSSTokenType TokenType, typename ResultType, typename F>
    KRYS_NODISCARD static Maybe<ResultType> Consume(CSSTokenRange &range, CSSPropertyParserState &state,
                                                    CSSCalcAllowedSymbols symbolsAllowed,
                                                    CSSPropertyParserOptions options, F &&func) noexcept
    {
      using Consumer = MetaConsumerDispatcher<TokenType, ConsumerDefinition<T>>;
      if constexpr (Consumer::Supported)
      {
        if (auto result = Consumer::Consume(range, state, symbolsAllowed, options))
        {
          return func(T {*result});
        }
      }
      return MetaConsumerUnroller<Ts...>::template Consume<TokenType, ResultType>(
        range, state, symbolsAllowed, options, std::forward<F>(func));
    }
  };

#pragma endregion

  /// The result of a meta consume.
  /// To be used with a list of `CSS` types (e.g. `ConsumeResult<CSS::Angle<Range>, CSS::Percentage<Range>,
  /// CSS::Keyword::None>`), which will yield a result type of either a Variant of those types
  /// (e.g.`Variant<CSS::Angle<Range>, CSS::Percentage<Range>, CSS::Keyword::None>`) or the type itself if
  /// only a single type was specified.
  template <typename... Ts>
  struct MetaConsumeResult
  {
    using TypeList = std::tuple<Ts...>;
    using type = VariantOrSingle<Ts...>;
  };

  // The `MetaConsumer` is the main driver of token consumption, dispatching
  // to a `MetaConsumerUnroller` based on token type. Caller use this directly.
  // An example use that attempts to consumer either a <number> or <percentage>
  // looks like (argument list elided for brevity):
  //
  //    auto result = MetaConsumer<CSS::Percentage<R>, CSS::Number<R>>::consume(range, ...);
  //
  // If a caller wants to avoid the overhead of switching on the returned variant
  // result, an alternative overload of `consume` is provided which takes an additional
  // `functor` argument which gets called with the result:
  //
  //    auto result = MetaConsumer<Percentage<R>, Number<R>>::Consume(range, ...,
  //        [](Percentage<R> percentage) { ... },
  //        [](Number<R> number) { ... }
  //    );
  template <typename T, typename... Ts>
  struct MetaConsumer
  {
    static_assert(AllTrue<HasConsumerDefinition::Check<T>(), HasConsumerDefinition::Check<Ts>()...>,
                  "ConsumerDefinition<T> is incomplete or has not been included.");

    using Unroller = MetaConsumerUnroller<T, Ts...>;

    template <typename... F>
    KRYS_NODISCARD static decltype(auto) Consume(CSSTokenRange &range, CSSPropertyParserState &state,
                                                 CSSCalcAllowedSymbols symbolsAllowed,
                                                 CSSPropertyParserOptions options, F &&...f) noexcept
    {
      auto visitor = CreateVisitor(std::forward<F>(f)...);
      using ResultType = decltype(visitor(std::declval<T>()));

      switch (range.Peek().Type())
      {
        case CSSTokenType::Function:
        {
          return Unroller::template Consume<CSSTokenType::Function, ResultType>(
            range, state, Krys::Move(symbolsAllowed), options, visitor);
        }
        case CSSTokenType::Number:
        {
          return Unroller::template Consume<CSSTokenType::Number, ResultType>(
            range, state, Krys::Move(symbolsAllowed), options, visitor);
        }
        case CSSTokenType::Percentage:
        {
          return Unroller::template Consume<CSSTokenType::Percentage, ResultType>(
            range, state, Krys::Move(symbolsAllowed), options, visitor);
        }
        case CSSTokenType::Dimension:
        {
          return Unroller::template Consume<CSSTokenType::Dimension, ResultType>(
            range, state, Krys::Move(symbolsAllowed), options, visitor);
        }
        case CSSTokenType::Ident:
        {
          return Unroller::template Consume<CSSTokenType::Ident, ResultType>(
            range, state, Krys::Move(symbolsAllowed), options, visitor);
        }
        default:
        {
          return Maybe<ResultType> {};
        }
      }
    }

    // Overloaded with the `CSSPropertyParserOptions` parameter removed so it can be defaulted when using the
    // continuation functor parameters.
    template <typename... F>
    KRYS_NODISCARD static decltype(auto) Consume(CSSTokenRange &range, CSSPropertyParserState &state,
                                                 CSSCalcAllowedSymbols symbolsAllowed, F &&...f) noexcept
    {
      return Consume(range, state, Krys::Move(symbolsAllowed), {}, std::forward<F>(f)...);
    }

    // Overloaded with the `CSSCalcAllowedSymbols` parameter removed so it can be defaulted when using the
    // continuation functor parameters.
    template <typename... F>
    KRYS_NODISCARD static decltype(auto) Consume(CSSTokenRange &range, CSSPropertyParserState &state,
                                                 CSSPropertyParserOptions options, F &&...f) noexcept
    {
      return Consume(range, state, {}, options, std::forward<F>(f)...);
    }

    // Overloaded with the `CSSPropertyParserOptions` and `CSSCalcAllowedSymbols` parameters removed so they
    // can be defaulted when using the continuation functor parameters.
    template <typename... F>
    KRYS_NODISCARD static decltype(auto) Consume(CSSTokenRange &range, CSSPropertyParserState &state,
                                                 F &&...f) noexcept
    {
      return Consume(range, state, {}, {}, std::forward<F>(f)...);
    }

    // Overloaded with no continuation functor parameters allowing a for simplified interface when returning a
    // single value / or Variant is acceptable.
    KRYS_NODISCARD static decltype(auto) Consume(CSSTokenRange &range, CSSPropertyParserState &state,
                                                 CSSCalcAllowedSymbols symbolsAllowed = {},
                                                 CSSPropertyParserOptions options = {}) noexcept
    {
      using ResultType = typename MetaConsumeResult<T, Ts...>::type;

      return Consume(range, state, Krys::Move(symbolsAllowed), options,
                     [](auto &&value) { return ResultType {Krys::Move(value)}; });
    }
  };
}