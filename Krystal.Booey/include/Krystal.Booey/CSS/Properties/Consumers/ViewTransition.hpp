#pragma once

#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class TokenRange;
  class Value;
  struct PropertyParserState;

  namespace PropertyParserHelpers
  {
    // For @view-transition descriptor
    KRYS_NODISCARD RefPtr<Value> ConsumeViewTransitionTypes(TokenRange &tokens,
                                                               PropertyParserState &state) noexcept;
  }
}