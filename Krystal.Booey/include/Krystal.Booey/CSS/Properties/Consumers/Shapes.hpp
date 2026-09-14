#pragma once

#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class TokenRange;
  class Value;
  struct PropertyParserState;

  namespace PropertyParserHelpers
  {
    enum class PathParsingOption : uint8_t
    {
      None = 0,
      RejectPathFillRule = 1 << 0,
      RejectPath = 1 << 1,
    };

    // <basic-shape> = <circle()> | <ellipse() | <inset()> | <path()> | <polygon()> | <rect()> | <shape()> |
    // <xywh()> https://drafts.csswg.org/css-shapes/#typedef-basic-shape
    KRYS_NODISCARD RefPtr<Value> ConsumeBasicShape(TokenRange &tokens, PropertyParserState &state,
                                                      PathParsingOption pathParsingOptions) noexcept;

    // <path()> = path( <'fill-rule'>? , <string> )
    // https://drafts.csswg.org/css-shapes/#funcdef-basic-shape-path
    KRYS_NODISCARD RefPtr<Value> ConsumePath(TokenRange &tokens, PropertyParserState &state) noexcept;

    // <'shape-outside'> = none | [ <basic-shape> || <shape-box> ] | <image>
    // https://drafts.csswg.org/css-shapes/#propdef-shape-outside
    KRYS_NODISCARD RefPtr<Value> ConsumeShapeOutside(TokenRange &tokens,
                                                        PropertyParserState &state) noexcept;
  }
}

KRYS_DEFINE_FLAGS_ENUM_TRAITS(krys::boo::css::PropertyParserHelpers::PathParsingOption, 3uz);