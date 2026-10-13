#pragma once

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/CSS/Values/ValueId.hpp"
#include "Krystal.Core/Types/Pair.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/SmallList.hpp"

namespace krys::boo::css
{
  class TokenRange;
  class Value;
  struct ParserContext;
  struct PropertyParserState;

  namespace PropertyParserHelpers
  {
    /// @grammar <keyframe-selector>
    /// @see https://drafts.csswg.org/css-animations-1/#typedef-keyframe-selector
    KRYS_NODISCARD SmallList<Pair<ValueId, double>>
      ConsumeKeyframeKeyList(TokenRange &tokens, PropertyParserState &state) noexcept;

    /// @grammar <keyframe-selector>
    /// @see https://drafts.csswg.org/css-animations-1/#typedef-keyframe-selector
    KRYS_NODISCARD SmallList<Pair<ValueId, double>>
      ParseKeyframeKeyList(const CSSOMString &str, const ParserContext &context) noexcept;

    /// @grammar <keyframes-name>
    /// @see https://drafts.csswg.org/css-animations/#typedef-keyframes-name
    KRYS_NODISCARD RefPtr<Value> ConsumeKeyframesName(TokenRange &tokens,
                                                      PropertyParserState &state) noexcept;
  }
}