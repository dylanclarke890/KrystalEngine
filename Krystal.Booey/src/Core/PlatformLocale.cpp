#include "Krystal.Booey/Core/PlatformLocale.hpp"
#include "Krystal.Booey/Core/DateTimeFormat.hpp"
#include "Krystal.Booey/Platform/DateComponents.hpp"
#include "Krystal.Core/Assert.hpp"
#include <format>
#include <unicode/uchar.h>
#include <unicode/utf8.h>

namespace krys::boo
{
  namespace
  {
    class DateTimeStringBuilder final : private DateTimeFormat::TokenHandler
    {
      KRYS_NON_COPYABLE(DateTimeStringBuilder);

    private:
      utf8_string _builder;
      Locale &_localizer;
      const DateComponents &_date;

    public:
      // The argument objects must be alive until this object dies.
      DateTimeStringBuilder(Locale &localizer, const DateComponents &date) noexcept
          : _localizer(localizer), _date(date)
      {
      }

      bool Build(utf8_stringview formatString) noexcept
      {
        _builder.reserve(formatString.size());
        return DateTimeFormat::Parse(formatString, *this);
      }

      KRYS_NODISCARD utf8_string ToString() noexcept
      {
        return krys::move(_builder);
      }

    private:
      void VisitField(DateTimeFormat::FieldType fieldType, int numberOfPatternCharacters) noexcept final
      {
        using FieldType = DateTimeFormat::FieldType;
        switch (fieldType)
        {
          case FieldType::Year:
            // Always use padding width of 4 so it matches DateTimeEditElement.
            AppendNumber(_date.FullYear(), 4);
            return;
          case FieldType::Month:
            if (numberOfPatternCharacters == 3)
            {
              _builder.append(_localizer.ShortMonthLabels()[static_cast<size_t>(_date.Month())]);
            }
            else if (numberOfPatternCharacters == 4)
            {
              _builder.append(_localizer.MonthLabels()[static_cast<size_t>(_date.Month())]);
            }
            else
            {
              // Always use padding width of 2 so it matches DateTimeEditElement.
              AppendNumber(_date.Month() + 1, 2);
            }
            return;
          case FieldType::MonthStandAlone:
            if (numberOfPatternCharacters == 3)
            {
              _builder.append(_localizer.ShortStandAloneMonthLabels()[static_cast<size_t>(_date.Month())]);
            }
            else if (numberOfPatternCharacters == 4)
            {
              _builder.append(_localizer.StandAloneMonthLabels()[static_cast<size_t>(_date.Month())]);
            }
            else
            {
              // Always use padding width of 2 so it matches DateTimeEditElement.
              AppendNumber(_date.Month() + 1, 2);
            }
            return;
          case FieldType::DayOfMonth:
            // Always use padding width of 2 so it matches DateTimeEditElement.
            AppendNumber(_date.MonthDay(), 2);
            return;
          case FieldType::WeekOfYear:
            // Always use padding width of 2 so it matches DateTimeEditElement.
            AppendNumber(_date.Week(), 2);
            return;
          case FieldType::Period:
            _builder.append(_localizer.TimeAMPMLabels()[_date.Hour() >= 12 ? 1 : 0]);
            return;
          case FieldType::Hour12:
          {
            int hour12 = _date.Hour() % 12;
            if (!hour12)
            {
              hour12 = 12;
            }
            AppendNumber(hour12, static_cast<size_t>(numberOfPatternCharacters));
            return;
          }
          case FieldType::Hour23:
            AppendNumber(_date.Hour(), static_cast<size_t>(numberOfPatternCharacters));
            return;
          case FieldType::Hour11:
            AppendNumber(_date.Hour() % 12, static_cast<size_t>(numberOfPatternCharacters));
            return;
          case FieldType::Hour24:
          {
            int hour24 = _date.Hour();
            if (!hour24)
            {
              hour24 = 24;
            }
            AppendNumber(hour24, static_cast<size_t>(numberOfPatternCharacters));
            return;
          }
          case FieldType::Minute:
            AppendNumber(_date.Minute(), static_cast<size_t>(numberOfPatternCharacters));
            return;
          case FieldType::Second:
            if (!_date.Millisecond())
            {
              AppendNumber(_date.Second(), static_cast<size_t>(numberOfPatternCharacters));
            }
            else
            {
              double second = _date.Second() + _date.Millisecond() / 1000.0;
              auto formatted = std::format("{:.3f}", second);
              auto zeroPaddedSecondString = ZeroPadString(
                utf8_stringview(reinterpret_cast<const char8 *>(formatted.data()), formatted.size()),
                static_cast<size_t>(numberOfPatternCharacters) + 4);
              _builder.append(_localizer.ConvertToLocalizedNumber(zeroPaddedSecondString));
            }
            return;
          default: return;
        }
      }

