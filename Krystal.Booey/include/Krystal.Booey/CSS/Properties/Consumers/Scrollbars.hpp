#pragma once

#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class TokenRange;
  struct PropertyParserState;
  class CSSValue;

  namespace PropertyParserHelpers
  {
    // MARK: <'scrollbar-color'> consuming
    // https://drafts.csswg.org/css-scrollbars/#propdef-scrollbar-color
    KRYS_NODISCARD RefPtr<CSSValue> ConsumeScrollbarColor(TokenRange &tokens,
                                                          PropertyParserState &state) noexcept;
  }
}