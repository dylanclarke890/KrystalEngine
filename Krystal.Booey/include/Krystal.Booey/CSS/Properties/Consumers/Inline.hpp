#pragma once

#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class TokenRange;
  struct PropertyParserState;

  namespace PropertyParserHelpers
  {
    // MARK: <'line-fit-edge'> consuming
    // https://drafts.csswg.org/css-inline-3/#propdef-line-fit-edge
    KRYS_NODISCARD RefPtr<Value> ConsumeLineFitEdge(TokenRange &tokens,
                                                       PropertyParserState &state) noexcept;

    // MARK: <'text-box-edge'> consuming
    // https://drafts.csswg.org/css-inline-3/#propdef-text-box-edge
    KRYS_NODISCARD RefPtr<Value> ConsumeTextBoxEdge(TokenRange &tokens,
                                                       PropertyParserState &state) noexcept;
  }
}