/*
 * Copyright (C) 2025 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/Text/UnicodeHelpers.hpp"
#include "Krystal.Core/Text/CodePointIterator.hpp"
#include <algorithm>
#include <unicode/uchar.h>

namespace krys::boo
{
  Maybe<TextDirection> BaseTextDirection(utf8_stringview content) noexcept
  {
    // Latin-1 text is left to right.
    if (std::ranges::all_of(content, [](char8 c) { return c < 0x80; }))
    {
      return TextDirection::LTR;
    }

    // The direction of the first strong character, as ubidi_getBaseDirection.
    bool isLatin1 = true;
    for (text::CodePointIterator it {content}; !it.AtEnd(); ++it)
    {
      isLatin1 = isLatin1 && *it <= 0xFF;
      switch (u_charDirection(static_cast<UChar32>(*it)))
      {
        case U_LEFT_TO_RIGHT:        return TextDirection::LTR;
        case U_RIGHT_TO_LEFT:
        case U_RIGHT_TO_LEFT_ARABIC: return TextDirection::RTL;
        default:                     break;
      }
    }
    if (isLatin1)
    {
      return TextDirection::LTR;
    }
    return null;
  }

  Maybe<TextDirection> DefaultWritingDirection(utf8_stringview content) noexcept
  {
    for (text::CodePointIterator it {content}; !it.AtEnd(); ++it)
    {
      switch (u_charDirection(static_cast<UChar32>(*it)))
      {
        case U_LEFT_TO_RIGHT:        return TextDirection::LTR;
        case U_RIGHT_TO_LEFT:
        case U_RIGHT_TO_LEFT_ARABIC: return TextDirection::RTL;
        default:                     break;
      }
    }
    return null;
  }
}
