/*
 * Copyright (C) 2007 Apple Inc. All rights reserved.
 * Copyright (C) 2008 Tony Chang <idealisms@gmail.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 3.  Neither the name of Apple Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Web/Editing/SmartReplace.hpp"
#include "Krystal.Text/CharacterProperties.hpp"

namespace krys::web
{
  namespace
  {
    /// @brief Whether @p c is in one of the CJK ranges, as CoreFoundation's smart replace has them.
    KRYS_NODISCARD constexpr bool IsSmartReplaceCJK(char32 c) noexcept
    {
      // FIXME: Looks like all these ranges include one extra character past the end.
      return (c >= 0x1100 && c <= 0x1100 + 256)          // Hangul Jamo (0x1100 - 0x11FF)
             || (c >= 0x2E80 && c <= 0x2E80 + 352)       // CJK & Kangxi Radicals (0x2E80 - 0x2FDF)
             || (c >= 0x2FF0 && c <= 0x2FF0 + 464)       // Ideograph Descriptions, CJK Symbols, Hiragana,
                                                         // Katakana, Bopomofo, Hangul Compatibility Jamo,
                                                         // Kanbun, & Bopomofo Ext (0x2FF0 - 0x31BF)
             || (c >= 0x3200 && c <= 0x3200 + 29'392)    // Enclosed CJK, CJK Ideographs (Uni Han & Ext A), &
                                                         // Yi (0x3200 - 0xA4CF)
             || (c >= 0xAC00 && c <= 0xAC00 + 11'183)    // Hangul Syllables (0xAC00 - 0xD7AF)
             || (c >= 0xF900 && c <= 0xF900 + 352)       // CJK Compatibility Ideographs (0xF900 - 0xFA5F)
             || (c >= 0xFE30 && c <= 0xFE30 + 32)        // CJK Compatibility From (0xFE30 - 0xFE4F)
             || (c >= 0xFF00 && c <= 0xFF00 + 240)       // Half/Full Width Form (0xFF00 - 0xFFEF)
             || (c >= 0x20000 && c <= 0x20000 + 0xA6D7)  // CJK Ideograph Exntension B
             || (c >= 0x2F800 && c <= 0x2F800 + 0x021E); // CJK Compatibility Ideographs (0x2F800 - 0x2FA1D)
    }

    /// @brief Whether @p c is whitespace or a newline (kCFCharacterSetWhitespaceAndNewline).
    KRYS_NODISCARD bool IsSmartReplaceWhitespace(char32 c) noexcept
    {
      return text::IsWhiteSpace(c) || c == 0x0A || c == 0x0B || c == 0x0C || c == 0x0D || c == 0x85;
    }
  }

  // This is mostly a port of the CoreFoundation version, with Unicode properties for CoreFoundation's
  // character classes.
  bool IsCharacterSmartReplaceExempt(char32 c, bool isPreviousCharacter) noexcept
  {
    if (IsSmartReplaceWhitespace(c) || IsSmartReplaceCJK(c))
    {
      return true;
    }
    if (isPreviousCharacter)
    {
      return c < 0x80
             && utf8_stringview(u8"([\"'#$/-`{").find(static_cast<char8>(c)) != utf8_stringview::npos;
    }
    // Punctuation (kCFCharacterSetPunctuation).
    return (c < 0x80
            && utf8_stringview(u8")].,;:?'!\"%*-/}").find(static_cast<char8>(c)) != utf8_stringview::npos)
           || text::IsPunctuation(c);
  }
}
