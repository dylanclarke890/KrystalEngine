/*
 * Copyright (C) 2012 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 */

#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/String.hpp"

namespace krys::boo
{
  /// @brief Parses date time formats defined in Unicode Technical Standard 35, Locale Data Markup Language
  /// (LDML).
  /// @see http://unicode.org/reports/tr35/tr35-6.html#Date_Format_Patterns
  class DateTimeFormat
  {
  public:
    enum class FieldType : char
    {
      Invalid,
      Literal,

      // Era: AD
      Era = 'G',

      // Year: 1996
      Year = 'y',
      YearOfWeekOfYear = 'Y',
      ExtendedYear = 'u',

      // Quarter: Q2
      Quarter = 'Q',
      QuarterStandAlone = 'q',

      // Month: September
      Month = 'M',
      MonthStandAlone = 'L',

      // Week: 42
      WeekOfYear = 'w',
      WeekOfMonth = 'W',

      // Day: 12
      DayOfMonth = 'd',
      DayOfYear = 'D',
      DayOfWeekInMonth = 'F',
      ModifiedJulianDay = 'g',

      // Week Day: Tuesday
      DayOfWeek = 'E',
      LocalDayOfWeek = 'e',
      LocalDayOfWeekStandAlone = 'c',

      // Period: AM or PM
      Period = 'a',

      // Hour: 7
      Hour12 = 'h',
      Hour23 = 'H',
      Hour11 = 'K',
      Hour24 = 'k',

      // Minute: 59
      Minute = 'm',

      // Second: 12
      Second = 's',
      FractionalSecond = 'S',
      MillisecondsInDay = 'A',

      // Zone: PDT
      Zone = 'z',
      RFC822Zone = 'Z',
      NonLocationZone = 'v',
    };

    class TokenHandler
    {
    public:
      virtual ~TokenHandler() noexcept = default;
      virtual void VisitField(FieldType fieldType, int numberOfPatternCharacters) noexcept = 0;
      virtual void VisitLiteral(utf8_stringview text) noexcept = 0;
    };

    /// @return Whether @p source parsed.
    static bool Parse(utf8_stringview source, TokenHandler &tokenHandler) noexcept;
    static void QuoteAndAppendLiteral(utf8_stringview literal, utf8_string &buffer) noexcept;
  };
}
