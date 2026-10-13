#pragma once

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/CSS/Values/Color/ColorType.hpp"
#include "Krystal.Booey/CSS/Values/ValueId.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  class Color;
}

namespace krys::boo::css
{
  class Color;
  class TokenRange;
  class Value;

  struct DynamicRangeLimit;
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
    KRYS_NODISCARD bool IsColorKeywordAllowed(ValueId id, const ParserContext &context) noexcept;

    // MARK: <color> consuming (unresolved)
    KRYS_NODISCARD Maybe<Color> ConsumeUnresolvedColor(TokenRange &tokens, PropertyParserState &state,
                                                       const ColorParsingOptions &options = {}) noexcept;

    // MARK: <color> consuming (Value)
    KRYS_NODISCARD RefPtr<Value> ConsumeColor(TokenRange &tokens, PropertyParserState &state,
                                              const ColorParsingOptions &options = {}) noexcept;

    // MARK: <color> consuming (raw)
    KRYS_NODISCARD boo::Color ConsumeColorRaw(TokenRange &tokens, PropertyParserState &state,
                                              const ColorParsingOptions &options,
                                              PlatformColorResolutionState &eagerResolutionState) noexcept;

    // MARK: <color> parsing (raw)

    /// @brief Parse with default options.
    /// @note Callers must include `ColorInlines.hpp` to use this.
    KRYS_NODISCARD boo::Color ParseColorRaw(CSSOMStringView string, const ParserContext &context) noexcept;

    /// @brief Fast variant to be used when you need to pass parsing options. If the result is invalid,
    /// callers should call `ParseColorRawGeneral()`.
    /// @note Callers must include `ColorInlines.hpp` to use this.
    KRYS_NODISCARD boo::Color ParseColorRawSimple(CSSOMStringView string,
                                                  const ParserContext &context) noexcept;

    /// @brief Parse with specific options.
    KRYS_NODISCARD boo::Color
      ParseColorRawGeneral(CSSOMStringView string, const ParserContext &context,
                           const ColorParsingOptions &options,
                           PlatformColorResolutionState &eagerResolutionState) noexcept;

    // FIXME: All callers are not getting the right Settings, keyword resolution and calc resolution
    // when using this function and should switch to parseColorRaw().
    KRYS_NODISCARD boo::Color
      DeprecatedParseColorRawWithoutContext(CSSOMStringView string,
                                            const ColorParsingOptions &options = {}) noexcept;

    // MARK: <dynamic-range-limit> (unresolved)
    KRYS_NODISCARD Maybe<DynamicRangeLimit>
      ConsumeUnresolvedDynamicRangeLimit(TokenRange &tokens, PropertyParserState &state) noexcept;

    // MARK: <dynamic-range-limit> (Value)
    KRYS_NODISCARD RefPtr<Value> ConsumeDynamicRangeLimit(TokenRange &tokens,
                                                          PropertyParserState &state) noexcept;
  }
}
