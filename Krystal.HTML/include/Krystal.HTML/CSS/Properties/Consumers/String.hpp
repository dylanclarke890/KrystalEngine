#pragma once

#include "Krystal.HTML/CSS/Parser/CSSTokenRange.hpp"
#include "Krystal.HTML/CSS/Values/CSSPrimitiveValue.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include "Krystal.Lib/String/String.hpp"

namespace Krys::HTML
{
  // MARK: <string>
  // https://drafts.csswg.org/css-values/#strings

  KRYS_NODISCARD utf8_stringview ConsumeStringRaw(CSSTokenRange &tokens) noexcept
  {
    if (tokens.Peek().Type() != CSSTokenType::String)
    {
      return {};
    }

    auto &token = tokens.Consume();
    tokens.DiscardWhitespace();

    return token.IdentCodePoints();
  }

  KRYS_NODISCARD RefPtr<CSSPrimitiveValue> ConsumeString(CSSTokenRange &tokens) noexcept
  {
    if (tokens.Peek().Type() != CSSTokenType::String)
    {
      return nullptr;
    }

    auto &token = tokens.Consume();
    tokens.DiscardWhitespace();

    return CSSPrimitiveValue::Create(CSSOMString(token.IdentCodePoints()));
  }
}