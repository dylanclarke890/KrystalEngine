#pragma once

#include "Krystal.HTML/CSS/Parser/CSSTokenRange.hpp"
#include "Krystal.HTML/CSS/Properties/CSSPropertyParserState.hpp"
#include "Krystal.HTML/CSS/Values/CSSPrimitiveValue.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"

namespace Krys::HTML
{
  // FIXME: Users of this function are likely getting incorrect results when used with calc() producing a
  // percent, as it is not getting divided by 100.
  KRYS_NODISCARD RefPtr<CSSPrimitiveValue>
    ConsumePercentageDividedBy100OrNumber(CSSTokenRange &tokens, CSSPropertyParserState &state) noexcept;
}