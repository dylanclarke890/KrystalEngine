// Copyright 2015 The Chromium Authors. All rights reserved.
// Copyright (C) 2016-2024 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/CSS/Parser/SupportsParser.hpp"
#include "Krystal.Booey/CSS/Parser/Parser.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Font.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyParserState.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorParser.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorParserContext.hpp"
#include "Krystal.Booey/CSS/Values/FontFaceSrcValue.hpp"
#include "Krystal.Booey/Graphics/Fonts/FontCustomPlatformData.hpp"
#include "Krystal.Core/Text/ASCII.hpp"

namespace krys::boo::css
{
  auto SupportsParser::SupportsCondition(TokenRange tokens, Parser &parser, ParsingMode mode) noexcept
    -> SupportsResult
  {
    // FIXME: The spec allows leading whitespace in @supports but not CSS.supports,
    // but major browser vendors allow it in CSS.supports also.
    tokens.DiscardWhitespace();
    SupportsParser supportsParser(parser);

    auto result = supportsParser.ConsumeCondition(tokens);
    if (mode == ParsingMode::ForAtRuleSupports || result != Invalid)
    {
      return result;
    }

    if (mode == ParsingMode::AllowBareDeclaration)
    {
      // A declaration is a property name and a colon; one the parser doesn't support is still valid.
      auto declaration = tokens;
      if (declaration.Peek().Type() != TokenType::Ident)
      {
        return Invalid;
      }
      (void)declaration.ConsumeIncludingWhitespace();
      if (declaration.Peek().Type() != TokenType::Colon)
      {
        return Invalid;
      }
      return parser.SupportsDeclaration(tokens) ? Supported : Unsupported;
    }

    // window.CSS.supports requires parsing as-if the condition was wrapped in
    // parenthesis. The only productions that wouldn't have parsed above are the
    // declaration condition or the general enclosed productions.
    return supportsParser.ConsumeSupportsFeatureOrGeneralEnclosed(tokens);
  }

  auto SupportsParser::SupportsCondition(CSSOMStringView condition, const ParserContext &context,
                                         ParsingMode mode) noexcept -> SupportsResult
  {
    Parser parser(context, condition);
    if (!parser.GetTokenizer())
    {
      return Invalid;
    }
    return SupportsCondition(parser.GetTokenizer()->Tokens(), parser, mode);
  }

  namespace
  {
    enum class ClauseType : uint8
    {
      Unresolved,
      Conjunction,
      Disjunction
    };
  }

  auto SupportsParser::ConsumeCondition(TokenRange tokens) noexcept -> SupportsResult
  {
    if (tokens.Peek().Type() == TokenType::Ident)
    {
      if (text::IsASCIICaselessEqual(tokens.Peek().IdentCodePoints(), utf8_stringview(u8"not")))
      {
        return ConsumeNegation(tokens);
      }
    }

    bool result = false;
    auto clauseType = ClauseType::Unresolved;

    auto previousTokenType = TokenType::Ident;

    while (true)
    {
      auto nextResult = ConsumeConditionInParenthesis(tokens, previousTokenType);
      if (nextResult == Invalid)
      {
        return Invalid;
      }
      bool nextSupported = nextResult;
      if (clauseType == ClauseType::Unresolved)
      {
        result = nextSupported;
      }
      else if (clauseType == ClauseType::Conjunction)
      {
        result &= nextSupported;
      }
      else
      {
        result |= nextSupported;
      }

      if (tokens.IsAtEnd())
      {
        break;
      }
      tokens.DiscardWhitespace();
      if (tokens.IsAtEnd())
      {
        break;
      }

      const Token &token = tokens.Peek();
      if (token.Type() != TokenType::Ident)
      {
        return Invalid;
      }

      previousTokenType = token.Type();

      if (clauseType == ClauseType::Unresolved)
      {
        clauseType =
          token.IdentCodePoints().size() == 3uz ? ClauseType::Conjunction : ClauseType::Disjunction;
      }
      if ((clauseType == ClauseType::Conjunction
           && !text::IsASCIICaselessEqual(token.IdentCodePoints(), utf8_stringview(u8"and")))
          || (clauseType == ClauseType::Disjunction
              && !text::IsASCIICaselessEqual(token.IdentCodePoints(), utf8_stringview(u8"or"))))
      {
        return Invalid;
      }

      tokens.Discard();
      if (tokens.Peek().Type() != TokenType::Whitespace)
      {
        return Invalid;
      }
      tokens.DiscardWhitespace();
    }
    return result ? Supported : Unsupported;
  }

