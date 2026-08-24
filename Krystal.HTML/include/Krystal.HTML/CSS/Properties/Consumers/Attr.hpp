#pragma once

#include "Krystal.Lib/Pointers/RefPtr.hpp"

namespace Krys::HTML
{
  class CSSTokenRange;
  class CSSValue;
  struct CSSPropertyParserState;

  namespace CSSPropertyParserHelpers
  {
    /// @grammar <attr()> consuming
    /// @see https://drafts.csswg.org/css-values-5/#funcdef-attr
    KRYS_NODISCARD RefPtr<CSSValue> ConsumeAttr(CSSTokenRange args, CSSPropertyParserState &state) noexcept;
  }
}