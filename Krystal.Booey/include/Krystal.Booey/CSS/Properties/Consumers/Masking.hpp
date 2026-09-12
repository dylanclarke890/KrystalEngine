#pragma once

#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class TokenRange;
  struct CSSPropertyParserState;
  class CSSValue;

  namespace CSSPropertyParserHelpers
  {
    // rect() = rect( [ <length> | auto ]#{4} | [ <length> | auto ]{4} )
    // https://drafts.fxtf.org/css-masking/#funcdef-clip-rect
    KRYS_NODISCARD RefPtr<CSSValue> ConsumeClipRectFunction(TokenRange &tokens,
                                                            CSSPropertyParserState &state) noexcept;

    // <'clip-path'> = none | <clip-source> | [ <basic-shape> || <geometry-box> ]
    // https://drafts.fxtf.org/css-masking/#propdef-clip-path
    KRYS_NODISCARD RefPtr<CSSValue> ConsumeClipPath(TokenRange &tokens,
                                                    CSSPropertyParserState &state) noexcept;
  }
}