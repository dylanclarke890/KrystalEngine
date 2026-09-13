#pragma once

#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class TokenRange;
  class CSSValue;
  struct PropertyParserState;

  namespace PropertyParserHelpers
  {
    // For @view-transition descriptor
    KRYS_NODISCARD RefPtr<CSSValue> ConsumeViewTransitionTypes(TokenRange &tokens,
                                                               PropertyParserState &state) noexcept;
  }
}