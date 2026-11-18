/*
 * Copyright (C) 2019 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer.
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials
 *    provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER “AS IS” AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 * TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "Krystal.Booey/CSS/TypedOM/CSSUnparsedValue.hpp"
#include "Krystal.Booey/CSS/Parser/InputStream.hpp"
#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"
#include "Krystal.Booey/CSS/Parser/Tokenizer.hpp"
#include "Krystal.Booey/CSS/Parser/TokenRange.hpp"
#include "Krystal.Booey/CSS/TypedOM/CSSVariableReferenceValue.hpp"
#include "Krystal.Booey/CSS/Values/VariableReferenceValue.hpp"

namespace krys::boo
{
  namespace
  {
    /// @brief Whether @p tokens, just after a `var(` token, start a variable reference this can reify: a
    /// custom property name (after whitespace), then a comma or the closing parenthesis.
    KRYS_NODISCARD bool StartsReifiableVariableReference(css::TokenRange tokens) noexcept
    {
      tokens.DiscardWhitespace();
      const auto &identifier = tokens.ConsumeIncludingWhitespace();
      if (identifier.Type() != css::TokenType::Ident || !identifier.IdentCodePoints().starts_with(u8"--"))
      {
        return false;
      }
      auto next = tokens.Peek().Type();
      return next == css::TokenType::Comma || next == css::TokenType::CloseParen;
    }
  }

  Ref<CSSUnparsedValue> CSSUnparsedValue::Create(List<CSSUnparsedSegment> segments) noexcept
  {
    return AdoptRef(*new CSSUnparsedValue(krys::move(segments)));
  }

  Ref<CSSUnparsedValue> CSSUnparsedValue::Create(css::TokenRange tokens) noexcept
  {
    StringBuilder builder;
    // The segments of the value, then of each variable reference's fallback being reified.
    List<List<CSSUnparsedSegment>> segmentStack;
    segmentStack.emplace_back();
    // For each open function or parenthesis, the name of the variable it references if it's a `var()` with a
    // fallback.
    List<Maybe<CSSOMString>> identifiers;

    auto flushBuilder = [&]
    {
      if (!builder.IsEmpty())
      {
        segmentStack.back().emplace_back(CSSOMString(builder.View()));
        builder.Clear();
      }
    };
    auto closeFallback = [&](CSSOMString &&variable)
    {
      auto fallback = CSSUnparsedValue::Create(krys::move(segmentStack.back()));
      segmentStack.pop_back();
      auto reference = CSSVariableReferenceValue::Create(krys::move(variable), krys::move(fallback));
      krys_debug_assert(!reference.HasException());
      segmentStack.back().emplace_back(RefPtr<CSSVariableReferenceValue>(reference.ReleaseValue()));
    };

    while (!tokens.IsAtEnd())
    {
      const auto &token = tokens.Consume();
      if (token.Type() == css::TokenType::Function && token.FunctionId() == css::ValueId::Var
          && StartsReifiableVariableReference(tokens))
      {
        flushBuilder();
        tokens.DiscardWhitespace();
        CSSOMString variable(tokens.ConsumeIncludingWhitespace().IdentCodePoints());
        if (tokens.Consume().Type() == css::TokenType::Comma)
        {
          // The fallback follows, up to the matching parenthesis.
          identifiers.emplace_back(krys::move(variable));
          segmentStack.emplace_back();
        }
        else
        {
          auto reference = CSSVariableReferenceValue::Create(krys::move(variable));
          krys_debug_assert(!reference.HasException());
          segmentStack.back().emplace_back(RefPtr<CSSVariableReferenceValue>(reference.ReleaseValue()));
        }
      }
      else if (token.Type() == css::TokenType::Function || token.Type() == css::TokenType::OpenParen)
      {
        token.Serialise(builder);
        identifiers.emplace_back();
      }
      else if (token.Type() == css::TokenType::CloseParen && !identifiers.empty())
      {
        auto identifier = krys::move(identifiers.back());
        identifiers.pop_back();
        if (identifier)
        {
          flushBuilder();
          closeFallback(krys::move(*identifier));
        }
        else
        {
          token.Serialise(builder);
        }
      }
      else
      {
        token.Serialise(builder);
      }
    }

    // Close what the tokens left open, as the end of the input closes blocks.
    flushBuilder();
    while (!identifiers.empty())
    {
      auto identifier = krys::move(identifiers.back());
      identifiers.pop_back();
      if (identifier)
      {
        closeFallback(krys::move(*identifier));
      }
    }
    krys_debug_assert(segmentStack.size() == 1uz);

    return CSSUnparsedValue::Create(krys::move(segmentStack.back()));
  }

  CSSUnparsedValue::CSSUnparsedValue(List<CSSUnparsedSegment> segments) noexcept
      : _segments(krys::move(segments))
  {
  }

  CSSUnparsedValue::~CSSUnparsedValue() noexcept = default;

  void CSSUnparsedValue::Serialise(StringBuilder &builder,
                                   EnumSet<SerialisationArguments> arguments) const noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#unparsedvalue-serialization
    for (const auto &segment : _segments)
    {
      krys::SwitchOn(
        segment, [&](const CSSOMString &string) { builder.Append(string); },
        [&](const RefPtr<CSSVariableReferenceValue> &reference)
        { reference->Serialise(builder, arguments); });
    }
  }

  Maybe<CSSUnparsedSegment> CSSUnparsedValue::Item(size_t index) const noexcept
  {
    if (index >= _segments.size())
    {
      return {};
    }
    return _segments[index];
  }

  ExceptionOr<CSSUnparsedSegment> CSSUnparsedValue::SetItem(size_t index, CSSUnparsedSegment segment) noexcept
  {
    if (index > _segments.size())
    {
      return Exception {ExceptionCode::RangeError};
    }
    if (index == _segments.size())
    {
      _segments.push_back(krys::move(segment));
    }
    else
    {
      _segments[index] = krys::move(segment);
    }
    return CSSUnparsedSegment {_segments[index]};
  }

  RefPtr<css::Value> CSSUnparsedValue::ToCSSValue() const noexcept
  {
    css::InputStream input {ToString()};
    css::Tokenizer tokenizer(input);
    std::ignore = tokenizer.PumpTokenizer();
    return css::VariableReferenceValue::Create(tokenizer.Tokens(), css::StrictParserContext());
  }
}