  auto SupportsParser::ConsumeNegation(TokenRange tokens) noexcept -> SupportsResult
  {
    krys_debug_assert(tokens.Peek().Type() == TokenType::Ident);
    auto tokenType = tokens.Peek().Type();

    if (tokens.Peek().Type() == TokenType::Ident)
    {
      tokens.Discard();
    }
    if (tokens.Peek().Type() != TokenType::Whitespace)
    {
      return Invalid;
    }
    tokens.DiscardWhitespace();
    auto result = ConsumeConditionInParenthesis(tokens, tokenType);
    tokens.DiscardWhitespace();
    if (!tokens.IsAtEnd() || result == Invalid)
    {
      return Invalid;
    }

    return result ? Unsupported : Supported;
  }

  // <function-token> <any-value>? | <supports-selector-fn> | <supports-font-format-fn> |
  // <supports-font-tech-fn>
  auto SupportsParser::ConsumeSupportsFunction(TokenRange &tokens) noexcept -> SupportsResult
  {
    if (tokens.Peek().Type() != TokenType::Function)
    {
      return Invalid;
    }

    switch (tokens.Peek().FunctionId())
    {
      case ValueId::Selector:   return ConsumeSupportsSelectorFunction(tokens);
      case ValueId::FontFormat: return ConsumeSupportsFontFormatFunction(tokens);
      case ValueId::FontTech:   return ConsumeSupportsFontTechFunction(tokens);
      default: // Unknown functions should parse as unsupported.
        tokens.DiscardComponentValue();
        return Unsupported;
    }
  }

  // <supports-feature> | <general-enclosed>
  auto SupportsParser::ConsumeSupportsFeatureOrGeneralEnclosed(TokenRange &tokens) noexcept -> SupportsResult
  {
    if (tokens.Peek().Type() == TokenType::Function)
    {
      if (auto result = ConsumeSupportsFunction(tokens); result != Invalid)
      {
        return result;
      }
    }

    return tokens.Peek().Type() == TokenType::Ident && _parser.SupportsDeclaration(tokens) ? Supported
                                                                                           : Unsupported;
  }

  // <supports-selector-fn>
  auto SupportsParser::ConsumeSupportsSelectorFunction(TokenRange &tokens) noexcept -> SupportsResult
  {
    krys_debug_assert(tokens.Peek().Type() == TokenType::Function
                      && tokens.Peek().FunctionId() == ValueId::Selector);

    auto block = tokens.ConsumeBlock();
    block.DiscardWhitespace();

    return SelectorParser::SupportsComplexSelector(block, SelectorParserContext(_parser.Context()))
             ? Supported
             : Unsupported;
  }

  // <supports-font-format-fn>
  auto SupportsParser::ConsumeSupportsFontFormatFunction(TokenRange &tokens) noexcept -> SupportsResult
  {
    krys_debug_assert(tokens.Peek().Type() == TokenType::Function
                      && tokens.Peek().FunctionId() == ValueId::FontFormat);

    auto state = PropertyParserState {.Context = _parser.Context()};
    auto format = PropertyParserHelpers::ConsumeFontFormat(tokens, state, true);
    if (!format)
    {
      return Unsupported;
    }
    return FontCustomPlatformData::SupportsFormat(*format) ? Supported : Unsupported;
  }

  // <supports-font-tech-fn>
  auto SupportsParser::ConsumeSupportsFontTechFunction(TokenRange &tokens) noexcept -> SupportsResult
  {
    krys_debug_assert(tokens.Peek().Type() == TokenType::Function
                      && tokens.Peek().FunctionId() == ValueId::FontTech);

    auto state = PropertyParserState {.Context = _parser.Context()};
    auto technologies = PropertyParserHelpers::ConsumeFontTech(tokens, state, true);
    if (technologies.empty())
    {
      return Unsupported;
    }
    krys_debug_assert(technologies.size() == 1uz);
    return FontCustomPlatformData::SupportsTechnology(technologies[0]) ? Supported : Unsupported;
  }

  // <supports-in-parens> = ( <supports-condition> ) | <supports-feature> | <general-enclosed>
  auto SupportsParser::ConsumeConditionInParenthesis(TokenRange &tokens, TokenType startTokenType) noexcept
    -> SupportsResult
  {
    if (startTokenType == TokenType::Ident && tokens.Peek().Type() != TokenType::OpenParen)
    {
      if (auto result = ConsumeSupportsFunction(tokens); result != Invalid)
      {
        return result;
      }
      return Invalid;
    }

    auto innerRange = tokens.ConsumeBlock();
    innerRange.DiscardWhitespace();

    if (auto result = ConsumeCondition(innerRange); result != Invalid)
    {
      return result;
    }

    return ConsumeSupportsFeatureOrGeneralEnclosed(innerRange);
  }
}