      void VisitLiteral(utf8_stringview text) noexcept final
      {
        krys_debug_assert(!text.empty());
        _builder.append(text);
      }

      KRYS_NODISCARD static utf8_string ZeroPadString(utf8_stringview string, size_t width) noexcept
      {
        if (string.size() >= width)
        {
          return utf8_string(string);
        }
        utf8_string zeroPadded(width - string.size(), u8'0');
        zeroPadded.append(string);
        return zeroPadded;
      }

      void AppendNumber(int number, size_t width) noexcept
      {
        auto digits = std::to_string(number);
        auto zeroPaddedNumberString = ZeroPadString(
          utf8_stringview(reinterpret_cast<const char8 *>(digits.data()), digits.size()), width);
        _builder.append(_localizer.ConvertToLocalizedNumber(zeroPaddedNumberString));
      }
    };

    KRYS_NODISCARD bool Matches(utf8_stringview text, size_t position, utf8_stringview part) noexcept
    {
      if (part.empty())
      {
        return true;
      }
      if (position + part.size() > text.size())
      {
        return false;
      }
      return text.substr(position, part.size()) == part;
    }

    KRYS_NODISCARD bool IsSpaceOrNewline(UChar32 c) noexcept
    {
      if (c <= 0x7F)
      {
        return c == ' ' || (c >= '\t' && c <= '\r');
      }
      return u_charDirection(c) == U_WHITE_SPACE_NEUTRAL;
    }

    /// @brief @p text without the white space at its ends.
    KRYS_NODISCARD utf8_stringview Trim(utf8_stringview text) noexcept
    {
      auto *bytes = reinterpret_cast<const uint8_t *>(text.data());
      auto length = static_cast<int32>(text.size());
      int32 start = 0;
      while (start < length)
      {
        int32 next = start;
        UChar32 c;
        U8_NEXT(bytes, next, length, c);
        if (c < 0 || !IsSpaceOrNewline(c))
        {
          break;
        }
        start = next;
      }
      int32 end = length;
      while (end > start)
      {
        int32 previous = end;
        UChar32 c;
        U8_PREV(bytes, start, previous, c);
        if (c < 0 || !IsSpaceOrNewline(c))
        {
          break;
        }
        end = previous;
      }
      return text.substr(static_cast<size_t>(start), static_cast<size_t>(end - start));
    }
  }

  UniquePtr<Locale> Locale::CreateDefault() noexcept
  {
    return Create(DefaultLanguage());
  }

  Locale::~Locale() noexcept = default;

  void Locale::SetLocaleData(const std::array<utf8_string, DecimalSymbolsSize> &symbols,
                             utf8_stringview positivePrefix, utf8_stringview positiveSuffix,
                             utf8_stringview negativePrefix, utf8_stringview negativeSuffix) noexcept
  {
    for (size_t i = 0; i < symbols.size(); ++i)
    {
      krys_debug_assert(!symbols[i].empty());
      _decimalSymbols[i] = symbols[i];
    }
    _positivePrefix = positivePrefix;
    _positiveSuffix = positiveSuffix;
    _negativePrefix = negativePrefix;
    _negativeSuffix = negativeSuffix;
    krys_debug_assert(!_positivePrefix.empty() || !_positiveSuffix.empty() || !_negativePrefix.empty()
                      || !_negativeSuffix.empty());
    _hasLocaleData = true;
  }

  utf8_string Locale::ConvertToLocalizedNumber(utf8_stringview input) noexcept
  {
    InitializeLocaleData();
    if (!_hasLocaleData || input.empty())
    {
      return utf8_string(input);
    }

    size_t i = 0;
    bool isNegative = false;
    utf8_string builder;
    builder.reserve(input.size());

    if (input[0] == '-')
    {
      ++i;
      isNegative = true;
      builder.append(_negativePrefix);
    }
    else
    {
      builder.append(_positivePrefix);
    }

    for (; i < input.size(); ++i)
    {
      auto ch = input[i];
      if (ch >= '0' && ch <= '9')
      {
        builder.append(_decimalSymbols[static_cast<size_t>(ch - '0')]);
      }
      else if (ch == '.')
      {
        builder.append(_decimalSymbols[DecimalSeparatorIndex]);
      }
      else
      {
        krys_debug_assert(false);
      }
    }

    builder.append(isNegative ? _negativeSuffix : _positiveSuffix);

    return builder;
  }

