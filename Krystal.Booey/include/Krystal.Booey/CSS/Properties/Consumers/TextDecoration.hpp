#pragma once

#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class TokenRange;
  class CSSValue;
  struct PropertyParserState;

  namespace PropertyParserHelpers
  {
    // MARK: <'text-shadow'> consuming
    // https://drafts.csswg.org/css-text-decor-3/#propdef-text-shadow
    KRYS_NODISCARD RefPtr<CSSValue> ConsumeTextShadow(TokenRange &tokens,
                                                      PropertyParserState &state) noexcept;
  }
}