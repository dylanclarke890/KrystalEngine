// Copyright 2014 The Chromium Authors. All rights reserved.
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

#include "Krystal.Booey/CSS/Parser/Token.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"
#include "Krystal.Core/Types/Span.hpp"

namespace krys::boo::css
{
  class TokenRange
  {
  private:
    Span<const Token> _tokens;

    KRYS_NODISCARD static const Token &EOFToken() noexcept
    {
      static const Token eofToken {TokenType::EndOfFile};
      return eofToken;
    }

  public:
    explicit TokenRange(Span<const Token> tokens) noexcept : _tokens(tokens)
    {
    }

    KRYS_NODISCARD bool IsAtEnd() const noexcept
    {
      return _tokens.empty();
    }

    KRYS_NODISCARD size_t Size() const noexcept
    {
      return _tokens.size();
    }

    KRYS_NODISCARD const Token &Peek(size_t offset = 0uz) const noexcept
    {
      if (offset >= _tokens.size())
      {
        return EOFToken();
      }

      return _tokens[offset];
    }

    KRYS_NODISCARD const Token &Consume() noexcept
    {
      if (_tokens.empty())
      {
        return EOFToken();
      }

      auto &token = _tokens.front();
      _tokens = _tokens.subspan(1uz);

      return token;
    }

    KRYS_NODISCARD TokenRange ConsumeAll() noexcept
    {
      auto range = _tokens;
      _tokens = {};
      return TokenRange(range);
    }

    KRYS_NODISCARD const Token &ConsumeIncludingWhitespace() noexcept
    {
      auto &token = Consume();
      DiscardWhitespace();
      return token;
    }

    KRYS_NODISCARD const Token &ConsumeLast() noexcept
    {
      if (_tokens.empty())
      {
        return EOFToken();
      }

      auto &token = _tokens.back();
      _tokens = _tokens.first(_tokens.size() - 1uz);

      return token;
    }

    void Discard() noexcept
    {
      if (_tokens.empty())
      {
        return;
      }

      _tokens = _tokens.subspan(1uz);
    }

    KRYS_NODISCARD TokenRange ConsumeBlock() noexcept
    {
      krys_debug_assert(Peek().IsBlockStart());

      auto start = _tokens.subspan(1uz);

      size_t nestingLevel = 0uz;
      do
      {
        const Token &token = Consume();
        if (token.IsBlockStart())
        {
          nestingLevel++;
        }
        else if (token.IsBlockEnd())
        {
          nestingLevel--;
        }
      } while (nestingLevel && !_tokens.empty());

      if (nestingLevel)
      {
        // Ended at EOF
        return TokenRange(start.first(_tokens.data() - start.data()));
      }

      return TokenRange(start.first(_tokens.data() - start.data() - 1uz));
    }

    /// @brief Skips over a component value, which may be a simple token or a block of tokens.
    void DiscardComponentValue() noexcept
    {
      size_t nestingLevel = 0uz;
      do
      {
        const Token &token = Consume();
        if (token.IsBlockStart())
        {
          nestingLevel++;
        }
        else if (token.IsBlockEnd())
        {
          nestingLevel--;
        }
      } while (nestingLevel && !_tokens.empty());
    }

    /// @see https://drafts.csswg.org/css-syntax/#token-stream-discard-whitespace
    void DiscardWhitespace() noexcept
    {
      size_t count = 0uz;
      for (auto &token : _tokens)
      {
        if (token.Type() != TokenType::Whitespace)
        {
          break;
        }

        ++count;
      }

      _tokens = _tokens.subspan(count);
    }

    void DiscardTrailingWhitespace() noexcept
    {
      size_t count = 0uz;
      for (auto it = _tokens.rbegin(); it != _tokens.rend(); ++it)
      {
        if (it->Type() != TokenType::Whitespace)
        {
          break;
        }

        ++count;
      }

      _tokens = _tokens.first(_tokens.size() - count);
    }

    /// @brief Creates a new token range that ends at the specified end token range.
    TokenRange RangeUntil(const TokenRange &end) noexcept
    {
      return TokenRange(_tokens.first(end._tokens.data() - _tokens.data()));
    }

    CSSOMString Serialise(TokenSerialisationMode mode) const
    {
      StringBuilder builder;
      for (size_t i = 0; i < _tokens.size(); ++i)
      {
        _tokens[i].Serialise(builder, (i + 1) == _tokens.size() ? nullptr : &_tokens[i + 1], mode);
      }

      return krys::move(builder).ToString();
    }

    /// @brief The tokens remaining in the range.
    KRYS_NODISCARD Span<const Token> Tokens() const noexcept
    {
      return _tokens;
    }

    auto begin() const noexcept
    {
      return _tokens.begin();
    }

    auto end() const noexcept
    {
      return _tokens.end();
    }
  };
}