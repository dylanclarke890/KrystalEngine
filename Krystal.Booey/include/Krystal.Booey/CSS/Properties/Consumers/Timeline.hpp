#pragma once

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class TokenRange;
  struct PropertyParserState;
  struct ParserContext;
  class Value;

  enum class ValueId : uint16;

  enum class SingleAnimationRangeType : bool
  {
    Start,
    End
  };

  namespace PropertyParserHelpers
  {
    KRYS_NODISCARD bool IsAnimationRangeKeyword(ValueId id) noexcept;

    // MARK: - Consumer functions

    // <scroll()> = scroll( [ <scroller> || <axis> ]? )
    // https://drafts.csswg.org/scroll-animations-1/#scroll-notation
    KRYS_NODISCARD RefPtr<Value> ConsumeAnimationTimelineScroll(TokenRange &tokens,
                                                                   PropertyParserState &state) noexcept;

    // <view()> = view( [ <axis> || <'view-timeline-inset'> ]? )
    // https://drafts.csswg.org/scroll-animations-1/#view-notation
    KRYS_NODISCARD RefPtr<Value> ConsumeAnimationTimelineView(TokenRange &tokens,
                                                                 PropertyParserState &state) noexcept;

    // <single-view-timeline-inset-item> = <single-view-timeline-inset>{1,2}
    // https://drafts.csswg.org/scroll-animations-1/#propdef-view-timeline-inset
    KRYS_NODISCARD RefPtr<Value>
      ConsumeSingleViewTimelineInsetItem(TokenRange &tokens, PropertyParserState &state) noexcept;

    KRYS_NODISCARD RefPtr<Value>
      ParseSingleViewTimelineInsetItem(const CSSOMString &input, const ParserContext &context) noexcept;

    // <single-animation-range> = normal | <length-percentage> | <timeline-range-name> <length-percentage>?
    // https://drafts.csswg.org/scroll-animations-1/#propdef-animation-range-start
    KRYS_NODISCARD RefPtr<Value> ConsumeSingleAnimationRange(TokenRange &tokens,
                                                                PropertyParserState &state,
                                                                SingleAnimationRangeType type) noexcept;

    KRYS_NODISCARD RefPtr<Value> ConsumeSingleAnimationRangeStart(TokenRange &tokens,
                                                                     PropertyParserState &state) noexcept;

    KRYS_NODISCARD RefPtr<Value> ConsumeSingleAnimationRangeEnd(TokenRange &tokens,
                                                                   PropertyParserState &state) noexcept;

    KRYS_NODISCARD RefPtr<Value> ParseSingleAnimationRange(const CSSOMString &input,
                                                              const ParserContext &context,
                                                              SingleAnimationRangeType type) noexcept;
  }
}