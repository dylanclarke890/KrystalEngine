#pragma once

#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class TokenRange;
  class Value;
  struct PropertyParserState;

  namespace PropertyParserHelpers
  {
    // MARK: <'text-shadow'> consuming
    // https://drafts.csswg.org/css-text-decor-3/#propdef-text-shadow
    KRYS_NODISCARD RefPtr<Value> ConsumeTextShadow(TokenRange &tokens,
                                                      PropertyParserState &state) noexcept;
  }
}