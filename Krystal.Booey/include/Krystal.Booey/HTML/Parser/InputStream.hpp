#pragma once

#include "Krystal.Booey/Types/IsEOF.hpp"
#include "Krystal.Booey/Types/SourceLocation.hpp"
#include "Krystal.Core/Base.hpp"
#include "Krystal.Core/Text/Encodings/Decode.hpp"
#include "Krystal.Core/Text/Encodings/Encode.hpp"
#include "Krystal.Core/Text/Encodings/UTF.hpp"

namespace krys::boo::html
{
  class InputStream
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
      krys_debug_assert(_data.empty() || _data.back() != EOFMarker);

      if (!chunk.empty())
      {
        auto decodeResult = krys::text::Decode<krys::text::UTF8>(utf8_stringview(chunk));
        krys_debug_assert(decodeResult.Error == krys::text::DecodeError::None);
        _data.append(decodeResult.Output);
      }

      if (isEOF)
      {
        _data.append(1uz, EOFMarker);
      }
    }

    void Insert(utf8_string &&chunk) noexcept
    {
      krys_debug_assert(_insertionPosition != utf32_string::npos);

      if (!chunk.empty())
      {
        auto decodeResult = krys::text::Decode<krys::text::UTF8>(utf8_stringview(chunk));
        krys_debug_assert(decodeResult.Error == krys::text::DecodeError::None);
        _data.insert(_insertionPosition, decodeResult.Output);
        _insertionPosition += decodeResult.Output.size();
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
    KRYS_NODISCARD MatchResult AdvancePast(stringview characters) noexcept
    {
#if KRYS_DEBUG
      for (auto character : characters)
      {
        krys_debug_assert(character != '\n' && character != '\r' && character != EOFMarker);
      }
#endif

      size_t availableCharacters = RemainingCharacters();
      if (availableCharacters < characters.size())
      {
        return MatchResult::NotEnoughCharacters;
      }

      for (size_t i = 0uz; i < characters.size(); ++i)
      {
        char32 inputChar = _data[_readPosition + i];
        char32 matchChar = characters[i];

        if constexpr (CaseInsensitive)
        {
          inputChar = krys::text::ToASCIILower(inputChar);
          matchChar = krys::text::ToASCIILower(matchChar);
        }

        if (inputChar != matchChar)
        {
          return MatchResult::DidNotMatch;
        }
      }

      _readPosition += characters.size();
      _currentLocation.Column += characters.size();
      _nextInputCharacter = _data[_readPosition];

      return MatchResult::Matched;
    }

    KRYS_NODISCARD SourceLocation GetCurrentLocation() const noexcept
    {
      return _currentLocation;
    }
  };
}
