#pragma once

#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class TokenRange;
  class Value;
  struct PropertyParserState;

  namespace PropertyParserHelpers
  {
    /// @grammar <attr()> consuming
    /// @see https://drafts.csswg.org/css-values-5/#funcdef-attr
    KRYS_NODISCARD RefPtr<Value> ConsumeAttr(TokenRange args, PropertyParserState &state) noexcept;
  }
}