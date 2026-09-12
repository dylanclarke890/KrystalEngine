#pragma once

#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class TokenRange;
  struct CSSPropertyParserState;
  class CSSValue;

  namespace CSSPropertyParserHelpers
  {
    // <'offset-path'> = none | <offset-path> || <coord-box>
    // https://drafts.fxtf.org/motion-1/#propdef-offset-path
    KRYS_NODISCARD RefPtr<CSSValue> ConsumeOffsetPath(TokenRange &tokens,
                                                      CSSPropertyParserState &state) noexcept;
  }
}