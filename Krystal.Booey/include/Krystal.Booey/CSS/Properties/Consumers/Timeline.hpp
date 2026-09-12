#pragma once

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class TokenRange;
  struct CSSPropertyParserState;
  struct ParserContext;
  class CSSValue;

  enum class CSSValueId : uint16;

  enum class SingleAnimationRangeType : bool
  {
    Start,
    End
  };

  namespace CSSPropertyParserHelpers
  {
    KRYS_NODISCARD bool IsAnimationRangeKeyword(CSSValueId id) noexcept;

    // MARK: - Consumer functions

    // <scroll()> = scroll( [ <scroller> || <axis> ]? )
    // https://drafts.csswg.org/scroll-animations-1/#scroll-notation
    KRYS_NODISCARD RefPtr<CSSValue> ConsumeAnimationTimelineScroll(TokenRange &tokens,
                                                                   CSSPropertyParserState &state) noexcept;

    // <view()> = view( [ <axis> || <'view-timeline-inset'> ]? )
    // https://drafts.csswg.org/scroll-animations-1/#view-notation
    KRYS_NODISCARD RefPtr<CSSValue> ConsumeAnimationTimelineView(TokenRange &tokens,
                                                                 CSSPropertyParserState &state) noexcept;

    // <single-view-timeline-inset-item> = <single-view-timeline-inset>{1,2}
    // https://drafts.csswg.org/scroll-animations-1/#propdef-view-timeline-inset
    KRYS_NODISCARD RefPtr<CSSValue>
      ConsumeSingleViewTimelineInsetItem(TokenRange &tokens, CSSPropertyParserState &state) noexcept;

    KRYS_NODISCARD RefPtr<CSSValue>
      ParseSingleViewTimelineInsetItem(const CSSOMString &input, const ParserContext &context) noexcept;

    // <single-animation-range> = normal | <length-percentage> | <timeline-range-name> <length-percentage>?
    // https://drafts.csswg.org/scroll-animations-1/#propdef-animation-range-start
    KRYS_NODISCARD RefPtr<CSSValue> ConsumeSingleAnimationRange(TokenRange &tokens,
                                                                CSSPropertyParserState &state,
                                                                SingleAnimationRangeType type) noexcept;

    KRYS_NODISCARD RefPtr<CSSValue> ConsumeSingleAnimationRangeStart(TokenRange &tokens,
                                                                     CSSPropertyParserState &state) noexcept;

    KRYS_NODISCARD RefPtr<CSSValue> ConsumeSingleAnimationRangeEnd(TokenRange &tokens,
                                                                   CSSPropertyParserState &state) noexcept;

    KRYS_NODISCARD RefPtr<CSSValue> ParseSingleAnimationRange(const CSSOMString &input,
                                                              const ParserContext &context,
                                                              SingleAnimationRangeType type) noexcept;
  }
}