#pragma once

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class TokenRange;
  class Value;

  struct ColorScheme;
  struct ParserContext;
  struct PropertyParserState;

  namespace PropertyParserHelpers
  {
    // <'color-scheme'> = normal | [ light | dark | <custom-ident> ]+ && only?
    // https://drafts.csswg.org/css-color-adjust/#propdef-color-scheme

    // MARK: <'color-scheme'> consuming (unresolved)
    KRYS_NODISCARD Maybe<ColorScheme> ConsumeUnresolvedColorScheme(TokenRange &tokens,
                                                                   PropertyParserState &state) noexcept;

    // MARK: <'color-scheme'> parsing (unresolved)
    KRYS_NODISCARD Maybe<ColorScheme> ParseUnresolvedColorScheme(CSSOMStringView string,
                                                                 const ParserContext &context) noexcept;

    // MARK: <'color-scheme'> consuming (Value)
    KRYS_NODISCARD RefPtr<Value> ConsumeColorScheme(TokenRange &tokens, PropertyParserState &state) noexcept;
  }
}
