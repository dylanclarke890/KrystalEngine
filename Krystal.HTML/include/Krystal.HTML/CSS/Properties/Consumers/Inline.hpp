#pragma once

#include "Krystal.HTML/CSS/Values/CSSValue.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"

namespace Krys::HTML
{
  class CSSTokenRange;
  struct CSSPropertyParserState;

  namespace CSSPropertyParserHelpers
  {
    // MARK: <'line-fit-edge'> consuming
    // https://drafts.csswg.org/css-inline-3/#propdef-line-fit-edge
    KRYS_NODISCARD RefPtr<CSSValue> ConsumeLineFitEdge(CSSTokenRange &tokens,
                                                       CSSPropertyParserState &state) noexcept;

    // MARK: <'text-box-edge'> consuming
    // https://drafts.csswg.org/css-inline-3/#propdef-text-box-edge
    KRYS_NODISCARD RefPtr<CSSValue> ConsumeTextBoxEdge(CSSTokenRange &tokens,
                                                       CSSPropertyParserState &state) noexcept;
  }
}