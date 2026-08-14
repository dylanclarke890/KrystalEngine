#pragma once

#include "Krystal.HTML/CSS/Calc/CSSCalcAllowedSymbols.hpp"
#include "Krystal.HTML/CSS/Parser/CSSToken.hpp"
#include "Krystal.HTML/CSS/Parser/CSSTokenRange.hpp"
#include "Krystal.HTML/CSS/Properties/CSSPropertyParserOptions.hpp"
#include "Krystal.HTML/CSS/Properties/CSSPropertyParserState.hpp"
#include "Krystal.HTML/CSS/Values/CSSValue.hpp"
#include "Krystal.HTML/CSS/Values/Primitives/CSSPrimitiveNumericRange.hpp"
#include "Krystal.HTML/CSS/Values/Primitives/CSSPrimitiveNumericUnits.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"

namespace Krys::HTML
{
  class CSSCalcValue
  {
  public:
    KRYS_NODISCARD static RefPtr<CSSValue> Parse(CSSTokenRange &tokens, CSSPropertyParserState &state,
                                                 NumericTypeCategory category, CSSRange range,
                                                 CSSCalcAllowedSymbols symbolsAllowed,
                                                 CSSPropertyParserOptions options) noexcept
    {
      return nullptr;
    }
  };
}