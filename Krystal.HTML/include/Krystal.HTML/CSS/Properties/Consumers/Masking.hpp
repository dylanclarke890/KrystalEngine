#pragma once

#include "Krystal.Lib/Pointers/RefPtr.hpp"

namespace Krys::HTML
{
  class CSSTokenRange;
  struct CSSPropertyParserState;
  class CSSValue;

  namespace CSSPropertyParserHelpers
  {
    // rect() = rect( [ <length> | auto ]#{4} | [ <length> | auto ]{4} )
    // https://drafts.fxtf.org/css-masking/#funcdef-clip-rect
    KRYS_NODISCARD RefPtr<CSSValue> ConsumeClipRectFunction(CSSTokenRange &tokens,
                                                            CSSPropertyParserState &state) noexcept;

    // <'clip-path'> = none | <clip-source> | [ <basic-shape> || <geometry-box> ]
    // https://drafts.fxtf.org/css-masking/#propdef-clip-path
    KRYS_NODISCARD RefPtr<CSSValue> ConsumeClipPath(CSSTokenRange &tokens,
                                                    CSSPropertyParserState &state) noexcept;
  }
}