#pragma once

#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class TokenRange;
  struct CSSPropertyParserState;
  class CSSValue;

  namespace CSSPropertyParserHelpers
  {
    // MARK: <'scrollbar-color'> consuming
    // https://drafts.csswg.org/css-scrollbars/#propdef-scrollbar-color
    KRYS_NODISCARD RefPtr<CSSValue> ConsumeScrollbarColor(TokenRange &tokens,
                                                          CSSPropertyParserState &state) noexcept;
  }
}