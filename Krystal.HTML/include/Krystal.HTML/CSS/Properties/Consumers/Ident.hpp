#pragma once

#include "Krystal.HTML/CSS/Parser/CSSParserIdioms.hpp"
#include "Krystal.HTML/CSS/Parser/CSSToken.hpp"
#include "Krystal.HTML/CSS/Parser/CSSTokenRange.hpp"
#include "Krystal.HTML/CSS/Values/CSSPrimitiveValue.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include "Krystal.Lib/Types/Maybe.hpp"
#include "Krystal.Text/ASCII.hpp"
#include <ranges>

namespace Krys::HTML::CSSPropertyParserHelpers
{
#pragma region <ident>

  KRYS_NODISCARD Maybe<CSSValueId> ConsumeIdentRaw(CSSTokenRange &tokens) noexcept
  {
    if (tokens.Peek().Type() != CSSTokenType::Ident)
    {
      return Null;
    }

    const auto &token = tokens.Consume();
    tokens.DiscardWhitespace();

    return token.ValueId();
  }

  KRYS_NODISCARD RefPtr<CSSPrimitiveValue> ConsumeIdent(CSSTokenRange &tokens) noexcept
  {
    if (auto result = ConsumeIdentRaw(tokens))
    {
      return CSSPrimitiveValue::Create(*result);
    }

    return nullptr;
  }

  void DiscardIdent(CSSTokenRange &tokens) noexcept
  {
    if (tokens.Peek().Type() == CSSTokenType::Ident)
    {
      tokens.Discard();
      tokens.DiscardWhitespace();
    }
  }

  KRYS_NODISCARD Maybe<CSSValueId> ConsumeIdentRangeRaw(CSSTokenRange &tokens, CSSValueId lower,
                                                        CSSValueId upper) noexcept
  {
    if (tokens.Peek().ValueId() < lower || tokens.Peek().ValueId() > upper)
    {
      return Null;
    }

    return ConsumeIdentRaw(tokens);
  }

  KRYS_NODISCARD RefPtr<CSSPrimitiveValue> ConsumeIdentRange(CSSTokenRange &tokens, CSSValueId lower,
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
  KRYS_NODISCARD Maybe<CSSValueId> ConsumeIdentRaw(CSSTokenRange &tokens) noexcept
  {
    if (tokens.Peek().Type() != CSSTokenType::Ident || !IdentMatches<Names...>(tokens.Peek().ValueId()))
    {
      return Null;
    }

    const auto &token = tokens.Consume();
    tokens.DiscardWhitespace();

    return token.ValueId();
  }

  template <CSSValueId... Names>
  KRYS_NODISCARD RefPtr<CSSPrimitiveValue> ConsumeIdent(CSSTokenRange &tokens) noexcept
  {
    if (tokens.Peek().Type() != CSSTokenType::Ident || !IdentMatches<Names...>(tokens.Peek().ValueId()))
    {
      return nullptr;
    }

    const auto &token = tokens.Consume();
    tokens.DiscardWhitespace();

    return CSSPrimitiveValue::Create(token.ValueId());
  }

  template <typename Predicate, typename... Args>
  KRYS_NODISCARD Maybe<CSSValueId> ConsumeIdentRaw(CSSTokenRange &tokens, Predicate &&predicate,
                                                   Args &&...args) noexcept
  {
    if (auto keyword = tokens.Peek().ValueId(); predicate(keyword, std::forward<Args>(args)...))
    {
      tokens.Discard();
      tokens.DiscardWhitespace();

      return keyword;
    }

    return Null;
  }

  template <typename Predicate, typename... Args>
  KRYS_NODISCARD RefPtr<CSSPrimitiveValue> ConsumeIdent(CSSTokenRange &tokens, Predicate &&predicate,
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

  KRYS_NODISCARD Maybe<CSSOMString> ConsumeCustomIdentRaw(CSSTokenRange &tokens,
                                                          bool shouldLowercase = false) noexcept
  {
    if (tokens.Peek().Type() != CSSTokenType::Ident
        || !CSSParserIdioms::IsValidCustomIdentifier(tokens.Peek().ValueId()))
    {
      return Null;
    }

    auto &token = tokens.Consume();
    tokens.DiscardWhitespace();

    auto identifier = token.IdentCodePoints();

    return shouldLowercase ? Krys::Text::ToASCIILowercase(identifier) : CSSOMString(identifier);
  }

  KRYS_NODISCARD RefPtr<CSSPrimitiveValue> ConsumeCustomIdent(CSSTokenRange &tokens,
                                                              bool shouldLowercase = false) noexcept
  {
    auto identifier = ConsumeCustomIdentRaw(tokens, shouldLowercase);
    if (!identifier.has_value())
    {
      return nullptr;
    }

    return CSSPrimitiveValue::Create(Krys::Move(*identifier));
  }

  KRYS_NODISCARD RefPtr<CSSPrimitiveValue>
    ConsumeCustomIdentExcluding(CSSTokenRange &tokens, std::initializer_list<const CSSValueId> excluding,
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

  KRYS_NODISCARD Maybe<CSSOMString> ConsumeDashedIdentRaw(CSSTokenRange &tokens,
                                                          bool shouldLowercase = false) noexcept
  {
    auto tokensCopy = tokens;

    auto identifier = ConsumeCustomIdentRaw(tokens, shouldLowercase);
    if (!identifier->starts_with(u8"--"))
    {
      tokens = tokensCopy;
      return Null;
    }

    return identifier;
  }

  RefPtr<CSSPrimitiveValue> ConsumeDashedIdent(CSSTokenRange &tokens, bool shouldLowercase = false) noexcept
  {
    auto identifier = ConsumeDashedIdentRaw(tokens, shouldLowercase);
    if (!identifier.has_value())
    {
      return nullptr;
    }

    return CSSPrimitiveValue::Create(Krys::Move(*identifier));
  }

#pragma endregion
}