#pragma once

#include "Krystal.HTML/CSS/Parser/CSSToken.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include "Krystal.Lib/Types/Span.hpp"
#include <cassert>

namespace Krys::HTML
{
  class CSSTokenRange
  {
  private:
    Span<const CSSToken> _tokens;

    KRYS_NODISCARD static const CSSToken &EOFToken() noexcept
    {
      static const CSSToken eofToken {CSSTokenType::EndOfFile};
      return eofToken;
    }

  public:
    explicit CSSTokenRange(Span<const CSSToken> tokens) noexcept : _tokens(tokens)
    {
    }

    KRYS_NODISCARD bool IsAtEnd() const noexcept
    {
      return _tokens.empty();
    }

    KRYS_NODISCARD const CSSToken &Peek(size_t offset = 0uz) const noexcept
    {
      if (offset >= _tokens.size())
      {
        return EOFToken();
      }

      return _tokens[offset];
    }

    KRYS_NODISCARD const CSSToken &Consume() noexcept
    {
      if (_tokens.empty())
      {
        return EOFToken();
      }

      auto &token = _tokens.front();
      _tokens = _tokens.subspan(1uz);
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

    KRYS_NODISCARD CSSTokenRange ConsumeBlock() noexcept
    {
      assert(Peek().Type() == CSSTokenType::OpenCurly);

      auto start = _tokens.subspan(1uz);

      size_t nestingLevel = 0uz;
      do
      {
        const CSSToken &token = Consume();
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
        return CSSTokenRange(start.first(_tokens.data() - start.data()));
      }

      return CSSTokenRange(start.first(_tokens.data() - start.data() - 1uz));
    }

    void SkipComponentValue() noexcept
    {
      size_t nestingLevel = 0uz;
      do
      {
        const CSSToken &token = Consume();
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
        if (token.Type() != CSSTokenType::Whitespace)
        {
          break;
        }
        ++count;
      }

      _tokens = _tokens.subspan(count);
    }

    /// @brief Creates a new token range that ends at the specified end token range.
    CSSTokenRange RangeUntil(const CSSTokenRange &end) noexcept
    {
      return CSSTokenRange(_tokens.first(end._tokens.data() - _tokens.data()));
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