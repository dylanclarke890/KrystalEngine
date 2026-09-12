#pragma once

#include "Krystal.Booey/CSS/Values/Color/ColorType.hpp"
#include "Krystal.Booey/CSS/Values/Enums/CSSValueId.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::css
{
  class Color;
  class TokenRange;
  class CSSValue;
  class DynamicRangeLimit;
  class PlatformColorResolutionState;

  struct ParserContext;
  struct CSSPropertyParserState;

  namespace CSSPropertyParserHelpers
  {
    // Options to augment color parsing.
    struct CSSColorParsingOptions
    {
      ColorType AllowedColorTypes = ColorType::Absolute | ColorType::Current | ColorType::System;
    };

    // MARK: Mode specific color settings.
    KRYS_NODISCARD bool IsColorKeywordAllowed(CSSValueId keyword, const ParserContext &context) noexcept;

    // MARK: <color> consuming (unresolved)
    KRYS_NODISCARD Maybe<Color> ConsumeUnresolvedColor(TokenRange &tokens, CSSPropertyParserState &state,
                                                       const CSSColorParsingOptions & = {}) noexcept;

    // MARK: <color> consuming (CSSValue)
    KRYS_NODISCARD RefPtr<CSSValue> ConsumeColor(TokenRange &tokens, CSSPropertyParserState &state,
                                                 const CSSColorParsingOptions & = {}) noexcept;

    // MARK: <color> consuming (raw)
    KRYS_NODISCARD Color ConsumeColorRaw(TokenRange &tokens, CSSPropertyParserState &state,
                                         const CSSColorParsingOptions &,
                                         PlatformColorResolutionState &) noexcept;

    // MARK: <dynamic-range-limit> (unresolved)
    KRYS_NODISCARD Maybe<DynamicRangeLimit>
      ConsumeUnresolvedDynamicRangeLimit(TokenRange &tokens, CSSPropertyParserState &state) noexcept;

    // MARK: <dynamic-range-limit> (CSSValue)
    KRYS_NODISCARD RefPtr<CSSValue> ConsumeDynamicRangeLimit(TokenRange &tokens,
                                                             CSSPropertyParserState &state) noexcept;
  }
}