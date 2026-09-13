#pragma once

#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class TokenRange;
  class CSSValue;
  struct PropertyParserState;

  namespace PropertyParserHelpers
  {
    /// @grammar <'align-content'>
    /// @see https://drafts.csswg.org/css-align/#propdef-align-content
    KRYS_NODISCARD RefPtr<CSSValue> ConsumeAlignContent(TokenRange &tokens,
                                                        PropertyParserState &state) noexcept;

    /// @grammar <'justify-content'>
    /// @see https://drafts.csswg.org/css-align/#propdef-justify-content
    KRYS_NODISCARD RefPtr<CSSValue> ConsumeJustifyContent(TokenRange &tokens,
                                                          PropertyParserState &state) noexcept;

    /// @grammar <'align-self'>
    /// @see https://drafts.csswg.org/css-align/#propdef-align-self
    KRYS_NODISCARD RefPtr<CSSValue> ConsumeAlignSelf(TokenRange &tokens,
                                                     PropertyParserState &state) noexcept;

    /// @grammar <'justify-self'>
    /// @see https://drafts.csswg.org/css-align/#propdef-justify-self
    KRYS_NODISCARD RefPtr<CSSValue> ConsumeJustifySelf(TokenRange &tokens,
                                                       PropertyParserState &state) noexcept;

    /// @grammar <'align-items'>
    /// @see https://drafts.csswg.org/css-align/#propdef-align-items
    KRYS_NODISCARD RefPtr<CSSValue> ConsumeAlignItems(TokenRange &tokens,
                                                      PropertyParserState &state) noexcept;

    /// @grammar <'justify-items'>
    /// @see https://drafts.csswg.org/css-align/#propdef-justify-items
    KRYS_NODISCARD RefPtr<CSSValue> ConsumeJustifyItems(TokenRange &tokens,
                                                        PropertyParserState &state) noexcept;
  }
}