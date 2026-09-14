#pragma once

#include "Krystal.Booey/CSS/Calc/CalcAllowedSymbols.hpp"
#include "Krystal.Booey/CSS/Parser/Token.hpp"
#include "Krystal.Booey/CSS/Parser/TokenRange.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/MetaConsumerDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyParserOptions.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyParserState.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/PrimitiveNumeric.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/Range.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/Variant.hpp"
#include <cassert>
#include <cmath>

namespace krys::boo::css
{
#pragma region Meta Consumer Dispatchers

  template <TokenType TokenType, typename Consumer, typename = void>
  struct MetaConsumerDispatcher
  {
    constexpr static bool Supported = false;
  };

  template <typename Consumer>
  struct MetaConsumerDispatcher<TokenType::Function, Consumer,
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
  struct MetaConsumerDispatcher<TokenType::Number, Consumer, typename void_t<typename Consumer::NumberToken>>
  {
    constexpr static bool Supported = true;

    template <typename... Args>
    KRYS_NODISCARD static decltype(auto) Consume(Args &&...args) noexcept
    {
      return Consumer::NumberToken::Consume(std::forward<Args>(args)...);
    }
  };

  template <typename Consumer>
  struct MetaConsumerDispatcher<TokenType::Percentage, Consumer,
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
  struct MetaConsumerDispatcher<TokenType::Dimension, Consumer,
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
  struct MetaConsumerDispatcher<TokenType::Ident, Consumer, typename void_t<typename Consumer::IdentToken>>
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
    template <TokenType, typename ResultType>
    KRYS_NODISCARD static std::nullopt_t Consume(TokenRange &tokens, PropertyParserState &state,
                                                 CalcAllowedSymbols allowedSymbols,
                                                 PropertyParserOptions options) noexcept
    {
      return null;
    }

    template <TokenType, typename ResultType, typename F>
    KRYS_NODISCARD static std::nullopt_t Consume(TokenRange &tokens, PropertyParserState &state,
                                                 CalcAllowedSymbols allowedSymbols,
                                                 PropertyParserOptions options, F &&func) noexcept
    {
      return null;
    }
  };

  // Actionable case, checks if the `Consumer` defined for type `T` supports the
  // current token, trying to consume if it does, and in either case, falling
  // back to recursively trying the same on the remainder of the type list `Ts...`.
  template <typename T, typename... Ts>
  struct MetaConsumerUnroller<T, Ts...>
  {
    template <TokenType TokenType, typename ResultType>
    KRYS_NODISCARD static Maybe<ResultType> Consume(TokenRange &tokens, PropertyParserState &state,
                                                    CalcAllowedSymbols symbolsAllowed,
                                                    PropertyParserOptions options) noexcept
    {
      using Consumer = MetaConsumerDispatcher<TokenType, ConsumerDefinition<T>>;
      if constexpr (Consumer::Supported)
      {
        if (auto result = Consumer::Consume(tokens, state, symbolsAllowed, options))
        {
          return {T {*result}};
        }
      }
      return MetaConsumerUnroller<Ts...>::template Consume<TokenType, ResultType>(tokens, state,
                                                                                  symbolsAllowed, options);
    }

    template <TokenType TokenType, typename ResultType, typename F>
    KRYS_NODISCARD static Maybe<ResultType> Consume(TokenRange &tokens, PropertyParserState &state,
                                                    CalcAllowedSymbols symbolsAllowed,
                                                    PropertyParserOptions options, F &&func) noexcept
    {
      using Consumer = MetaConsumerDispatcher<TokenType, ConsumerDefinition<T>>;
      if constexpr (Consumer::Supported)
      {
        if (auto result = Consumer::Consume(tokens, state, symbolsAllowed, options))
        {
          return func(T {*result});
        }
      }
      return MetaConsumerUnroller<Ts...>::template Consume<TokenType, ResultType>(
        tokens, state, symbolsAllowed, options, std::forward<F>(func));
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
  //    auto result = MetaConsumer<CSS::Percentage<R>, CSS::Number<R>>::consume(tokens, ...);
  //
  // If a caller wants to avoid the overhead of switching on the returned variant
  // result, an alternative overload of `consume` is provided which takes an additional
  // `functor` argument which gets called with the result:
  //
  //    auto result = MetaConsumer<Percentage<R>, Number<R>>::Consume(tokens, ...,
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
    KRYS_NODISCARD static decltype(auto) Consume(TokenRange &tokens, PropertyParserState &state,
                                                 CalcAllowedSymbols symbolsAllowed,
                                                 PropertyParserOptions options, F &&...f) noexcept
    {
      auto visitor = CreateVisitor(std::forward<F>(f)...);
      using ResultType = decltype(visitor(std::declval<T>()));

      switch (tokens.Peek().Type())
      {
        case TokenType::Function:
        {
          return Unroller::template Consume<TokenType::Function, ResultType>(
            tokens, state, krys::move(symbolsAllowed), options, visitor);
        }
        case TokenType::Number:
        {
          return Unroller::template Consume<TokenType::Number, ResultType>(
            tokens, state, krys::move(symbolsAllowed), options, visitor);
        }
        case TokenType::Percentage:
        {
          return Unroller::template Consume<TokenType::Percentage, ResultType>(
            tokens, state, krys::move(symbolsAllowed), options, visitor);
        }
        case TokenType::Dimension:
        {
          return Unroller::template Consume<TokenType::Dimension, ResultType>(
            tokens, state, krys::move(symbolsAllowed), options, visitor);
        }
        case TokenType::Ident:
        {
          return Unroller::template Consume<TokenType::Ident, ResultType>(
            tokens, state, krys::move(symbolsAllowed), options, visitor);
        }
        default:
        {
          return Maybe<ResultType> {};
        }
      }
    }

    // Overloaded with the `PropertyParserOptions` parameter removed so it can be defaulted when using the
    // continuation functor parameters.
    template <typename... F>
    KRYS_NODISCARD static decltype(auto) Consume(TokenRange &tokens, PropertyParserState &state,
                                                 CalcAllowedSymbols symbolsAllowed, F &&...f) noexcept
    {
      return Consume(tokens, state, krys::move(symbolsAllowed), {}, std::forward<F>(f)...);
    }

    // Overloaded with the `CalcAllowedSymbols` parameter removed so it can be defaulted when using the
    // continuation functor parameters.
    template <typename... F>
    KRYS_NODISCARD static decltype(auto) Consume(TokenRange &tokens, PropertyParserState &state,
                                                 PropertyParserOptions options, F &&...f) noexcept
    {
      return Consume(tokens, state, {}, options, std::forward<F>(f)...);
    }

    // Overloaded with the `PropertyParserOptions` and `CalcAllowedSymbols` parameters removed so they
    // can be defaulted when using the continuation functor parameters.
    template <typename... F>
    KRYS_NODISCARD static decltype(auto) Consume(TokenRange &tokens, PropertyParserState &state,
                                                 F &&...f) noexcept
    {
      return Consume(tokens, state, {}, {}, std::forward<F>(f)...);
    }

    // Overloaded with no continuation functor parameters allowing a for simplified interface when returning a
    // single value / or Variant is acceptable.
    KRYS_NODISCARD static decltype(auto) Consume(TokenRange &tokens, PropertyParserState &state,
                                                 CalcAllowedSymbols symbolsAllowed = {},
                                                 PropertyParserOptions options = {}) noexcept
    {
      using ResultType = typename MetaConsumeResult<T, Ts...>::type;

      return Consume(tokens, state, krys::move(symbolsAllowed), options,
                     [](auto &&value) { return ResultType {krys::move(value)}; });
    }
  };
}