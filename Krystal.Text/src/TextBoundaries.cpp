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

#include "Krystal.Text/TextBoundaries.hpp"
#include "Krystal.Core/Strings/CodePointIterator.hpp"
#include "Krystal.Text/TextBreakIterator.hpp"
#include <unicode/uchar.h>

namespace krys::text
{
  namespace
  {
    /// @brief The code point that ends at @p offset.
    KRYS_NODISCARD char32 CodePointBefore(utf8_stringview text, size_t offset) noexcept
    {
      return CodePointAt(text, PreviousCodePointStart(text, offset));
    }
  }

  bool RequiresContextForWordBoundary(char32 character) noexcept
  {
    // FIXME: This function won't be needed when https://bugs.webkit.org/show_bug.cgi?id=120656 is fixed.

    // We can get rid of this constant if we require a newer version of the ICU headers, which has
    // U_LB_CONDITIONAL_JAPANESE_STARTER.
    constexpr int lineBreakConditionalJapaneseStarter = 37;

    int lineBreak = u_getIntPropertyValue(static_cast<UChar32>(character), UCHAR_LINE_BREAK);
    return lineBreak == U_LB_COMPLEX_CONTEXT || lineBreak == lineBreakConditionalJapaneseStarter
           || lineBreak == U_LB_IDEOGRAPHIC;
  }

  uint32 EndOfFirstWordBoundaryContext(utf8_stringview text) noexcept
  {
    int32 length = static_cast<int32>(text.size());
    for (int32 i = 0; i < length;)
    {
      int32 first = i;
      auto decoded = DecodeUTF8(Span<const char8>(text.data() + i, static_cast<size_t>(length - i)));
      i += decoded.Length;
      if (!RequiresContextForWordBoundary(decoded.CodePoint))
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
      i = static_cast<int32>(PreviousCodePointStart(text, static_cast<size_t>(i)));
      if (!RequiresContextForWordBoundary(CodePointAt(text, static_cast<size_t>(i))))
      {
        return static_cast<uint32>(last);
      }
    }
    return 0;
  }

  int32 FindNextWordFromIndex(utf8_stringview text, int32 position, bool forward) noexcept
  {
    auto it = WordBreakIterator(text);

    if (forward)
    {
      for (auto boundary = it.Following(static_cast<uint32>(position)); boundary;
           boundary = it.Following(*boundary))
      {
        // We stop searching when the character preceeding the break is alphanumeric.
        if (*boundary < text.size() && u_isalnum(CodePointBefore(text, *boundary)))
        {
          return static_cast<int32>(*boundary);
        }
      }

      return static_cast<int32>(text.size());
    }

    for (auto boundary = it.Preceding(static_cast<uint32>(position)); boundary;
         boundary = it.Preceding(*boundary))
    {
      // We stop searching when the character following the break is alphanumeric.
      if (*boundary && u_isalnum(CodePointAt(text, *boundary)))
      {
        return static_cast<int32>(*boundary);
      }
    }

    return 0;
  }

  void FindWordBoundary(utf8_stringview text, int32 position, int32 *start, int32 *end) noexcept
  {
    auto it = WordBreakIterator(text);
    auto following = it.Following(static_cast<uint32>(position));
    *end = static_cast<int32>(following ? *following : it.Last());
    *start = static_cast<int32>(it.Previous().value_or(0));
  }

  void FindEndWordBoundary(utf8_stringview text, int32 position, int32 *end) noexcept
  {
    auto it = WordBreakIterator(text);
    auto following = it.Following(static_cast<uint32>(position));
    *end = static_cast<int32>(following ? *following : it.Last());
  }
}
