#pragma once

#include "Krystal.Booey/CSS/Values/Color/ColorType.hpp"
#include "Krystal.Booey/CSS/Values/ValueId.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class Color;
  class TokenRange;
  class Value;
  class DynamicRangeLimit;

  struct ParserContext;
  struct PlatformColorResolutionState;
  struct PropertyParserState;

  namespace PropertyParserHelpers
  {
    // Options to augment color parsing.
    struct CSSColorParsingOptions
    {
      ColorType AllowedColorTypes = ColorType::Absolute | ColorType::Current | ColorType::System;
    };

    // MARK: Mode specific color settings.
    KRYS_NODISCARD bool IsColorKeywordAllowed(ValueId keyword, const ParserContext &context) noexcept;

    // MARK: <color> consuming (unresolved)
    KRYS_NODISCARD Maybe<Color> ConsumeUnresolvedColor(TokenRange &tokens, PropertyParserState &state,
                                                       const CSSColorParsingOptions & = {}) noexcept;

    // MARK: <color> consuming (Value)
    KRYS_NODISCARD RefPtr<Value> ConsumeColor(TokenRange &tokens, PropertyParserState &state,
                                              const CSSColorParsingOptions & = {}) noexcept;

    // MARK: <color> consuming (raw)
    KRYS_NODISCARD Color ConsumeColorRaw(TokenRange &tokens, PropertyParserState &state,
                                         const CSSColorParsingOptions &,
                                         PlatformColorResolutionState &) noexcept;

    // MARK: <dynamic-range-limit> (unresolved)
    KRYS_NODISCARD Maybe<DynamicRangeLimit>
      ConsumeUnresolvedDynamicRangeLimit(TokenRange &tokens, PropertyParserState &state) noexcept;

    // MARK: <dynamic-range-limit> (Value)
    KRYS_NODISCARD RefPtr<Value> ConsumeDynamicRangeLimit(TokenRange &tokens,
                                                          PropertyParserState &state) noexcept;
  }
}