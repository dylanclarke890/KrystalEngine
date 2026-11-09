/*
 * Copyright (C) 2022 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/Graphics/TextRuns/TextRun.hpp"
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
