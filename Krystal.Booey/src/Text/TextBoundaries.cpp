/*
 * Copyright (C) 2006, 2007 Apple Inc. All rights reserved.
 * Copyright (C) 2009 Dominik Röttsches <dominik.roettsches@access-company.com>
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

#include "Krystal.Booey/Text/TextBoundaries.hpp"
#include "Krystal.Booey/Text/TextBreakIterator.hpp"
#include <unicode/ubrk.h>
#include <unicode/utf8.h>

namespace krys::boo
{
  namespace
  {
    KRYS_NODISCARD const uint8_t *Bytes(utf8_stringview text) noexcept
    {
      return reinterpret_cast<const uint8_t *>(text.data());
    }

    /// @brief The code point that starts at @p offset.
    KRYS_NODISCARD UChar32 CodePointAt(utf8_stringview text, int32 offset) noexcept
    {
      UChar32 c;
      U8_NEXT(Bytes(text), offset, static_cast<int32>(text.size()), c);
      return c;
    }

    /// @brief The code point that ends at @p offset.
    KRYS_NODISCARD UChar32 CodePointBefore(utf8_stringview text, int32 offset) noexcept
    {
      UChar32 c;
      U8_PREV(Bytes(text), 0, offset, c);
      return c;
    }
  }

  uint32 EndOfFirstWordBoundaryContext(utf8_stringview text) noexcept
  {
    int32 length = static_cast<int32>(text.size());
    for (int32 i = 0; i < length;)
    {
      int32 first = i;
      UChar32 ch;
      U8_NEXT(Bytes(text), i, length, ch);
      if (!RequiresContextForWordBoundary(static_cast<char32>(ch)))
      {
        return static_cast<uint32>(first);
      }
    }
    return static_cast<uint32>(length);
  }

  uint32 StartOfLastWordBoundaryContext(utf8_stringview text) noexcept
  {
    int32 length = static_cast<int32>(text.size());
    for (int32 i = length; i > 0;)
    {
      int32 last = i;
      UChar32 ch;
      U8_PREV(Bytes(text), 0, i, ch);
      if (!RequiresContextForWordBoundary(static_cast<char32>(ch)))
      {
        return static_cast<uint32>(last);
      }
    }
    return 0;
  }

  int32 FindNextWordFromIndex(utf8_stringview text, int32 position, bool forward) noexcept
  {
    UBreakIterator *it = WordBreakIterator(text);

    if (forward)
    {
      position = ubrk_following(it, position);
      while (position != UBRK_DONE)
      {
        // We stop searching when the character preceeding the break is alphanumeric.
        if (static_cast<uint32>(position) < text.size() && u_isalnum(CodePointBefore(text, position)))
        {
          return position;
        }

        position = ubrk_following(it, position);
      }

      return static_cast<int32>(text.size());
    }
    else
    {
      position = ubrk_preceding(it, position);
      while (position != UBRK_DONE)
      {
        // We stop searching when the character following the break is alphanumeric.
        if (position && u_isalnum(CodePointAt(text, position)))
        {
          return position;
        }

        position = ubrk_preceding(it, position);
      }

      return 0;
    }
  }

  void FindWordBoundary(utf8_stringview text, int32 position, int32 *start, int32 *end) noexcept
  {
    UBreakIterator *it = WordBreakIterator(text);
    *end = ubrk_following(it, position);
    if (*end < 0)
    {
      *end = ubrk_last(it);
    }
    *start = ubrk_previous(it);
  }

  void FindEndWordBoundary(utf8_stringview text, int32 position, int32 *end) noexcept
  {
    UBreakIterator *it = WordBreakIterator(text);
    *end = ubrk_following(it, position);
    if (*end < 0)
    {
      *end = ubrk_last(it);
    }
  }
}
