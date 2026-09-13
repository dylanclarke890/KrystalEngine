#pragma once

#include "Krystal.Booey/CSS/Parser/TokenRange.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyParserState.hpp"
#include "Krystal.Booey/CSS/Values/CSSPrimitiveValue.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css::PropertyParserHelpers
{
  // FIXME: Users of this function are likely getting incorrect results when used with calc() producing a
  // percent, as it is not getting divided by 100.
  KRYS_NODISCARD RefPtr<CSSPrimitiveValue>
    ConsumePercentageDividedBy100OrNumber(TokenRange &tokens, PropertyParserState &state) noexcept;
}