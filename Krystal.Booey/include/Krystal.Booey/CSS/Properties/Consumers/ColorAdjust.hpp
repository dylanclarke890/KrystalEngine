#pragma once

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::css
{
  class TokenRange;
  class CSSValue;

  struct ColorScheme;
  struct ParserContext;
  struct PropertyParserState;

  // <'color-scheme'> = normal | [ light | dark | <custom-ident> ]+ && only?
  // https://drafts.csswg.org/css-color-adjust/#propdef-color-scheme

  // MARK: <'color-scheme'> consuming (unresolved)
  KRYS_NODISCARD Maybe<ColorScheme> ConsumeUnresolvedColorScheme(TokenRange &tokens,
                                                                 PropertyParserState &state) noexcept;

  // MARK: <'color-scheme'> parsing (unresolved)
  KRYS_NODISCARD Maybe<ColorScheme> ParseUnresolvedColorScheme(const CSSOMString &scheme,
                                                               const ParserContext &context) noexcept;

  // MARK: <'color-scheme'> consuming (CSSValue)
  KRYS_NODISCARD RefPtr<CSSValue> ConsumeColorScheme(TokenRange &tokens,
                                                     PropertyParserState &state) noexcept;
}