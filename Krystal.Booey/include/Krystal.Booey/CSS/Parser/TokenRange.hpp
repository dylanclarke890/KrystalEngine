#pragma once

#include "Krystal.Booey/CSS/Parser/Token.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Numeric.hpp"
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
      CSSOMString builder;
      for (size_t i = 0; i < _tokens.size(); ++i)
      {
        _tokens[i].Serialise(builder, (i + 1) == _tokens.size() ? nullptr : &_tokens[i + 1], mode);
      }

      return builder;
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