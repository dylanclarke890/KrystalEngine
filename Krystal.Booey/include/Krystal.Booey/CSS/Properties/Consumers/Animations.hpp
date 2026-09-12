#pragma once

#include "Krystal.Booey/CSS/Values/Enums/CSSValueId.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/SmallList.hpp"
#include "Krystal.Core/Types/Pair.hpp"

namespace krys::boo::css
{
  class TokenRange;
  class CSSValue;
  struct CSSPropertyParserState;

  namespace CSSPropertyParserHelpers
  {
    /// @grammar <keyframe-selector>
    /// @see https://drafts.csswg.org/css-animations-1/#typedef-keyframe-selector
    SmallList<Pair<CSSValueId, double>> ConsumeKeyframeKeyList(TokenRange &tokens,
                                                                 CSSPropertyParserState &state)noexcept;

    /// @grammar <keyframe-selector>
    /// @see https://drafts.csswg.org/css-animations-1/#typedef-keyframe-selector
    SmallList<Pair<CSSValueId, double>> ParseKeyframeKeyList(const CSSOMString &str, const ParserContext &context) noexcept;

    /// @grammar <keyframes-name>
    /// @see https://drafts.csswg.org/css-animations/#typedef-keyframes-name
    RefPtr<CSSValue> ConsumeKeyframesName(TokenRange &tokens, CSSPropertyParserState &state)noexcept;
  }
}