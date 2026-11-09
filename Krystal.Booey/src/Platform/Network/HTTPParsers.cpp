/*
 * Copyright (C) 2006 Alexey Proskuryakov (ap@webkit.org)
 * Copyright (C) 2006-2017 Apple Inc. All rights reserved.
 * Copyright (C) 2009 Torch Mobile Inc. http://www.torchmobile.com/
 * Copyright (C) 2009 Google Inc. All rights reserved.
 * Copyright (C) 2011 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Platform/Network/HTTPParsers.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include "Krystal.Core/Types/Array.hpp"

namespace krys::boo
{
  namespace
  {
    constexpr Array<utf8_stringview, 12> HTTPDateMonths {u8"Jan", u8"Feb", u8"Mar", u8"Apr",
                                                         u8"May", u8"Jun", u8"Jul", u8"Aug",
                                                         u8"Sep", u8"Oct", u8"Nov", u8"Dec"};
    constexpr Array<utf8_stringview, 7> HTTPDateDayNames {u8"Mon", u8"Tue", u8"Wed", u8"Thu",
                                                          u8"Fri", u8"Sat", u8"Sun"};
    constexpr Array<utf8_stringview, 7> HTTPDateLongDayNames {
      u8"Monday", u8"Tuesday", u8"Wednesday", u8"Thursday", u8"Friday", u8"Saturday", u8"Sunday"};

    /// @brief Reads an HTTP-date's parts from the front of its text.
    class HTTPDateReader
    {
    private:
      utf8_stringview _text;

    public:
      explicit HTTPDateReader(utf8_stringview text) noexcept : _text(text)
      {
      }

      KRYS_NODISCARD bool AtEnd() const noexcept
      {
        return _text.empty();
      }

      /// @brief Skips @p literal, if it's next.
      bool Skip(utf8_stringview literal) noexcept
      {
        if (!_text.starts_with(literal))
        {
          return false;
        }
        _text.remove_prefix(literal.size());
        return true;
      }

      /// @brief Reads exactly @p digits digits.
      KRYS_NODISCARD Maybe<uint32> Number(size_t digits) noexcept
      {
        if (_text.size() < digits)
        {
          return null;
        }
        uint32 value = 0;
        for (size_t i = 0; i < digits; ++i)
        {
          if (!text::IsASCIIDigit(_text[i]))
          {
            return null;
          }
          value = value * 10 + static_cast<uint32>(_text[i] - u8'0');
        }
        _text.remove_prefix(digits);
        return value;
      }

      /// @brief Reads one of @p names, the index of the one read.
      template <size_t N>
      KRYS_NODISCARD Maybe<size_t> OneOf(const Array<utf8_stringview, N> &names) noexcept
      {
        // The longest first, so "Monday" isn't read as "Mon".
        Maybe<size_t> best;
        for (size_t i = 0; i < N; ++i)
        {
          if (_text.starts_with(names[i]) && (!best || names[i].size() > names[*best].size()))
          {
            best = i;
          }
        }
        if (best)
        {
          _text.remove_prefix(names[*best].size());
        }
        return best;
      }

      /// @brief Reads a time of day, `hh:mm:ss`, as seconds; a leap second is allowed.
      KRYS_NODISCARD Maybe<std::chrono::seconds> TimeOfDay() noexcept
      {
        auto hour = Number(2);
        if (!hour || *hour > 23 || !Skip(u8":"))
        {
          return null;
        }
        auto minute = Number(2);
        if (!minute || *minute > 59 || !Skip(u8":"))
        {
          return null;
        }
        auto second = Number(2);
        if (!second || *second > 60)
        {
          return null;
        }
        return std::chrono::hours(*hour) + std::chrono::minutes(*minute) + std::chrono::seconds(*second);
      }
    };

    KRYS_NODISCARD Maybe<std::chrono::sys_seconds> HTTPDateTime(int32 year, size_t month, uint32 day,
                                                                std::chrono::seconds timeOfDay) noexcept
    {
      std::chrono::year_month_day date {
        std::chrono::year(year), std::chrono::month(static_cast<uint32>(month) + 1), std::chrono::day(day)};
      if (!date.ok())
      {
        return null;
      }
      return std::chrono::sys_days(date) + timeOfDay;
    }

    /// @brief The year an RFC 850 date's two digits name: one more than 50 years ahead is the most recent
    /// past year with those digits.
    KRYS_NODISCARD int32 RFC850Year(uint32 twoDigits) noexcept
    {
      auto now =
        std::chrono::year_month_day(std::chrono::floor<std::chrono::days>(std::chrono::system_clock::now()));
      int32 currentYear = static_cast<int32>(now.year());
      int32 year = currentYear - currentYear % 100 + static_cast<int32>(twoDigits);
      if (year > currentYear + 50)
      {
        year -= 100;
      }
      else if (year <= currentYear - 50)
      {
        year += 100;
      }
      return year;
    }
  }

  Maybe<std::chrono::sys_seconds> ParseHTTPDate(utf8_stringview value) noexcept
  {
    // IMF-fixdate: day-name "," SP day SP month SP year SP time-of-day SP "GMT".
    {
      HTTPDateReader reader(value);
      if (reader.OneOf(HTTPDateDayNames) && reader.Skip(u8", "))
      {
        auto day = reader.Number(2);
        if (day && reader.Skip(u8" "))
        {
          auto month = reader.OneOf(HTTPDateMonths);
          if (month && reader.Skip(u8" "))
          {
            auto year = reader.Number(4);
            if (year && reader.Skip(u8" "))
            {
              auto time = reader.TimeOfDay();
              if (time && reader.Skip(u8" GMT") && reader.AtEnd())
              {
                return HTTPDateTime(static_cast<int32>(*year), *month, *day, *time);
              }
            }
          }
        }
      }
    }

    // rfc850-date: day-name-l "," SP day "-" month "-" 2DIGIT SP time-of-day SP "GMT".
    {
      HTTPDateReader reader(value);
      if (reader.OneOf(HTTPDateLongDayNames) && reader.Skip(u8", "))
      {
        auto day = reader.Number(2);
        if (day && reader.Skip(u8"-"))
        {
          auto month = reader.OneOf(HTTPDateMonths);
          if (month && reader.Skip(u8"-"))
          {
            auto year = reader.Number(2);
            if (year && reader.Skip(u8" "))
            {
              auto time = reader.TimeOfDay();
              if (time && reader.Skip(u8" GMT") && reader.AtEnd())
              {
                return HTTPDateTime(RFC850Year(*year), *month, *day, *time);
              }
            }
          }
        }
      }
    }

    // asctime-date: day-name SP month SP ( 2DIGIT / ( SP DIGIT ) ) SP time-of-day SP year.
    {
      HTTPDateReader reader(value);
      if (reader.OneOf(HTTPDateDayNames) && reader.Skip(u8" "))
      {
        auto month = reader.OneOf(HTTPDateMonths);
        if (month && reader.Skip(u8" "))
        {
          auto day = reader.Skip(u8" ") ? reader.Number(1) : reader.Number(2);
          if (day && reader.Skip(u8" "))
          {
            auto time = reader.TimeOfDay();
            if (time && reader.Skip(u8" "))
            {
              auto year = reader.Number(4);
              if (year && reader.AtEnd())
              {
                return HTTPDateTime(static_cast<int32>(*year), *month, *day, *time);
              }
            }
          }
        }
      }
    }

    return null;
  }

  bool IsNosniffContentTypeOptions(utf8_stringview value) noexcept
  {
    // Get, decode, and split the header: its first value, without the HTTP whitespace around it.
    auto first = value.substr(0, value.find(u8','));
    auto isHTTPTabOrSpace = [](char8 c)
    {
      return c == u8' ' || c == u8'\t';
    };
    while (!first.empty() && isHTTPTabOrSpace(first.front()))
    {
      first.remove_prefix(1);
    }
    while (!first.empty() && isHTTPTabOrSpace(first.back()))
    {
      first.remove_suffix(1);
    }
    return text::IsASCIICaselessEqual(first, u8"nosniff");
  }
}
