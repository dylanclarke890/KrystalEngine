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
    struct ColorParsingOptions
    {
      ColorType AllowedColorTypes = ColorType::Absolute | ColorType::Current | ColorType::System;
    };

    // MARK: Mode specific color settings.
    KRYS_NODISCARD bool IsColorKeywordAllowed(ValueId keyword, const ParserContext &context) noexcept;

    // MARK: <color> consuming (unresolved)
    KRYS_NODISCARD Maybe<Color> ConsumeUnresolvedColor(TokenRange &tokens, PropertyParserState &state,
                                                       const ColorParsingOptions & = {}) noexcept;

    // MARK: <color> consuming (Value)
    KRYS_NODISCARD RefPtr<Value> ConsumeColor(TokenRange &tokens, PropertyParserState &state,
                                              const ColorParsingOptions & = {}) noexcept;

    // MARK: <color> consuming (raw)
    KRYS_NODISCARD Color ConsumeColorRaw(TokenRange &tokens, PropertyParserState &state,
                                         const ColorParsingOptions &,
                                         PlatformColorResolutionState &) noexcept;

    // MARK: <dynamic-range-limit> (unresolved)
    KRYS_NODISCARD Maybe<DynamicRangeLimit>
      ConsumeUnresolvedDynamicRangeLimit(TokenRange &tokens, PropertyParserState &state) noexcept;

    // MARK: <dynamic-range-limit> (Value)
    KRYS_NODISCARD RefPtr<Value> ConsumeDynamicRangeLimit(TokenRange &tokens,
                                                          PropertyParserState &state) noexcept;

    // FIXME: All callers are not getting the right Settings, keyword resolution and calc resolution
    // when using this function and should switch to parseColorRaw().
    KRYS_NODISCARD boo::Color
      DeprecatedParseColorRawWithoutContext(const CSSOMString &str,
                                            const ColorParsingOptions & = {}) noexcept;
  }
}