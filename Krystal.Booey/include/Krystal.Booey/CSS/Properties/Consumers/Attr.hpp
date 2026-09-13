#pragma once

#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class TokenRange;
  class CSSValue;
  struct PropertyParserState;

  namespace PropertyParserHelpers
  {
    /// @grammar <attr()> consuming
    /// @see https://drafts.csswg.org/css-values-5/#funcdef-attr
    KRYS_NODISCARD RefPtr<CSSValue> ConsumeAttr(TokenRange args, PropertyParserState &state) noexcept;
  }
}