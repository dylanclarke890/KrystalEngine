#pragma once

#include "Krystal.HTML/CSS/Types/CSSOMString.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include "Krystal.Lib/Types/Maybe.hpp"

namespace Krys::HTML
{
  class CSSTokenRange;
  class CSSValue;

  struct ColorScheme;
  struct CSSParserContext;
  struct CSSPropertyParserState;

  // <'color-scheme'> = normal | [ light | dark | <custom-ident> ]+ && only?
  // https://drafts.csswg.org/css-color-adjust/#propdef-color-scheme

  // MARK: <'color-scheme'> consuming (unresolved)
  KRYS_NODISCARD Maybe<ColorScheme> ConsumeUnresolvedColorScheme(CSSTokenRange &tokens,
                                                                 CSSPropertyParserState &state) noexcept;

  // MARK: <'color-scheme'> parsing (unresolved)
  KRYS_NODISCARD Maybe<ColorScheme> ParseUnresolvedColorScheme(const CSSOMString &scheme,
                                                               const CSSParserContext &context) noexcept;

  // MARK: <'color-scheme'> consuming (CSSValue)
  KRYS_NODISCARD RefPtr<CSSValue> ConsumeColorScheme(CSSTokenRange &tokens,
                                                     CSSPropertyParserState &state) noexcept;
}