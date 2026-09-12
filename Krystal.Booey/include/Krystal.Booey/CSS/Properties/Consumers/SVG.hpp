#pragma once

#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class TokenRange;
  struct CSSPropertyParserState;
  class CSSValue;

  namespace CSSPropertyParserHelpers
  {
    // MARK: <'paint-order'> consuming
    // https://svgwg.org/svg2-draft/painting.html#PaintOrderProperty
    KRYS_NODISCARD RefPtr<CSSValue> ConsumePaintOrder(TokenRange &tokens,
                                                      CSSPropertyParserState &state) noexcept;

    // MARK: <'stroke-dasharray'> consuming
    // https://svgwg.org/svg2-draft/painting.html#StrokeDashing
    KRYS_NODISCARD RefPtr<CSSValue> ConsumeStrokeDasharray(TokenRange &tokens,
                                                           CSSPropertyParserState &state) noexcept;
  }
}