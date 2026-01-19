#pragma once

#include "Krystal.HTML/StronglyTypedValues.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/String/String.hpp"
#include "Krystal.Lib/Types/Deque.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include "Krystal.Text/Unicode.hpp"
#include <cassert>

namespace Krys::HTML
{
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
    constexpr static inline char32 EOFMarker = U'\uFFFF';

    void Append(utf32_string &&chunk, IsEOF isEOF = IsEOF(false))
    {
      assert(!_appendedLastChunk);

      if (!chunk.empty())
      {
        _data.append(chunk);
      }

      if (isEOF)
      {
        _appendedLastChunk = true;
        _data.append(1uz, EOFMarker);
      }
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
    bool Peek()
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
    bool Advance()
    {
      _readPosition++;
      return Peek();
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
