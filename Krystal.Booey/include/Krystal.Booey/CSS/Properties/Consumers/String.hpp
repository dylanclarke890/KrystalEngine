#pragma once

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class TokenRange;
  class CSSPrimitiveValue;

  // MARK: <string>
  // https://drafts.csswg.org/css-values/#strings

  namespace PropertyParserHelpers
  {
    KRYS_NODISCARD CSSOMStringView ConsumeStringRaw(TokenRange &tokens) noexcept;

    KRYS_NODISCARD RefPtr<CSSPrimitiveValue> ConsumeString(TokenRange &tokens) noexcept;
  }
}