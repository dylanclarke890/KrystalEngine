#pragma once

#include "Krystal.Lib/Pointers/RefPtr.hpp"

namespace Krys::HTML
{
  class CSSTokenRange;
  struct CSSPropertyParserState;
  class CSSValue;

  namespace CSSPropertyParserHelpers
  {
    // MARK: <'scrollbar-color'> consuming
    // https://drafts.csswg.org/css-scrollbars/#propdef-scrollbar-color
    KRYS_NODISCARD RefPtr<CSSValue> ConsumeScrollbarColor(CSSTokenRange &tokens,
                                                          CSSPropertyParserState &state) noexcept;
  }
}