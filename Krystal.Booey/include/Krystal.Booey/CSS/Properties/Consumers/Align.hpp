#pragma once

#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class TokenRange;
  class Value;
  struct PropertyParserState;

  namespace PropertyParserHelpers
  {
    /// @grammar <'align-content'>
    /// @see https://drafts.csswg.org/css-align/#propdef-align-content
    KRYS_NODISCARD RefPtr<Value> ConsumeAlignContent(TokenRange &tokens,
                                                        PropertyParserState &state) noexcept;

    /// @grammar <'justify-content'>
    /// @see https://drafts.csswg.org/css-align/#propdef-justify-content
    KRYS_NODISCARD RefPtr<Value> ConsumeJustifyContent(TokenRange &tokens,
                                                          PropertyParserState &state) noexcept;

    /// @grammar <'align-self'>
    /// @see https://drafts.csswg.org/css-align/#propdef-align-self
    KRYS_NODISCARD RefPtr<Value> ConsumeAlignSelf(TokenRange &tokens,
                                                     PropertyParserState &state) noexcept;

    /// @grammar <'justify-self'>
    /// @see https://drafts.csswg.org/css-align/#propdef-justify-self
    KRYS_NODISCARD RefPtr<Value> ConsumeJustifySelf(TokenRange &tokens,
                                                       PropertyParserState &state) noexcept;

    /// @grammar <'align-items'>
    /// @see https://drafts.csswg.org/css-align/#propdef-align-items
    KRYS_NODISCARD RefPtr<Value> ConsumeAlignItems(TokenRange &tokens,
                                                      PropertyParserState &state) noexcept;

    /// @grammar <'justify-items'>
    /// @see https://drafts.csswg.org/css-align/#propdef-justify-items
    KRYS_NODISCARD RefPtr<Value> ConsumeJustifyItems(TokenRange &tokens,
                                                        PropertyParserState &state) noexcept;
  }
}