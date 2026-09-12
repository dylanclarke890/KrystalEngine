#pragma once

#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class TokenRange;
  struct CSSPropertyParserState;
  class CSSValue;

  namespace CSSPropertyParserHelpers
  {
    // MARK: <'scroll-snap-type'> consuming
    // https://drafts.csswg.org/css-scroll-snap-1/#scroll-snap-type
    KRYS_NODISCARD RefPtr<CSSValue> ConsumeScrollSnapType(TokenRange &tokens,
                                                          CSSPropertyParserState &state) noexcept;
  }
}