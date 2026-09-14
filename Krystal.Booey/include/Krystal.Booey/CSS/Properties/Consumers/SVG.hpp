#pragma once

#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class TokenRange;
  struct PropertyParserState;
  class Value;

  namespace PropertyParserHelpers
  {
    // MARK: <'paint-order'> consuming
    // https://svgwg.org/svg2-draft/painting.html#PaintOrderProperty
    KRYS_NODISCARD RefPtr<Value> ConsumePaintOrder(TokenRange &tokens,
                                                      PropertyParserState &state) noexcept;

    // MARK: <'stroke-dasharray'> consuming
    // https://svgwg.org/svg2-draft/painting.html#StrokeDashing
    KRYS_NODISCARD RefPtr<Value> ConsumeStrokeDasharray(TokenRange &tokens,
                                                           PropertyParserState &state) noexcept;
  }
}