#pragma once

#include "Krystal.Lib/Pointers/RefPtr.hpp"

namespace Krys::HTML
{
  class CSSTokenRange;
  struct CSSPropertyParserState;
  class CSSValue;

  namespace CSSPropertyParserHelpers
  {
    // MARK: <'paint-order'> consuming
    // https://svgwg.org/svg2-draft/painting.html#PaintOrderProperty
    KRYS_NODISCARD RefPtr<CSSValue> ConsumePaintOrder(CSSTokenRange &tokens,
                                                      CSSPropertyParserState &state) noexcept;

    // MARK: <'stroke-dasharray'> consuming
    // https://svgwg.org/svg2-draft/painting.html#StrokeDashing
    KRYS_NODISCARD RefPtr<CSSValue> ConsumeStrokeDasharray(CSSTokenRange &tokens,
                                                           CSSPropertyParserState &state) noexcept;
  }
}