#pragma once

#include "Krystal.HTML/CSS/Values/Enums/ColorType.hpp"
#include "Krystal.HTML/CSS/Values/Enums/CSSValueId.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include "Krystal.Lib/Types/Maybe.hpp"

namespace Krys::HTML
{
  class Color;
  class CSSTokenRange;
  class CSSValue;
  class DynamicRangeLimit;
  class PlatformColorResolutionState;

  struct CSSParserContext;
  struct CSSPropertyParserState;

  namespace CSSPropertyParserHelpers
  {
    // Options to augment color parsing.
    struct CSSColorParsingOptions
    {
      ColorType AllowedColorTypes = ColorType::Absolute | ColorType::Current | ColorType::System;
    };

    // MARK: Mode specific color settings.
    KRYS_NODISCARD bool IsColorKeywordAllowed(CSSValueId keyword, const CSSParserContext &context) noexcept;

    // MARK: <color> consuming (unresolved)
    KRYS_NODISCARD Maybe<Color> ConsumeUnresolvedColor(CSSTokenRange &tokens, CSSPropertyParserState &state,
                                                       const CSSColorParsingOptions & = {}) noexcept;

    // MARK: <color> consuming (CSSValue)
    KRYS_NODISCARD RefPtr<CSSValue> ConsumeColor(CSSTokenRange &tokens, CSSPropertyParserState &state,
                                                 const CSSColorParsingOptions & = {}) noexcept;

    // MARK: <color> consuming (raw)
    KRYS_NODISCARD Color ConsumeColorRaw(CSSTokenRange &tokens, CSSPropertyParserState &state,
                                         const CSSColorParsingOptions &,
                                         PlatformColorResolutionState &) noexcept;

    // MARK: <dynamic-range-limit> (unresolved)
    KRYS_NODISCARD Maybe<DynamicRangeLimit>
      ConsumeUnresolvedDynamicRangeLimit(CSSTokenRange &tokens, CSSPropertyParserState &state) noexcept;

    // MARK: <dynamic-range-limit> (CSSValue)
    KRYS_NODISCARD RefPtr<CSSValue> ConsumeDynamicRangeLimit(CSSTokenRange &tokens,
                                                             CSSPropertyParserState &state) noexcept;
  }
}