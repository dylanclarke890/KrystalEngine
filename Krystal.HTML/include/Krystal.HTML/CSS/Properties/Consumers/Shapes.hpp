#pragma once

#include "Krystal.Lib/Core/Enum.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"

namespace Krys::HTML
{
  class CSSTokenRange;
  class CSSValue;
  struct CSSPropertyParserState;

  namespace CSSPropertyParserHelpers
  {
    enum class PathParsingOption : uint8_t
    {
      None = 0,
      RejectPathFillRule = 1 << 0,
      RejectPath = 1 << 1,
    };

    // <basic-shape> = <circle()> | <ellipse() | <inset()> | <path()> | <polygon()> | <rect()> | <shape()> |
    // <xywh()> https://drafts.csswg.org/css-shapes/#typedef-basic-shape
    KRYS_NODISCARD RefPtr<CSSValue> ConsumeBasicShape(CSSTokenRange &tokens, CSSPropertyParserState &state,
                                                      PathParsingOption pathParsingOptions) noexcept;

    // <path()> = path( <'fill-rule'>? , <string> )
    // https://drafts.csswg.org/css-shapes/#funcdef-basic-shape-path
    KRYS_NODISCARD RefPtr<CSSValue> ConsumePath(CSSTokenRange &tokens,
                                                CSSPropertyParserState &state) noexcept;

    // <'shape-outside'> = none | [ <basic-shape> || <shape-box> ] | <image>
    // https://drafts.csswg.org/css-shapes/#propdef-shape-outside
    KRYS_NODISCARD RefPtr<CSSValue> ConsumeShapeOutside(CSSTokenRange &tokens,
                                                        CSSPropertyParserState &state) noexcept;
  }
}

KRYS_DEFINE_FLAGS_ENUM_TRAITS(Krys::HTML::CSSPropertyParserHelpers::PathParsingOption, 3uz);