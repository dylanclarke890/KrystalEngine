#pragma once

#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class TokenRange;
  struct PropertyParserState;
  class CSSValue;

  namespace PropertyParserHelpers
  {
    // MARK: <'scroll-snap-type'> consuming
    // https://drafts.csswg.org/css-scroll-snap-1/#scroll-snap-type
    KRYS_NODISCARD RefPtr<CSSValue> ConsumeScrollSnapType(TokenRange &tokens,
                                                          PropertyParserState &state) noexcept;
  }
}