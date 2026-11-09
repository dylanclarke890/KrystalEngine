/*
 * Copyright (C) 2005, 2006, 2007, 2014 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Graphics/TextRuns/StringTruncator.hpp"
#include "Krystal.Booey/Graphics/Fonts/FontCascade.hpp"
#include "Krystal.Booey/Graphics/TextRuns/TextRun.hpp"
#include "Krystal.Booey/Text/TextBreakIterator.hpp"
#include <unicode/ubrk.h>

namespace krys::boo
{
  namespace
  {
    constexpr uint32 StringBufferSize = 2'048;

    constexpr utf8_stringview HorizontalEllipsis = u8"\u2026";

    using TruncationFunction = utf8_string(utf8_stringview string, uint32 length, uint32 keepCount,
                                           bool shouldInsertEllipsis);

    KRYS_NODISCARD uint32 TextBreakAtOrPreceding(UBreakIterator *it, uint32 offset) noexcept
    {
      if (ubrk_isBoundary(it, static_cast<int32_t>(offset)))
      {
        return offset;
      }

      int32_t result = ubrk_preceding(it, static_cast<int32_t>(offset));
      return result == UBRK_DONE ? 0 : static_cast<uint32>(result);
    }

    KRYS_NODISCARD uint32 BoundedTextBreakFollowing(UBreakIterator *it, uint32 offset, uint32 length) noexcept
    {
      int32_t result = ubrk_following(it, static_cast<int32_t>(offset));
      return result == UBRK_DONE ? length : static_cast<uint32>(result);
    }

    KRYS_NODISCARD utf8_string CenterTruncateToBuffer(utf8_stringview string, uint32 length, uint32 keepCount,
                                                      bool shouldInsertEllipsis) noexcept
    {
      krys_assert(keepCount < length);
      krys_assert(keepCount < StringBufferSize);

      uint32 omitStart = (keepCount + 1) / 2;
      NonSharedCharacterBreakIterator it(string.substr(0, length));
      uint32 omitEnd = BoundedTextBreakFollowing(it, omitStart + (length - keepCount) - 1, length);
      omitStart = TextBreakAtOrPreceding(it, omitStart);

      utf8_string buffer(string.substr(0, omitStart));
      if (shouldInsertEllipsis)
      {
        buffer += HorizontalEllipsis;
      }
      buffer += string.substr(omitEnd, length - omitEnd);
      return buffer;
    }

    KRYS_NODISCARD utf8_string RightTruncateToBuffer(utf8_stringview string, uint32 length, uint32 keepCount,
                                                     bool shouldInsertEllipsis) noexcept
    {
      krys_assert(keepCount < length);
      krys_assert(keepCount < StringBufferSize);

      NonSharedCharacterBreakIterator it(string.substr(0, length));
      uint32 keepLength = TextBreakAtOrPreceding(it, keepCount);

      utf8_string buffer(string.substr(0, keepLength));
      if (shouldInsertEllipsis)
      {
        buffer += HorizontalEllipsis;
      }
      return buffer;
    }

    KRYS_NODISCARD utf8_string RightClipToCharacterBuffer(utf8_stringview string, uint32 length,
                                                          uint32 keepCount, bool) noexcept
    {
      krys_debug_assert(keepCount < length);
      krys_debug_assert(keepCount < StringBufferSize);

      NonSharedCharacterBreakIterator it(string.substr(0, length));
      uint32 keepLength = TextBreakAtOrPreceding(it, keepCount);
      return utf8_string(string.substr(0, keepLength));
    }

    KRYS_NODISCARD utf8_string RightClipToWordBuffer(utf8_stringview string, uint32 length, uint32 keepCount,
                                                     bool) noexcept
    {
      krys_debug_assert(keepCount < length);
      krys_debug_assert(keepCount < StringBufferSize);

      UBreakIterator *it = WordBreakIterator(string.substr(0, length));
      uint32 keepLength = TextBreakAtOrPreceding(it, keepCount);
      return utf8_string(string.substr(0, keepLength));
    }

    KRYS_NODISCARD utf8_string LeftTruncateToBuffer(utf8_stringview string, uint32 length, uint32 keepCount,
                                                    bool shouldInsertEllipsis) noexcept
    {
      krys_debug_assert(keepCount < length);
      krys_debug_assert(keepCount < StringBufferSize);

      uint32 startIndex = length - keepCount;

      NonSharedCharacterBreakIterator it(string);
      uint32 adjustedStartIndex = ubrk_isBoundary(it, static_cast<int32_t>(startIndex))
                                    ? startIndex
                                    : BoundedTextBreakFollowing(it, startIndex, length);

      // Strip single character after ellipsis character, when that character is preceded by a space
      if (adjustedStartIndex < length && string[adjustedStartIndex] != u8' '
          && adjustedStartIndex < length - 1 && string[adjustedStartIndex + 1] == u8' ')
      {
        ++adjustedStartIndex;
      }

      // Strip whitespace after the ellipsis character
      while (adjustedStartIndex < length && string[adjustedStartIndex] == u8' ')
      {
        ++adjustedStartIndex;
      }

      utf8_string buffer;
      if (shouldInsertEllipsis)
      {
        buffer += HorizontalEllipsis;
      }
      buffer += string.substr(adjustedStartIndex, length - adjustedStartIndex);
      return buffer;
    }

    KRYS_NODISCARD float StringWidth(const FontCascade &renderer, utf8_stringview characters) noexcept
    {
      TextRun run(characters);
      return renderer.Width(run);
    }

    KRYS_NODISCARD utf8_string TruncateString(utf8_stringview string, float maxWidth, const FontCascade &font,
                                              TruncationFunction truncateToBuffer,
                                              float *resultWidth = nullptr, bool shouldInsertEllipsis = true,
                                              float customTruncationElementWidth = 0,
                                              bool alwaysTruncate = false) noexcept
    {
      if (string.empty())
      {
        return utf8_string(string);
      }

      if (resultWidth)
      {
        *resultWidth = 0;
      }

      krys_debug_assert(maxWidth >= 0);

      float currentEllipsisWidth =
        shouldInsertEllipsis ? StringWidth(font, HorizontalEllipsis) : customTruncationElementWidth;

      utf8_string stringBuffer;
      uint32 keepCount;
      auto length = static_cast<uint32>(string.size());

      if (length > StringBufferSize)
      {
        if (shouldInsertEllipsis)
        {
          keepCount = StringBufferSize - 1; // need 1 character for the ellipsis
        }
        else
        {
          keepCount = 0;
        }
        stringBuffer = CenterTruncateToBuffer(string, length, keepCount, shouldInsertEllipsis);
      }
      else
      {
        keepCount = length;
        stringBuffer = utf8_string(string);
      }

      float width = StringWidth(font, stringBuffer);
      if (!shouldInsertEllipsis && alwaysTruncate)
      {
        width += customTruncationElementWidth;
      }
      if ((width - maxWidth) < 0.0001) // Ignore rounding errors.
      {
        if (resultWidth)
        {
          *resultWidth = width;
        }
        return utf8_string(string);
      }

      uint32 keepCountForLargestKnownToFit = 0;
      float widthForLargestKnownToFit = currentEllipsisWidth;

      uint32 keepCountForSmallestKnownToNotFit = keepCount;
      float widthForSmallestKnownToNotFit = width;

      if (currentEllipsisWidth >= maxWidth)
      {
        keepCountForLargestKnownToFit = 1;
        keepCountForSmallestKnownToNotFit = 2;
      }

      while (keepCountForLargestKnownToFit + 1 < keepCountForSmallestKnownToNotFit)
      {
        krys_assert(widthForLargestKnownToFit <= maxWidth);
        krys_assert(widthForSmallestKnownToNotFit > maxWidth);

        float ratio = (keepCountForSmallestKnownToNotFit - keepCountForLargestKnownToFit)
                      / (widthForSmallestKnownToNotFit - widthForLargestKnownToFit);
        keepCount = static_cast<uint32>(maxWidth * ratio);

        if (keepCount <= keepCountForLargestKnownToFit)
        {
          keepCount = keepCountForLargestKnownToFit + 1;
        }
        else if (keepCount >= keepCountForSmallestKnownToNotFit)
        {
          keepCount = keepCountForSmallestKnownToNotFit - 1;
        }

        krys_assert(keepCount < length);
        krys_debug_assert(keepCount > 0);
        krys_assert(keepCount < keepCountForSmallestKnownToNotFit);
        krys_assert(keepCount > keepCountForLargestKnownToFit);

        stringBuffer = truncateToBuffer(string, length, keepCount, shouldInsertEllipsis);

        width = StringWidth(font, stringBuffer);
        if (!shouldInsertEllipsis)
        {
          width += customTruncationElementWidth;
        }
        if (width <= maxWidth)
        {
          keepCountForLargestKnownToFit = keepCount;
          widthForLargestKnownToFit = width;
          if (resultWidth)
          {
            *resultWidth = width;
          }
        }
        else
        {
          keepCountForSmallestKnownToNotFit = keepCount;
          widthForSmallestKnownToNotFit = width;
        }
      }

      if (keepCountForLargestKnownToFit == 0)
      {
        keepCountForLargestKnownToFit = 1;
      }

      if (keepCount != keepCountForLargestKnownToFit)
      {
        keepCount = keepCountForLargestKnownToFit;
        stringBuffer = truncateToBuffer(string, length, keepCount, shouldInsertEllipsis);
      }

      return stringBuffer;
    }
  }

  utf8_string StringTruncator::CenterTruncate(utf8_stringview string, float maxWidth,
                                              const FontCascade &font) noexcept
  {
    return TruncateString(string, maxWidth, font, CenterTruncateToBuffer);
  }

  utf8_string StringTruncator::RightTruncate(utf8_stringview string, float maxWidth,
                                             const FontCascade &font) noexcept
  {
    return TruncateString(string, maxWidth, font, RightTruncateToBuffer);
  }

  float StringTruncator::Width(utf8_stringview string, const FontCascade &font) noexcept
  {
    return StringWidth(font, string);
  }

  utf8_string StringTruncator::CenterTruncate(utf8_stringview string, float maxWidth, const FontCascade &font,
                                              float &resultWidth, bool shouldInsertEllipsis,
                                              float customTruncationElementWidth) noexcept
  {
    return TruncateString(string, maxWidth, font, CenterTruncateToBuffer, &resultWidth, shouldInsertEllipsis,
                          customTruncationElementWidth);
  }

  utf8_string StringTruncator::RightTruncate(utf8_stringview string, float maxWidth, const FontCascade &font,
                                             float &resultWidth, bool shouldInsertEllipsis,
                                             float customTruncationElementWidth) noexcept
  {
    return TruncateString(string, maxWidth, font, RightTruncateToBuffer, &resultWidth, shouldInsertEllipsis,
                          customTruncationElementWidth);
  }

  utf8_string StringTruncator::LeftTruncate(utf8_stringview string, float maxWidth, const FontCascade &font,
                                            float &resultWidth, bool shouldInsertEllipsis,
                                            float customTruncationElementWidth) noexcept
  {
    return TruncateString(string, maxWidth, font, LeftTruncateToBuffer, &resultWidth, shouldInsertEllipsis,
                          customTruncationElementWidth);
  }

  utf8_string StringTruncator::RightClipToCharacter(utf8_stringview string, float maxWidth,
                                                    const FontCascade &font, float &resultWidth,
                                                    bool shouldInsertEllipsis,
                                                    float customTruncationElementWidth) noexcept
  {
    return TruncateString(string, maxWidth, font, RightClipToCharacterBuffer, &resultWidth,
                          shouldInsertEllipsis, customTruncationElementWidth);
  }

  utf8_string StringTruncator::RightClipToWord(utf8_stringview string, float maxWidth,
                                               const FontCascade &font, float &resultWidth,
                                               bool shouldInsertEllipsis, float customTruncationElementWidth,
                                               bool alwaysTruncate) noexcept
  {
    return TruncateString(string, maxWidth, font, RightClipToWordBuffer, &resultWidth, shouldInsertEllipsis,
                          customTruncationElementWidth, alwaysTruncate);
  }
}
