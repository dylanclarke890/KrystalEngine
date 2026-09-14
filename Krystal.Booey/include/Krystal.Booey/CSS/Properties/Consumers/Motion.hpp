#pragma once

#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class TokenRange;
  struct PropertyParserState;
  class Value;

  namespace PropertyParserHelpers
  {
    // <'offset-path'> = none | <offset-path> || <coord-box>
    // https://drafts.fxtf.org/motion-1/#propdef-offset-path
    KRYS_NODISCARD RefPtr<Value> ConsumeOffsetPath(TokenRange &tokens,
                                                      PropertyParserState &state) noexcept;
  }
}