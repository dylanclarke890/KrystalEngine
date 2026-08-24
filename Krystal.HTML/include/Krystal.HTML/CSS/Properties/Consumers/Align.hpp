#pragma once

#include "Krystal.Lib/Pointers/RefPtr.hpp"

namespace Krys::HTML
{
  class CSSTokenRange;
  class CSSValue;
  struct CSSPropertyParserState;

  namespace CSSPropertyParserHelpers
  {
    /// @grammar <'align-content'>
    /// @see https://drafts.csswg.org/css-align/#propdef-align-content
    KRYS_NODISCARD RefPtr<CSSValue> ConsumeAlignContent(CSSTokenRange &tokens,
                                                        CSSPropertyParserState &state) noexcept;

    /// @grammar <'justify-content'>
    /// @see https://drafts.csswg.org/css-align/#propdef-justify-content
    KRYS_NODISCARD RefPtr<CSSValue> ConsumeJustifyContent(CSSTokenRange &tokens,
                                                          CSSPropertyParserState &state) noexcept;

    /// @grammar <'align-self'>
    /// @see https://drafts.csswg.org/css-align/#propdef-align-self
    KRYS_NODISCARD RefPtr<CSSValue> ConsumeAlignSelf(CSSTokenRange &tokens,
                                                     CSSPropertyParserState &state) noexcept;

    /// @grammar <'justify-self'>
    /// @see https://drafts.csswg.org/css-align/#propdef-justify-self
    KRYS_NODISCARD RefPtr<CSSValue> ConsumeJustifySelf(CSSTokenRange &tokens,
                                                       CSSPropertyParserState &state) noexcept;

    /// @grammar <'align-items'>
    /// @see https://drafts.csswg.org/css-align/#propdef-align-items
    KRYS_NODISCARD RefPtr<CSSValue> ConsumeAlignItems(CSSTokenRange &tokens,
                                                      CSSPropertyParserState &state) noexcept;

    /// @grammar <'justify-items'>
    /// @see https://drafts.csswg.org/css-align/#propdef-justify-items
    KRYS_NODISCARD RefPtr<CSSValue> ConsumeJustifyItems(CSSTokenRange &tokens,
                                                        CSSPropertyParserState &state) noexcept;
  }
}