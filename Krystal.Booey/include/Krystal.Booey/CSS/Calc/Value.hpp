#pragma once

#include "Krystal.Booey/CSS/Calc/CalcAllowedSymbols.hpp"
#include "Krystal.Booey/CSS/Parser/Token.hpp"
#include "Krystal.Booey/CSS/Parser/TokenRange.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyParserOptions.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyParserState.hpp"
#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/Range.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/Units.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css::calc
{
  class Value : public RefCounted<Value>
  {
  public:
    KRYS_NODISCARD static RefPtr<Value> Parse(TokenRange &tokens, PropertyParserState &state,
                                                 NumericTypeCategory category, Range range,
                                                 CalcAllowedSymbols symbolsAllowed,
                                                 PropertyParserOptions options) noexcept
    {
      (void)tokens;
      (void)state;
      (void)category;
      (void)range;
      (void)symbolsAllowed;
      (void)options;

      krys_not_implemented();
    }
  };
}