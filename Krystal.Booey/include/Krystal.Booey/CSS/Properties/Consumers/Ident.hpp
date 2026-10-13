#pragma once

#include "Krystal.Booey/CSS/Parser/ParserIdioms.hpp"
#include "Krystal.Booey/CSS/Parser/Token.hpp"
#include "Krystal.Booey/CSS/Parser/TokenRange.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Core/Concepts.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include <ranges>

namespace krys::boo::css::PropertyParserHelpers
{
#pragma region <ident>

  KRYS_NODISCARD inline Maybe<ValueId> ConsumeIdentRaw(TokenRange &tokens) noexcept
  {
    if (tokens.Peek().Type() != TokenType::Ident)
    {
      return null;
    }

    const auto &token = tokens.Consume();
    tokens.DiscardWhitespace();

    return token.ValueId();
  }

  KRYS_NODISCARD RefPtr<PrimitiveValue> ConsumeIdent(TokenRange &tokens) noexcept;

  inline void DiscardIdent(TokenRange &tokens) noexcept
  {
    if (tokens.Peek().Type() == TokenType::Ident)
    {
      tokens.Discard();
      tokens.DiscardWhitespace();
    }
  }

  KRYS_NODISCARD inline Maybe<ValueId> ConsumeIdentRangeRaw(TokenRange &tokens, ValueId lower,
                                                            ValueId upper) noexcept
  {
    if (tokens.Peek().ValueId() < lower || tokens.Peek().ValueId() > upper)
    {
      return null;
    }

    return ConsumeIdentRaw(tokens);
  }

  KRYS_NODISCARD RefPtr<PrimitiveValue> ConsumeIdentRange(TokenRange &tokens, ValueId lower,
                                                          ValueId upper) noexcept;

  template <typename... EmptyBaseCase>
  KRYS_NODISCARD bool IdentMatches(ValueId valueId) noexcept
  {
    (void)valueId;
    return false;
  }

  template <ValueId Head, ValueId... Tail>
  KRYS_NODISCARD bool IdentMatches(ValueId valueId) noexcept
  {
    return valueId == Head || IdentMatches<Tail...>(valueId);
  }

  template <ValueId... Names>
  KRYS_NODISCARD Maybe<ValueId> ConsumeIdentRaw(TokenRange &tokens) noexcept
  {
    if (tokens.Peek().Type() != TokenType::Ident || !IdentMatches<Names...>(tokens.Peek().ValueId()))
    {
      return null;
    }

    const auto &token = tokens.Consume();
    tokens.DiscardWhitespace();

    return token.ValueId();
  }

  template <ValueId... Names>
  KRYS_NODISCARD RefPtr<PrimitiveValue> ConsumeIdent(TokenRange &tokens) noexcept
  {
    if (tokens.Peek().Type() != TokenType::Ident || !IdentMatches<Names...>(tokens.Peek().ValueId()))
    {
      return nullptr;
    }

    const auto &token = tokens.Consume();
    tokens.DiscardWhitespace();

    return PrimitiveValue::Create(token.ValueId());
  }

  template <typename Predicate, typename... Args>
  KRYS_NODISCARD Maybe<ValueId> ConsumeIdentRaw(TokenRange &tokens, Predicate &&predicate,
                                                Args &&...args) noexcept
  {
    if (auto keyword = tokens.Peek().ValueId(); predicate(keyword, std::forward<Args>(args)...))
    {
      tokens.Discard();
      tokens.DiscardWhitespace();

      return keyword;
    }

    return null;
  }

  template <typename Predicate, typename... Args>
  KRYS_NODISCARD RefPtr<PrimitiveValue> ConsumeIdent(TokenRange &tokens, Predicate &&predicate,
                                                     Args &&...args) noexcept
  {
    if (auto keyword = tokens.Peek().ValueId(); predicate(keyword, std::forward<Args>(args)...))
    {
      tokens.Discard();
      tokens.DiscardWhitespace();

      return PrimitiveValue::Create(keyword);
    }

    return nullptr;
  }

  /// @brief Maps the next token's identifier with `mapping`, without consuming it.
  /// @param mapping Takes a `ValueId` and returns a `Maybe` of the mapped value (`null` if unmapped).
  template <Callable<ValueId> TMapping>
  KRYS_NODISCARD auto PeekIdentUsingMapping(TokenRange &tokens, TMapping &&mapping) noexcept
  {
    return mapping(tokens.Peek().ValueId());
  }

  /// @brief Consumes the next token if `mapping` maps its identifier to a value.
  /// @param mapping Takes a `ValueId` and returns a `Maybe` of the mapped value (`null` if unmapped).
  template <Callable<ValueId> TMapping>
  KRYS_NODISCARD auto ConsumeIdentUsingMapping(TokenRange &tokens, TMapping &&mapping) noexcept
  {
    auto value = mapping(tokens.Peek().ValueId());
    if (value)
    {
      tokens.Discard();
      tokens.DiscardWhitespace();
    }
    return value;
  }

#pragma endregion

#pragma region <custom-ident> - https://drafts.csswg.org/css-values/#custom-idents

  KRYS_NODISCARD inline Maybe<CSSOMString> ConsumeCustomIdentRaw(TokenRange &tokens,
                                                                 bool shouldLowercase = false) noexcept
  {
    if (tokens.Peek().Type() != TokenType::Ident || !IsValidCustomIdentifier(tokens.Peek().ValueId()))
    {
      return null;
    }

    auto &token = tokens.Consume();
    tokens.DiscardWhitespace();

    auto identifier = token.IdentCodePoints();

    return shouldLowercase ? krys::text::ToASCIILower(identifier) : CSSOMString(identifier);
  }

  KRYS_NODISCARD RefPtr<PrimitiveValue> ConsumeCustomIdent(TokenRange &tokens,
                                                           bool shouldLowercase = false) noexcept;

  KRYS_NODISCARD inline RefPtr<PrimitiveValue>
    ConsumeCustomIdentExcluding(TokenRange &tokens, std::initializer_list<const ValueId> excluding,
                                bool shouldLowercase = false) noexcept
  {
    if (std::ranges::find(excluding, tokens.Peek().ValueId()) != excluding.end())
    {
      return nullptr;
    }

    return ConsumeCustomIdent(tokens, shouldLowercase);
  }

#pragma endregion

#pragma region <dashed-ident> - https://drafts.csswg.org/css-values/#dashed-idents

  KRYS_NODISCARD inline Maybe<CSSOMString> ConsumeDashedIdentRaw(TokenRange &tokens,
                                                                 bool shouldLowercase = false) noexcept
  {
    auto tokensCopy = tokens;

    auto identifier = ConsumeCustomIdentRaw(tokens, shouldLowercase);
    if (!identifier || !identifier->starts_with(u8"--"))
    {
      tokens = tokensCopy;
      return null;
    }

    return identifier;
  }

  KRYS_NODISCARD RefPtr<PrimitiveValue> ConsumeDashedIdent(TokenRange &tokens,
                                                           bool shouldLowercase = false) noexcept;

#pragma endregion
}