#pragma once

#include "Krystal.HTML/Types/IsEOF.hpp"
#include "Krystal.HTML/Types/SourceLocation.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/String/String.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include "Krystal.Text/ASCII.hpp"
#include "Krystal.Text/ASCIILiteral.hpp"
#include "Krystal.Text/StringConversion.hpp"
#include "Krystal.Text/Unicode.hpp"
#include <cassert>

namespace Krys::HTML
{
  class HTMLInputStream
  {
  private:
    // TODO(perf): a lot of text is just plain ASCII, we should optimize for that case.
    utf32_string _data;
    size_t _readPosition {0uz};
    size_t _insertionPosition = utf32_string::npos;
    char32 _nextInputCharacter {0};
    bool _skipNextNewLine : 1 {false};
    SourceLocation _currentLocation;

  public:
    void Append(utf8_string &&chunk, IsEOF isEOF = IsEOF(false)) noexcept
    {
      assert(_data.empty() || _data.back() != EOFMarker);

      if (!chunk.empty())
      {
        _data.append(Krys::Text::ConvertToUTF32(utf8_stringview(chunk)));
      }

      if (isEOF)
      {
        _data.append(1uz, EOFMarker);
      }
    }

    void Insert(utf8_string &&chunk) noexcept
    {
      assert(_insertionPosition != utf32_string::npos);

      if (!chunk.empty())
      {
        _data.insert(_insertionPosition, Krys::Text::ConvertToUTF32(utf8_stringview(chunk)));
        _insertionPosition += chunk.size();
      }
    }

    void SetInsertionPoint(size_t insertionPoint) noexcept
    {
      _insertionPosition = insertionPoint;
    }

    void ClearInsertionPoint() noexcept
    {
      _insertionPosition = utf32_string::npos;
    }

    KRYS_NODISCARD char32 NextInputCharacter() const noexcept
    {
      return _nextInputCharacter;
    }

    /// @brief Peek the next input character without consuming it.
    bool Peek() noexcept
    {
      if (IsEmpty()) KRYS_UNLIKELY
      {
        return false;
      }

      _nextInputCharacter = _data[_readPosition];

      constexpr char32 NotLFOrCROrNullMask = '\n' | '\r';
      if (_nextInputCharacter & ~NotLFOrCROrNullMask) KRYS_LIKELY
      {
        _skipNextNewLine = false;
        return true;
      }

      /// @see https://html.spec.whatwg.org/#preprocessing-the-input-stream
      if (_nextInputCharacter == '\n' && _skipNextNewLine)
      {
        _skipNextNewLine = false;
        _readPosition++;

        if (IsEmpty())
        {
          return false;
        }

        _nextInputCharacter = _data[_readPosition];
      }

      if (_nextInputCharacter == '\r')
      {
        _nextInputCharacter = '\n';
        _skipNextNewLine = true;
        return true;
      }

      return true;
    }

    /// @brief Advance to the next input character.
    bool Advance() noexcept
    {
      if (IsEmpty()) KRYS_UNLIKELY
      {
        return false;
      }

      if (_nextInputCharacter == '\n')
      {
        _currentLocation.Line++;
        _currentLocation.Column = 1uz;
      }
      else
      {
        _currentLocation.Column++;
      }

      _readPosition++;
      return Peek();
    }

    KRYS_NODISCARD bool IsEmpty() const noexcept
    {
      return _readPosition >= _data.size();
    }

    KRYS_NODISCARD size_t RemainingCharacters() const noexcept
    {
      if (IsEmpty())
      {
        return 0uz;
      }

      return (_data.size() - _readPosition) - (_data.back() == EOFMarker ? 1uz : 0uz);
    }

    enum class MatchResult : uint8
    {
      Matched,
      NotEnoughCharacters,
      DidNotMatch,
    };

    template <bool CaseInsensitive = true>
    KRYS_NODISCARD MatchResult AdvancePast(Krys::Text::ASCIILiteral characters) noexcept
    {
#if KRYS_ENV(DEV)
      for (auto character : characters.ToSpan())
      {
        assert(character != '\n' && character != '\r' && character != EOFMarker);
      }
#endif

      size_t availableCharacters = RemainingCharacters();
      if (availableCharacters < characters.Length())
      {
        return MatchResult::NotEnoughCharacters;
      }

      for (size_t i = 0uz; i < characters.Length(); ++i)
      {
        char32 inputChar = _data[_readPosition + i];
        char32 matchChar = characters[i];

        if constexpr (CaseInsensitive)
        {
          inputChar = Krys::Text::ToASCIILower(inputChar);
          matchChar = Krys::Text::ToASCIILower(matchChar);
        }

        if (inputChar != matchChar)
        {
          return MatchResult::DidNotMatch;
        }
      }

      _readPosition += characters.Length();
      _currentLocation.Column += characters.Length();
      _nextInputCharacter = _data[_readPosition];

      return MatchResult::Matched;
    }

    KRYS_NODISCARD SourceLocation GetCurrentLocation() const noexcept
    {
      return _currentLocation;
    }
  };
}
