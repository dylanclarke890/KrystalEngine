#pragma once

#include "Krystal.Booey/Graphics/TextRun.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Text/CodePointIterator.hpp"
#include <unicode/uchar.h>

namespace krys::boo
{
  /// @brief Steps through a `TextRun` a code point at a time, for the bidi resolver. Offsets are byte
  /// offsets.
  class TextRunIterator
  {
  private:
    const TextRun *_textRun {nullptr};
    uint32 _offset {0};

  public:
    TextRunIterator() noexcept = default;

    TextRunIterator(const TextRun *textRun, uint32 offset) noexcept : _textRun(textRun), _offset(offset)
    {
    }

    KRYS_NODISCARD uint32 Offset() const noexcept
    {
      return _offset;
    }

    /// @brief The offset just past the code point at this offset, which ends a bidi run.
    KRYS_NODISCARD uint32 NextOffset() const noexcept
    {
      if (AtEnd())
      {
        return _offset + 1;
      }
      auto text = _textRun->Text();
      return _offset
             + text::DecodeUTF8(Span<const char8>(text.data() + _offset, text.size() - _offset)).Length;
    }

    void Increment() noexcept
    {
      _offset = NextOffset();
    }

    KRYS_NODISCARD bool AtEnd() const noexcept
    {
      return !_textRun || _offset >= _textRun->Length();
    }

    KRYS_NODISCARD char32 Current() const noexcept
    {
      return text::CodePointAt(_textRun->Text(), _offset);
    }

    KRYS_NODISCARD UCharDirection Direction() const noexcept
    {
      return AtEnd() ? U_OTHER_NEUTRAL : u_charDirection(static_cast<UChar32>(Current()));
    }

    KRYS_NODISCARD friend bool operator==(const TextRunIterator &,
                                          const TextRunIterator &) noexcept = default;
  };
}
