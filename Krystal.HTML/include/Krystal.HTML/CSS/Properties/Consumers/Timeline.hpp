#pragma once

#include "Krystal.HTML/CSS/Types/CSSOMString.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"

namespace Krys::HTML
{
  class CSSTokenRange;
  struct CSSPropertyParserState;
  struct CSSParserContext;
  class CSSValue;

  namespace Style
  {
    enum class SingleAnimationRangeType : bool;
  }

  namespace CSSPropertyParserHelpers
  {
    KRYS_NODISCARD bool IsAnimationRangeKeyword(CSSValueId) noexcept;

    // MARK: - Consumer functions

    // <scroll()> = scroll( [ <scroller> || <axis> ]? )
    // https://drafts.csswg.org/scroll-animations-1/#scroll-notation
    KRYS_NODISCARD RefPtr<CSSValue> ConsumeAnimationTimelineScroll(CSSTokenRange &tokens,
                                                                   CSSPropertyParserState &state) noexcept;

    // <view()> = view( [ <axis> || <'view-timeline-inset'> ]? )
    // https://drafts.csswg.org/scroll-animations-1/#view-notation
    KRYS_NODISCARD RefPtr<CSSValue> ConsumeAnimationTimelineView(CSSTokenRange &tokens,
                                                                 CSSPropertyParserState &state) noexcept;

    // <single-view-timeline-inset-item> = <single-view-timeline-inset>{1,2}
    // https://drafts.csswg.org/scroll-animations-1/#propdef-view-timeline-inset
    KRYS_NODISCARD RefPtr<CSSValue>
      ConsumeSingleViewTimelineInsetItem(CSSTokenRange &tokens, CSSPropertyParserState &state) noexcept;

    KRYS_NODISCARD RefPtr<CSSValue>
      ParseSingleViewTimelineInsetItem(const CSSOMString &input, const CSSParserContext &context) noexcept;

    // <single-animation-range> = normal | <length-percentage> | <timeline-range-name> <length-percentage>?
    // https://drafts.csswg.org/scroll-animations-1/#propdef-animation-range-start
    KRYS_NODISCARD RefPtr<CSSValue>
      ConsumeSingleAnimationRange(CSSTokenRange &tokens, CSSPropertyParserState &state,
                                  Style::SingleAnimationRangeType type) noexcept;

    KRYS_NODISCARD RefPtr<CSSValue> ConsumeSingleAnimationRangeStart(CSSTokenRange &tokens,
                                                                     CSSPropertyParserState &state) noexcept;

    KRYS_NODISCARD RefPtr<CSSValue> ConsumeSingleAnimationRangeEnd(CSSTokenRange &tokens,
                                                                   CSSPropertyParserState &state) noexcept;

    KRYS_NODISCARD RefPtr<CSSValue> ParseSingleAnimationRange(const CSSOMString &input,
                                                              const CSSParserContext &context,
                                                              Style::SingleAnimationRangeType type) noexcept;
  }
}