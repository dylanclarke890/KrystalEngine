#pragma once

#include "Krystal.Lib/Pointers/RefPtr.hpp"

namespace Krys::HTML
{
  class CSSTokenRange;
  struct CSSPropertyParserState;
  class CSSValue;

  namespace CSSPropertyParserHelpers
  {
    // <'offset-path'> = none | <offset-path> || <coord-box>
    // https://drafts.fxtf.org/motion-1/#propdef-offset-path
    KRYS_NODISCARD RefPtr<CSSValue> ConsumeOffsetPath(CSSTokenRange &tokens,
                                                      CSSPropertyParserState &state) noexcept;
  }
}