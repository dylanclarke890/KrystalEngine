#pragma once

#include "Krystal.Lib/Pointers/RefPtr.hpp"

namespace Krys::HTML
{
  class CSSTokenRange;
  struct CSSPropertyParserState;
  class CSSValue;

  namespace CSSPropertyParserHelpers
  {
    // MARK: <'scroll-snap-type'> consuming
    // https://drafts.csswg.org/css-scroll-snap-1/#scroll-snap-type
    KRYS_NODISCARD RefPtr<CSSValue> ConsumeScrollSnapType(CSSTokenRange &tokens,
                                                          CSSPropertyParserState &state) noexcept;
  }
}