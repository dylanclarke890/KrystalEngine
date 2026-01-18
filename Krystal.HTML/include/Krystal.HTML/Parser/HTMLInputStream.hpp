#pragma once

#include "Krystal.Lib/String/String.hpp"
#include "Krystal.Lib/Types/Deque.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include "Krystal.Lib/Types/StronglyTypedValue.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"
#include <cassert>

namespace Krys::HTML
{
  struct IsEOF : StronglyTypedBool<IsEOF>
  {
    using StronglyTypedBool::StronglyTypedBool;
  };

  struct IgnoreNullCharacters : StronglyTypedBool<IgnoreNullCharacters>
  {
    using StronglyTypedBool::StronglyTypedBool;
  };

  class HTMLInputStream
  {
  private:
    utf32_string _data;
    size_t _readPosition {0uz};
    size_t _insertionPosition = utf32_string::npos;
    char32 _nextInputCharacter {0};
    char32 _currentInputCharacter {0};
    bool _appendedLastChunk : 1 {false};
    bool _skipNextNewLine : 1 {false};

  public:
    void Append(utf32_string &&chunk, IsEOF isEOF = IsEOF(false))
    {
      assert(!_appendedLastChunk);

      if (!chunk.empty())
      {
        _data.append(chunk);
      }

      _appendedLastChunk = isEOF;
    }

    void Insert(utf32_string &&chunk)
    {
      if (!chunk.empty())
      {
        assert(_insertionPosition != utf32_string::npos);
        _data.insert(_insertionPosition, chunk);
        _insertionPosition += chunk.size();
      }
    }

    void SetInsertionPoint(size_t insertionPoint) noexcept
    {
      _insertionPosition = insertionPoint;
    }

    void UnsetInsertionPoint() noexcept
    {
      _insertionPosition = utf32_string::npos;
    }

    KRYS_NODISCARD char32 NextInputCharacter() const noexcept
    {
      return _nextInputCharacter;
    }

    /// @brief Peek the next input character without consuming it.
    /// @param ignoreNullCharacters If true, U+0000 is skipped, else becomes U+FFFD.
    bool Peek(IgnoreNullCharacters ignoreNullCharacters = IgnoreNullCharacters(false))
    {
      if (IsEmpty()) KRYS_UNLIKELY
      {
        return false;
      }

      _nextInputCharacter = _data[_readPosition];

      constexpr char32 NotLFOrCROrNullMask = '\n' | '\r' | '\0';
      if (_nextInputCharacter & ~NotLFOrCROrNullMask) KRYS_LIKELY
      {
        _skipNextNewLine = false;
        return true;
      }

      while (true)
      {
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

        _skipNextNewLine = false;
        if (_nextInputCharacter)
        {
          return true;
        }

        if (ignoreNullCharacters)
        {
          _readPosition++;
          if (IsEmpty())
          {
            return false;
          }

          _nextInputCharacter = _data[_readPosition];
          continue;
        }

        _nextInputCharacter = ::Krys::Text::Unicode::Replacement<char32>;
        return true;
      }
    }

    /// @brief Advance to the next input character.
    /// @param ignoreNullCharacters If true, U+0000 is skipped, else becomes U+FFFD.
    bool Advance(IgnoreNullCharacters ignoreNullCharacters = IgnoreNullCharacters(false))
    {
      _readPosition++;
      return Peek(ignoreNullCharacters);
    }

    KRYS_NODISCARD bool IsEmpty() const noexcept
    {
      return _readPosition >= _data.size();
    }

    KRYS_NODISCARD bool IsAtEOF() const noexcept
    {
      if (IsEmpty())
      {
        return _appendedLastChunk;
      }
      return false;
    }
  };
}
