#pragma once

#include "Krystal.Booey/CSS/Parser/ParserIdioms.hpp"
#include "Krystal.Booey/CSS/Parser/Token.hpp"
#include "Krystal.Booey/CSS/Parser/TokenRange.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
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

  KRYS_NODISCARD inline RefPtr<PrimitiveValue> ConsumeIdent(TokenRange &tokens) noexcept
  {
    if (auto result = ConsumeIdentRaw(tokens))
    {
      return PrimitiveValue::Create(*result);
    }

    return nullptr;
  }

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

  KRYS_NODISCARD inline RefPtr<PrimitiveValue> ConsumeIdentRange(TokenRange &tokens, ValueId lower,
                                                                    ValueId upper) noexcept
  {
    auto value = ConsumeIdentRangeRaw(tokens, lower, upper);
    if (!value)
    {
      return nullptr;
    }

    return PrimitiveValue::Create(*value);
  }

  template <typename... EmptyBaseCase>
  KRYS_NODISCARD bool IdentMatches(ValueId valueId) noexcept
  {
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

  KRYS_NODISCARD inline RefPtr<PrimitiveValue> ConsumeCustomIdent(TokenRange &tokens,
                                                                     bool shouldLowercase = false) noexcept
  {
    auto identifier = ConsumeCustomIdentRaw(tokens, shouldLowercase);
    if (!identifier.has_value())
    {
      return nullptr;
    }

    return PrimitiveValue::Create(krys::move(*identifier));
  }

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
    if (!identifier->starts_with(u8"--"))
    {
      tokens = tokensCopy;
      return null;
    }

    return identifier;
  }

  KRYS_NODISCARD inline RefPtr<PrimitiveValue> ConsumeDashedIdent(TokenRange &tokens,
                                                                     bool shouldLowercase = false) noexcept
  {
    auto identifier = ConsumeDashedIdentRaw(tokens, shouldLowercase);
    if (!identifier.has_value())
    {
      return nullptr;
    }

    return PrimitiveValue::Create(krys::move(*identifier));
  }

#pragma endregion
}