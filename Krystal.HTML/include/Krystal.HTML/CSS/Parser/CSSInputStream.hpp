#pragma once

#include "Krystal.HTML/CSS/Types/CSSOMString.hpp"
#include "Krystal.HTML/Types/IsEOF.hpp"
#include "Krystal.HTML/Types/SourceLocation.hpp"
#include "Krystal.Lib/Core/Move.hpp"
#include "Krystal.Lib/String/String.hpp"
#include "Krystal.Text/ASCIILiteral.hpp"
#include "Krystal.Text/StringConversion.hpp"
#include "Krystal.Text/Unicode.hpp"
#include <cassert>

namespace Krys::HTML
{
  /// @see https://drafts.csswg.org/css-syntax/#input-stream
  class CSSInputStream
  {
  private:
    // TODO(perf): a lot of text is just plain ASCII, we should optimize for that case.
    utf32_string _data;
    size_t _readPosition {0uz};
    SourceLocation _currentLocation;

  public:
    CSSInputStream(CSSOMString &&input) noexcept : _data(FilterCodePoints(Krys::Move(input)))
    {
      assert(!_data.empty() && _data.back() != EOFMarker);
    }

    /// @brief Gets the size of the input stream. This is the number of characters in the input stream, not
    /// including the EOF marker. Not affected by the current read position.
    KRYS_NODISCARD size_t Size() const noexcept
    {
      return _data.size();
    }

    /// @brief Gets the next character without consuming it.
    KRYS_NODISCARD char32 NextInputCharacter() const noexcept
    {
      if (_readPosition >= _data.size()) KRYS_UNLIKELY
      {
        return EOFMarker;
      }

      return _data[_readPosition];
    }

    /// @brief Peek the next `n` characters ahead without consuming any.
    char32 Peek(size_t n) noexcept
    {
      if (_readPosition + n >= _data.size()) KRYS_UNLIKELY
      {
        return EOFMarker;
      }

      return _data[_readPosition + n];
    }

    /// @brief Consumes the next character.
    void Consume() noexcept
    {
      if (NextInputCharacter() == U'\n')
      {
        _currentLocation.Line++;
        _currentLocation.Column = 1uz;
      }
      else
      {
        _currentLocation.Column++;
      }

      _readPosition++;
    }

    /// @brief Consumes the next `n` characters.
    void Consume(size_t n) noexcept
    {
      for (size_t i = 0uz; i < n; ++i)
      {
        auto ch = Peek(i);
        if (Peek(i) == U'\n')
        {
          _currentLocation.Line++;
          _currentLocation.Column = 1uz;
        }
        else
        {
          _currentLocation.Column++;
        }
      }

      _readPosition += n;
    }

    /// @brief Reconsume the last input character. This is used when the parser needs to "unread" a character.
    void Reconsume(char32 ch) noexcept
    {
      _readPosition--;
      assert(NextInputCharacter() == ch);
    }

    KRYS_NODISCARD size_t ReadPosition() const noexcept
    {
      return _readPosition;
    }

    KRYS_NODISCARD SourceLocation CurrentLocation() const noexcept
    {
      return _currentLocation;
    }

  private:
    /// @see https://drafts.csswg.org/css-syntax/#css-filter-code-points
    static utf32_string FilterCodePoints(CSSOMString &&input) noexcept
    {
      utf8_string output;
      output.reserve(input.size());

      for (auto it = input.begin(); it != input.end(); it++)
      {
        auto ch = *it;

        // SPEC: Replace any U+000D CARRIAGE RETURN (CR) code points, U+000C FORM FEED (FF) code points, or
        // pairs of U+000D CARRIAGE RETURN (CR) followed by U+000A LINE FEED (LF) in input by a single U+000A
        // LINE FEED (LF) code point.

        if (ch == U'\r')
        {
          auto next = std::next(it);
          if (next != input.end() && *next == U'\n')
          {
            ++it; // consume the LF as well
          }

          output.push_back(U'\n');
          continue;
        }

        if (ch == U'\f') KRYS_UNLIKELY
        {
          output.push_back(U'\n');
          continue;
        }

        // SPEC: Replace any U+0000 NULL or surrogate code points in input with U+FFFD REPLACEMENT CHARACTER
        // (�).
        // NOTE: We don't need to check for surrogate code points here because the input is UTF-8 and will
        // never contain surrogate code points. However, we still check for NULL code points.

        if (ch == U'\0') KRYS_UNLIKELY
        {
          output += u8"\uFFFD";
          continue;
        }

        // finally, we can just append the text character to the output
        output.push_back(ch);
        continue;
      }

      return Krys::Text::ConvertToUTF32(utf8_stringview(output));
    }
  };
}