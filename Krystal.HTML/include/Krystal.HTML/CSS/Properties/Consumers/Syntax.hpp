#pragma once

#include "Krystal.Lib/Pointers/RefPtr.hpp"

namespace Krys::HTML
{
  class CSSTokenRange;
  class CSSValue;
  struct CSSPropertyParserState;

  namespace CSSPropertyParserHelpers
  {
    // MARK: <declaration-value> consuming
    // https://drafts.csswg.org/css-syntax-3/#typedef-declaration-value
    KRYS_NODISCARD RefPtr<CSSValue> ConsumeDeclarationValue(CSSTokenRange &tokens,
                                                            CSSPropertyParserState &state) noexcept;
  }
}