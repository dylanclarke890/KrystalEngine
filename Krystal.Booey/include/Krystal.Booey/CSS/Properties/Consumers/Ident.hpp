#pragma once

#include "Krystal.Booey/CSS/Parser/ParserIdioms.hpp"
#include "Krystal.Booey/CSS/Parser/Token.hpp"
#include "Krystal.Booey/CSS/Parser/TokenRange.hpp"
#include "Krystal.Booey/CSS/Values/CSSPrimitiveValue.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include <ranges>

namespace krys::boo::css::CSSPropertyParserHelpers
{
#pragma region <ident>

  KRYS_NODISCARD Maybe<CSSValueId> ConsumeIdentRaw(TokenRange &tokens) noexcept
  {
    if (tokens.Peek().Type() != TokenType::Ident)
    {
      return null;
    }

    const auto &token = tokens.Consume();
    tokens.DiscardWhitespace();

    return token.ValueId();
  }

  KRYS_NODISCARD RefPtr<CSSPrimitiveValue> ConsumeIdent(TokenRange &tokens) noexcept
  {
    if (auto result = ConsumeIdentRaw(tokens))
    {
      return CSSPrimitiveValue::Create(*result);
    }

    return nullptr;
  }

  void DiscardIdent(TokenRange &tokens) noexcept
  {
    if (tokens.Peek().Type() == TokenType::Ident)
    {
      tokens.Discard();
      tokens.DiscardWhitespace();
    }
  }

  KRYS_NODISCARD Maybe<CSSValueId> ConsumeIdentRangeRaw(TokenRange &tokens, CSSValueId lower,
                                                        CSSValueId upper) noexcept
  {
    if (tokens.Peek().ValueId() < lower || tokens.Peek().ValueId() > upper)
    {
      return null;
    }

    return ConsumeIdentRaw(tokens);
  }

  KRYS_NODISCARD RefPtr<CSSPrimitiveValue> ConsumeIdentRange(TokenRange &tokens, CSSValueId lower,
                                                             CSSValueId upper) noexcept
  {
    auto value = ConsumeIdentRangeRaw(tokens, lower, upper);
    if (!value)
    {
      return nullptr;
    }

    return CSSPrimitiveValue::Create(*value);
  }

  template <typename... EmptyBaseCase>
  KRYS_NODISCARD bool IdentMatches(CSSValueId valueId) noexcept
  {
    return false;
  }

  template <CSSValueId Head, CSSValueId... Tail>
  KRYS_NODISCARD bool IdentMatches(CSSValueId valueId) noexcept
  {
    return valueId == Head || IdentMatches<Tail...>(valueId);
  }

  template <CSSValueId... Names>
  KRYS_NODISCARD Maybe<CSSValueId> ConsumeIdentRaw(TokenRange &tokens) noexcept
  {
    if (tokens.Peek().Type() != TokenType::Ident || !IdentMatches<Names...>(tokens.Peek().ValueId()))
    {
      return null;
    }

    const auto &token = tokens.Consume();
    tokens.DiscardWhitespace();

    return token.ValueId();
  }

  template <CSSValueId... Names>
  KRYS_NODISCARD RefPtr<CSSPrimitiveValue> ConsumeIdent(TokenRange &tokens) noexcept
  {
    if (tokens.Peek().Type() != TokenType::Ident || !IdentMatches<Names...>(tokens.Peek().ValueId()))
    {
      return nullptr;
    }

    const auto &token = tokens.Consume();
    tokens.DiscardWhitespace();

    return CSSPrimitiveValue::Create(token.ValueId());
  }

  template <typename Predicate, typename... Args>
  KRYS_NODISCARD Maybe<CSSValueId> ConsumeIdentRaw(TokenRange &tokens, Predicate &&predicate,
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
  KRYS_NODISCARD RefPtr<CSSPrimitiveValue> ConsumeIdent(TokenRange &tokens, Predicate &&predicate,
                                                        Args &&...args) noexcept
  {
    if (auto keyword = tokens.Peek().ValueId(); predicate(keyword, std::forward<Args>(args)...))
    {
      tokens.Discard();
      tokens.DiscardWhitespace();

      return CSSPrimitiveValue::Create(keyword);
    }

    return nullptr;
  }

#pragma endregion

#pragma region <custom-ident> - https://drafts.csswg.org/css-values/#custom-idents

  KRYS_NODISCARD Maybe<CSSOMString> ConsumeCustomIdentRaw(TokenRange &tokens,
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

  KRYS_NODISCARD RefPtr<CSSPrimitiveValue> ConsumeCustomIdent(TokenRange &tokens,
                                                              bool shouldLowercase = false) noexcept
  {
    auto identifier = ConsumeCustomIdentRaw(tokens, shouldLowercase);
    if (!identifier.has_value())
    {
      return nullptr;
    }

    return CSSPrimitiveValue::Create(krys::move(*identifier));
  }

  KRYS_NODISCARD RefPtr<CSSPrimitiveValue>
    ConsumeCustomIdentExcluding(TokenRange &tokens, std::initializer_list<const CSSValueId> excluding,
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

  KRYS_NODISCARD Maybe<CSSOMString> ConsumeDashedIdentRaw(TokenRange &tokens,
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

  RefPtr<CSSPrimitiveValue> ConsumeDashedIdent(TokenRange &tokens, bool shouldLowercase = false) noexcept
  {
    auto identifier = ConsumeDashedIdentRaw(tokens, shouldLowercase);
    if (!identifier.has_value())
    {
      return nullptr;
    }

    return CSSPrimitiveValue::Create(krys::move(*identifier));
  }

#pragma endregion
}