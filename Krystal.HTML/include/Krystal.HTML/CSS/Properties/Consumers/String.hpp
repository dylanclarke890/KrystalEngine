#pragma once

#include "Krystal.HTML/CSS/Types/CSSOMString.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"

namespace Krys::HTML
{
  class CSSTokenRange;
  class CSSPrimitiveValue;

  // MARK: <string>
  // https://drafts.csswg.org/css-values/#strings

  namespace CSSPropertyParserHelpers
  {
    KRYS_NODISCARD CSSOMStringView ConsumeStringRaw(CSSTokenRange &tokens) noexcept;

    KRYS_NODISCARD RefPtr<CSSPrimitiveValue> ConsumeString(CSSTokenRange &tokens) noexcept;
  }
}