#pragma once

#include "Krystal.Booey/CSS/Calc/CalcAllowedSymbols.hpp"
#include "Krystal.Booey/CSS/Parser/Token.hpp"
#include "Krystal.Booey/CSS/Parser/TokenRange.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyParserOptions.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyParserState.hpp"
#include "Krystal.Booey/CSS/Values/CSSValue.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/CSSPrimitiveNumericRange.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/CSSPrimitiveNumericUnits.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class CSSCalcValue
  {
  public:
    KRYS_NODISCARD static RefPtr<CSSValue> Parse(TokenRange &tokens, PropertyParserState &state,
                                                 NumericTypeCategory category, CSSRange range,
                                                 CalcAllowedSymbols symbolsAllowed,
                                                 PropertyParserOptions options) noexcept
    {
      return nullptr;
    }
  };
}