  bool Locale::DetectSignAndGetDigitRange(utf8_stringview input, bool &isNegative, size_t &startIndex,
                                          size_t &endIndex) const noexcept
  {
    startIndex = 0;
    endIndex = input.size();
    if (_negativePrefix.empty() && _negativeSuffix.empty())
    {
      if (input.starts_with(_positivePrefix) && input.ends_with(_positiveSuffix))
      {
        isNegative = false;
        startIndex = _positivePrefix.size();
        endIndex -= _positiveSuffix.size();
      }
      else
      {
        isNegative = true;
      }
    }
    else
    {
      if (input.starts_with(_negativePrefix) && input.ends_with(_negativeSuffix))
      {
        isNegative = true;
        startIndex = _negativePrefix.size();
        endIndex -= _negativeSuffix.size();
      }
      else
      {
        isNegative = false;
        if (input.starts_with(_positivePrefix) && input.ends_with(_positiveSuffix))
        {
          startIndex = _positivePrefix.size();
          endIndex -= _positiveSuffix.size();
        }
        else
        {
          return false;
        }
      }
    }
    return true;
  }

  size_t Locale::MatchedDecimalSymbolIndex(utf8_stringview input, size_t &position) const noexcept
  {
    for (size_t symbolIndex = 0; symbolIndex < DecimalSymbolsSize; ++symbolIndex)
    {
      if (!_decimalSymbols[symbolIndex].empty() && Matches(input, position, _decimalSymbols[symbolIndex]))
      {
        position += _decimalSymbols[symbolIndex].size();
        return symbolIndex;
      }
    }
    return DecimalSymbolsSize;
  }

  utf8_string Locale::ConvertFromLocalizedNumber(utf8_stringview localized) noexcept
  {
    InitializeLocaleData();
    auto input = Trim(localized);
    if (!_hasLocaleData || input.empty())
    {
      return utf8_string(input);
    }

    bool isNegative;
    size_t startIndex;
    size_t endIndex;
    if (!DetectSignAndGetDigitRange(input, isNegative, startIndex, endIndex))
    {
      return utf8_string(input);
    }

    // Ignore leading '+', but will reject '+'-only string later.
    if (!isNegative && endIndex - startIndex >= 2 && input[startIndex] == '+')
    {
      ++startIndex;
    }

    utf8_string builder;
    builder.reserve(input.size());
    if (isNegative)
    {
      builder.push_back('-');
    }
    for (size_t i = startIndex; i < endIndex;)
    {
      size_t symbolIndex = MatchedDecimalSymbolIndex(input, i);
      if (symbolIndex >= DecimalSymbolsSize)
      {
        return utf8_string(input);
      }
      if (symbolIndex == DecimalSeparatorIndex)
      {
        builder.push_back('.');
      }
      else if (symbolIndex == GroupSeparatorIndex)
      {
        return utf8_string(input);
      }
      else
      {
        builder.push_back(static_cast<char8>('0' + symbolIndex));
      }
    }
    // Ignore trailing '.', but will reject '.'-only string later.
    if (builder.size() >= 2 && builder.back() == '.')
    {
      builder.pop_back();
    }
    return builder;
  }

  Locale::WritingDirection Locale::DefaultWritingDirection() const noexcept
  {
    return WritingDirection::Default;
  }

  utf8_string Locale::FormatDateTime(const DateComponents &date, FormatType formatType) noexcept
  {
    if (date.Type() == DateComponentsType::Invalid)
    {
      return {};
    }

    DateTimeStringBuilder builder(*this, date);
    switch (date.Type())
    {
      case DateComponentsType::Time:
        builder.Build(formatType == FormatType::Short ? ShortTimeFormat() : TimeFormat());
        break;
      case DateComponentsType::Date: builder.Build(DateFormat()); break;
      case DateComponentsType::Month:
        builder.Build(formatType == FormatType::Short ? ShortMonthFormat() : MonthFormat());
        break;
      case DateComponentsType::Week:
        // FIXME: Add support for formatting weeks.
        break;
      case DateComponentsType::DateTimeLocal:
        builder.Build(formatType == FormatType::Short ? DateTimeFormatWithoutSeconds()
                                                      : DateTimeFormatWithSeconds());
        break;
      case DateComponentsType::Invalid: krys_debug_assert(false); break;
    }
    return builder.ToString();
  }

  const utf8_string &Locale::LocalizedDecimalSeparator() noexcept
  {
    InitializeLocaleData();
    return _decimalSymbols[DecimalSeparatorIndex];
  }
}
