#pragma once

#include "Krystal.HTML/CSS/Values/Enums/CSSValueId.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include "Krystal.Lib/Types/SmallList.hpp"
#include "Krystal.Lib/Types/Pair.hpp"

namespace Krys::HTML
{
  class CSSTokenRange;
  class CSSValue;
  struct CSSPropertyParserState;

  namespace CSSPropertyParserHelpers
  {
    /// @grammar <keyframe-selector>
    /// @see https://drafts.csswg.org/css-animations-1/#typedef-keyframe-selector
    SmallList<Pair<CSSValueId, double>> ConsumeKeyframeKeyList(CSSTokenRange &tokens,
                                                                 CSSPropertyParserState &state)noexcept;

    /// @grammar <keyframe-selector>
    /// @see https://drafts.csswg.org/css-animations-1/#typedef-keyframe-selector
    SmallList<Pair<CSSValueId, double>> ParseKeyframeKeyList(const CSSOMString &str, const CSSParserContext &context) noexcept;

    /// @grammar <keyframes-name>
    /// @see https://drafts.csswg.org/css-animations/#typedef-keyframes-name
    RefPtr<CSSValue> ConsumeKeyframesName(CSSTokenRange &tokens, CSSPropertyParserState &state)noexcept;
  }
}