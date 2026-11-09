// Copyright 2015 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 Apple Inc. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//    * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//    * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//    * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#pragma once

#include "Krystal.Booey/CSS/Parser/TokenRange.hpp"
#include "Krystal.Booey/CSS/Parser/TokenType.hpp"
#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Core/Attributes.hpp"

namespace krys::boo::css
{
  class Parser;
  struct ParserContext;

  /// @brief Evaluates `@supports` conditions and `CSS.supports()`.
  /// @see https://drafts.csswg.org/css-conditional-3/#at-supports
  class SupportsParser
  {
  public:
    enum SupportsResult : uint8
    {
      Unsupported = false,
      Supported = true,
      Invalid
    };

    enum class ParsingMode : uint8
    {
      ForAtRuleSupports,
      /// @brief `CSS.supports()`: a condition, or anything else as if it were in parentheses.
      AllowBareDeclarationAndGeneralEnclosed,
      /// @brief `@import`'s `supports()`: a condition or a declaration, and otherwise invalid.
      /// @see https://drafts.csswg.org/css-cascade-5/#at-import
      AllowBareDeclaration,
    };

  private:
    Parser &_parser;

    explicit SupportsParser(Parser &parser) noexcept : _parser(parser)
    {
    }

  public:
    KRYS_NODISCARD static SupportsResult SupportsCondition(TokenRange tokens, Parser &parser,
                                                           ParsingMode mode) noexcept;
    KRYS_NODISCARD static SupportsResult
      SupportsCondition(CSSOMStringView condition, const ParserContext &context, ParsingMode mode) noexcept;

  private:
    KRYS_NODISCARD SupportsResult ConsumeCondition(TokenRange tokens) noexcept;
    KRYS_NODISCARD SupportsResult ConsumeNegation(TokenRange tokens) noexcept;
    KRYS_NODISCARD SupportsResult ConsumeSupportsFunction(TokenRange &tokens) noexcept;
    KRYS_NODISCARD SupportsResult ConsumeSupportsFeatureOrGeneralEnclosed(TokenRange &tokens) noexcept;
    // https://drafts.csswg.org/css-conditional-4/#typedef-supports-selector-fn
    // <supports-selector-fn> = selector( <complex-selector> );
    KRYS_NODISCARD SupportsResult ConsumeSupportsSelectorFunction(TokenRange &tokens) noexcept;

    // https://drafts.csswg.org/css-conditional-5/#typedef-supports-font-format-fn
    KRYS_NODISCARD SupportsResult ConsumeSupportsFontFormatFunction(TokenRange &tokens) noexcept;
    // https://drafts.csswg.org/css-conditional-5/#typedef-supports-font-tech-fn
    KRYS_NODISCARD SupportsResult ConsumeSupportsFontTechFunction(TokenRange &tokens) noexcept;

    KRYS_NODISCARD SupportsResult ConsumeConditionInParenthesis(TokenRange &tokens,
                                                                TokenType startTokenType) noexcept;
  };
}
