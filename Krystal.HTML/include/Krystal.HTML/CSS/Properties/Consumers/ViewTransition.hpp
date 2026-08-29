#pragma once

#include "Krystal.Lib/Pointers/RefPtr.hpp"

namespace Krys::HTML
{
  class CSSTokenRange;
  class CSSValue;
  struct CSSPropertyParserState;

  namespace CSSPropertyParserHelpers
  {
    // For @view-transition descriptor
    KRYS_NODISCARD RefPtr<CSSValue> ConsumeViewTransitionTypes(CSSTokenRange &tokens,
                                                               CSSPropertyParserState &state) noexcept;
  }
}