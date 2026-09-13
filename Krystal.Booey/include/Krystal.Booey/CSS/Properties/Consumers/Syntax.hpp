#pragma once

#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class TokenRange;
  class CSSValue;
  struct PropertyParserState;

  namespace PropertyParserHelpers
  {
    // MARK: <declaration-value> consuming
    // https://drafts.csswg.org/css-syntax-3/#typedef-declaration-value
    KRYS_NODISCARD RefPtr<CSSValue> ConsumeDeclarationValue(TokenRange &tokens,
                                                            PropertyParserState &state) noexcept;
  }
}