#include "Krystal.Booey/Core/LocaleICU.hpp"
#include "Krystal.Booey/Core/LocaleToScriptMapping.hpp"
#include "Krystal.Booey/Platform/Language.hpp"
#include "Krystal.Core/Assert.hpp"
#include <array>
#include <hb.h>
#include <unicode/udatpg.h>
#include <unicode/uloc.h>
#include <unicode/ustring.h>

namespace krys::boo
{
  namespace
  {
    /// @brief @p characters (UTF-16) in UTF-8.
    KRYS_NODISCARD utf8_string ToUTF8(const UChar *characters, int32_t length) noexcept
    {
      UErrorCode status = U_ZERO_ERROR;
      int32_t utf8Length = 0;
      u_strToUTF8(nullptr, 0, &utf8Length, characters, length, &status);
      if (status != U_BUFFER_OVERFLOW_ERROR && U_FAILURE(status))
      {
        return {};
      }
      utf8_string result(static_cast<size_t>(utf8Length), u8'\0');
      status = U_ZERO_ERROR;
      u_strToUTF8(reinterpret_cast<char *>(result.data()), utf8Length, nullptr, characters, length, &status);
      if (U_FAILURE(status))
      {
        return {};
      }
      return result;
    }

    KRYS_NODISCARD bool NeedsToGrowToProduceBuffer(UErrorCode status) noexcept
    {
      return status == U_BUFFER_OVERFLOW_ERROR;
    }

    KRYS_NODISCARD utf8_string GetDateFormatPattern(const UDateFormat *dateFormat) noexcept
    {
      if (!dateFormat)
      {
        return {};
      }

      UErrorCode status = U_ZERO_ERROR;
      int32_t length = udat_toPattern(dateFormat, true, nullptr, 0, &status);
      if (!NeedsToGrowToProduceBuffer(status) || !length)
      {
        return {};
      }
      std::u16string buffer(static_cast<size_t>(length), u'\0');
      status = U_ZERO_ERROR;
      udat_toPattern(dateFormat, true, buffer.data(), length, &status);
      if (U_FAILURE(status))
      {
        return {};
      }
      return ToUTF8(buffer.data(), length);
    }

    KRYS_NODISCARD utf8_string GetFormatForSkeleton(const std::string &locale,
                                                    std::u16string_view skeleton) noexcept
    {
      utf8_string format = u8"yyyy-MM";
      UErrorCode status = U_ZERO_ERROR;
      UDateTimePatternGenerator *patternGenerator = udatpg_open(locale.c_str(), &status);
      if (!patternGenerator)
      {
        return format;
      }
      status = U_ZERO_ERROR;
      auto skeletonLength = static_cast<int32_t>(skeleton.size());
      int32_t length =
        udatpg_getBestPattern(patternGenerator, skeleton.data(), skeletonLength, nullptr, 0, &status);
      if (NeedsToGrowToProduceBuffer(status) && length)
      {
        std::u16string buffer(static_cast<size_t>(length), u'\0');
        status = U_ZERO_ERROR;
        udatpg_getBestPattern(patternGenerator, skeleton.data(), skeletonLength, buffer.data(), length,
                              &status);
        if (U_SUCCESS(status))
        {
          format = ToUTF8(buffer.data(), length);
        }
      }
      udatpg_close(patternGenerator);
      return format;
    }

    constexpr std::array<utf8_stringview, 12> MonthFullName {
      u8"January", u8"February", u8"March",     u8"April",   u8"May",      u8"June",
      u8"July",    u8"August",   u8"September", u8"October", u8"November", u8"December"};

    constexpr std::array<utf8_stringview, 12> MonthName {u8"Jan", u8"Feb", u8"Mar", u8"Apr",
                                                         u8"May", u8"Jun", u8"Jul", u8"Aug",
                                                         u8"Sep", u8"Oct", u8"Nov", u8"Dec"};

    KRYS_NODISCARD List<utf8_string> CreateFallbackMonthLabels() noexcept
    {
      return List<utf8_string>(MonthFullName.begin(), MonthFullName.end());
    }

    KRYS_NODISCARD List<utf8_string> CreateFallbackAMPMLabels() noexcept
    {
      return List<utf8_string> {u8"AM", u8"PM"};
    }
  }

  UniquePtr<Locale> Locale::Create(utf8_stringview locale) noexcept
  {
    return CreateUnique<LocaleICU>(std::string(reinterpret_cast<const char *>(locale.data()), locale.size()));
  }

  utf8_string DefaultLanguage() noexcept
  {
    // The host's preferred languages come first.
    if (auto &override = UserPreferredLanguagesOverride(); !override.empty())
    {
      return override.front();
    }

    std::array<char, ULOC_FULLNAME_CAPACITY> tag {};
    UErrorCode status = U_ZERO_ERROR;
    int32_t length =
      uloc_toLanguageTag(uloc_getDefault(), tag.data(), static_cast<int32_t>(tag.size()), false, &status);
    if (U_FAILURE(status) || length <= 0)
    {
      return u8"en-US";
    }
    return utf8_string(reinterpret_cast<const char8 *>(tag.data()), static_cast<size_t>(length));
  }

  LocaleICU::LocaleICU(std::string locale) noexcept : _locale(krys::move(locale))
  {
  }

  LocaleICU::~LocaleICU() noexcept
  {
    unum_close(_numberFormat);
    udat_close(_shortDateFormat);
    udat_close(_mediumTimeFormat);
    udat_close(_shortTimeFormat);
  }

  Locale::WritingDirection LocaleICU::DefaultWritingDirection() const noexcept
  {
    UScriptCode icuScript =
      LocaleToScriptCode(utf8_stringview(reinterpret_cast<const char8 *>(_locale.data()), _locale.size()));
    if (icuScript == USCRIPT_INVALID_CODE)
    {
      return WritingDirection::Default;
    }
    hb_script_t script = hb_script_from_string(uscript_getShortName(icuScript), -1);

    switch (hb_script_get_horizontal_direction(script))
    {
      case HB_DIRECTION_LTR: return WritingDirection::LeftToRight;
      case HB_DIRECTION_RTL: return WritingDirection::RightToLeft;
      default:               return WritingDirection::Default;
    }
  }

  utf8_string LocaleICU::DecimalSymbol(UNumberFormatSymbol symbol) const noexcept
  {
    UErrorCode status = U_ZERO_ERROR;
    int32_t bufferLength = unum_getSymbol(_numberFormat, symbol, nullptr, 0, &status);
    krys_debug_assert(U_SUCCESS(status) || NeedsToGrowToProduceBuffer(status));
    if (U_FAILURE(status) && !NeedsToGrowToProduceBuffer(status))
    {
      return {};
    }
    std::u16string buffer(static_cast<size_t>(bufferLength), u'\0');
    status = U_ZERO_ERROR;
    unum_getSymbol(_numberFormat, symbol, buffer.data(), bufferLength, &status);
    if (U_FAILURE(status))
    {
      return {};
    }
    return ToUTF8(buffer.data(), bufferLength);
  }

  utf8_string LocaleICU::DecimalTextAttribute(UNumberFormatTextAttribute tag) const noexcept
  {
    UErrorCode status = U_ZERO_ERROR;
    int32_t bufferLength = unum_getTextAttribute(_numberFormat, tag, nullptr, 0, &status);
    krys_debug_assert(U_SUCCESS(status) || NeedsToGrowToProduceBuffer(status));
    if (U_FAILURE(status) && !NeedsToGrowToProduceBuffer(status))
    {
      return {};
    }
    std::u16string buffer(static_cast<size_t>(bufferLength), u'\0');
    status = U_ZERO_ERROR;
    unum_getTextAttribute(_numberFormat, tag, buffer.data(), bufferLength, &status);
    krys_debug_assert(U_SUCCESS(status));
    if (U_FAILURE(status))
    {
      return {};
    }
    return ToUTF8(buffer.data(), bufferLength);
  }

  void LocaleICU::InitializeLocaleData() noexcept
  {
    if (_didCreateDecimalFormat)
    {
      return;
    }
    _didCreateDecimalFormat = true;
    UErrorCode status = U_ZERO_ERROR;
    _numberFormat = unum_open(UNUM_DECIMAL, nullptr, 0, _locale.c_str(), nullptr, &status);
    if (!U_SUCCESS(status))
    {
      return;
    }

    std::array<utf8_string, DecimalSymbolsSize> symbols {
      DecimalSymbol(UNUM_ZERO_DIGIT_SYMBOL),        DecimalSymbol(UNUM_ONE_DIGIT_SYMBOL),
      DecimalSymbol(UNUM_TWO_DIGIT_SYMBOL),         DecimalSymbol(UNUM_THREE_DIGIT_SYMBOL),
      DecimalSymbol(UNUM_FOUR_DIGIT_SYMBOL),        DecimalSymbol(UNUM_FIVE_DIGIT_SYMBOL),
      DecimalSymbol(UNUM_SIX_DIGIT_SYMBOL),         DecimalSymbol(UNUM_SEVEN_DIGIT_SYMBOL),
      DecimalSymbol(UNUM_EIGHT_DIGIT_SYMBOL),       DecimalSymbol(UNUM_NINE_DIGIT_SYMBOL),
      DecimalSymbol(UNUM_DECIMAL_SEPARATOR_SYMBOL), DecimalSymbol(UNUM_GROUPING_SEPARATOR_SYMBOL),
    };
    SetLocaleData(symbols, DecimalTextAttribute(UNUM_POSITIVE_PREFIX),
                  DecimalTextAttribute(UNUM_POSITIVE_SUFFIX), DecimalTextAttribute(UNUM_NEGATIVE_PREFIX),
                  DecimalTextAttribute(UNUM_NEGATIVE_SUFFIX));
  }

  bool LocaleICU::InitializeShortDateFormat() noexcept
  {
    if (_didCreateShortDateFormat)
    {
      return _shortDateFormat;
    }
    _shortDateFormat = OpenDateFormat(UDAT_NONE, UDAT_SHORT);
    _didCreateShortDateFormat = true;
    return _shortDateFormat;
  }

  UDateFormat *LocaleICU::OpenDateFormat(UDateFormatStyle timeStyle,
                                         UDateFormatStyle dateStyle) const noexcept
  {
    constexpr std::array<UChar, 3> gmtTimezone {u'G', u'M', u'T'};
    UErrorCode status = U_ZERO_ERROR;
    return udat_open(timeStyle, dateStyle, _locale.c_str(), gmtTimezone.data(),
                     static_cast<int32_t>(gmtTimezone.size()), nullptr, -1, &status);
  }

  Maybe<List<utf8_string>> LocaleICU::CreateLabelVector(const UDateFormat *dateFormat,
                                                        UDateFormatSymbolType type, int32_t startIndex,
                                                        int32_t size) noexcept
  {
    if (!dateFormat)
    {
      return null;
    }
    if (udat_countSymbols(dateFormat, type) != startIndex + size)
    {
      return null;
    }

    List<utf8_string> labels;
    labels.reserve(static_cast<size_t>(size));
    for (int32_t i = 0; i < size; ++i)
    {
      UErrorCode status = U_ZERO_ERROR;
      int32_t length = udat_getSymbols(dateFormat, type, startIndex + i, nullptr, 0, &status);
      if (!NeedsToGrowToProduceBuffer(status))
      {
        return null;
      }
      std::u16string buffer(static_cast<size_t>(length), u'\0');
      status = U_ZERO_ERROR;
      udat_getSymbols(dateFormat, type, startIndex + i, buffer.data(), length, &status);
      if (U_FAILURE(status))
      {
        return null;
      }
      labels.push_back(ToUTF8(buffer.data(), length));
    }
    return labels;
  }

  const List<utf8_string> &LocaleICU::MonthLabels() noexcept
  {
    if (_monthLabels)
    {
      return *_monthLabels;
    }
    if (InitializeShortDateFormat())
    {
      _monthLabels = CreateLabelVector(_shortDateFormat, UDAT_MONTHS, UCAL_JANUARY, 12);
      if (_monthLabels)
      {
        return *_monthLabels;
      }
    }
    _monthLabels = CreateFallbackMonthLabels();
    return *_monthLabels;
  }

  void LocaleICU::InitializeDateTimeFormat() noexcept
  {
    if (_didCreateTimeFormat)
    {
      return;
    }

    // We assume ICU medium time pattern and short time pattern are compatible with LDML, because ICU
    // specific pattern character "V" doesn't appear in both medium and short time pattern.
    _mediumTimeFormat = OpenDateFormat(UDAT_MEDIUM, UDAT_NONE);
    _timeFormatWithSeconds = GetDateFormatPattern(_mediumTimeFormat);

    _shortTimeFormat = OpenDateFormat(UDAT_SHORT, UDAT_NONE);
    _timeFormatWithoutSeconds = GetDateFormatPattern(_shortTimeFormat);

    UDateFormat *dateTimeFormatWithSeconds = OpenDateFormat(UDAT_MEDIUM, UDAT_SHORT);
    _dateTimeFormatWithSeconds = GetDateFormatPattern(dateTimeFormatWithSeconds);
    udat_close(dateTimeFormatWithSeconds);

    UDateFormat *dateTimeFormatWithoutSeconds = OpenDateFormat(UDAT_SHORT, UDAT_SHORT);
    _dateTimeFormatWithoutSeconds = GetDateFormatPattern(dateTimeFormatWithoutSeconds);
    udat_close(dateTimeFormatWithoutSeconds);

    auto timeAMPMLabels = CreateLabelVector(_mediumTimeFormat, UDAT_AM_PMS, UCAL_AM, 2);
    _timeAMPMLabels = timeAMPMLabels ? krys::move(*timeAMPMLabels) : CreateFallbackAMPMLabels();

    _didCreateTimeFormat = true;
  }

  utf8_string LocaleICU::DateFormat() noexcept
  {
    if (_dateFormat)
    {
      return *_dateFormat;
    }
    if (!InitializeShortDateFormat())
    {
      return u8"yyyy-MM-dd";
    }
    _dateFormat = GetDateFormatPattern(_shortDateFormat);
    return *_dateFormat;
  }

  utf8_string LocaleICU::MonthFormat() noexcept
  {
    if (!_monthFormat)
    {
      // Gets a format for "MMMM" because Windows API always provides formats for "MMMM" in some locales.
      _monthFormat = GetFormatForSkeleton(_locale, u"yyyyMMMM");
    }
    return *_monthFormat;
  }

  utf8_string LocaleICU::ShortMonthFormat() noexcept
  {
    if (!_shortMonthFormat)
    {
      _shortMonthFormat = GetFormatForSkeleton(_locale, u"yyyyMMM");
    }
    return *_shortMonthFormat;
  }

  utf8_string LocaleICU::TimeFormat() noexcept
  {
    InitializeDateTimeFormat();
    return _timeFormatWithSeconds;
  }

  utf8_string LocaleICU::ShortTimeFormat() noexcept
  {
    InitializeDateTimeFormat();
    return _timeFormatWithoutSeconds;
  }

  utf8_string LocaleICU::DateTimeFormatWithSeconds() noexcept
  {
    InitializeDateTimeFormat();
    return _dateTimeFormatWithSeconds;
  }

  utf8_string LocaleICU::DateTimeFormatWithoutSeconds() noexcept
  {
    InitializeDateTimeFormat();
    return _dateTimeFormatWithoutSeconds;
  }

  const List<utf8_string> &LocaleICU::ShortMonthLabels() noexcept
  {
    if (!_shortMonthLabels.empty())
    {
      return _shortMonthLabels;
    }
    if (InitializeShortDateFormat())
    {
      if (auto labels = CreateLabelVector(_shortDateFormat, UDAT_SHORT_MONTHS, UCAL_JANUARY, 12))
      {
        _shortMonthLabels = krys::move(*labels);
        return _shortMonthLabels;
      }
    }
    _shortMonthLabels.assign(MonthName.begin(), MonthName.end());
    return _shortMonthLabels;
  }

  const List<utf8_string> &LocaleICU::StandAloneMonthLabels() noexcept
  {
    if (!_standAloneMonthLabels.empty())
    {
      return _standAloneMonthLabels;
    }
    if (InitializeShortDateFormat())
    {
      if (auto labels = CreateLabelVector(_shortDateFormat, UDAT_STANDALONE_MONTHS, UCAL_JANUARY, 12))
      {
        _standAloneMonthLabels = krys::move(*labels);
        return _standAloneMonthLabels;
      }
    }
    _standAloneMonthLabels = MonthLabels();
    return _standAloneMonthLabels;
  }

  const List<utf8_string> &LocaleICU::ShortStandAloneMonthLabels() noexcept
  {
    if (!_shortStandAloneMonthLabels.empty())
    {
      return _shortStandAloneMonthLabels;
    }
    if (InitializeShortDateFormat())
    {
      if (auto labels = CreateLabelVector(_shortDateFormat, UDAT_STANDALONE_SHORT_MONTHS, UCAL_JANUARY, 12))
      {
        _shortStandAloneMonthLabels = krys::move(*labels);
        return _shortStandAloneMonthLabels;
      }
    }
    _shortStandAloneMonthLabels = ShortMonthLabels();
    return _shortStandAloneMonthLabels;
  }

  const List<utf8_string> &LocaleICU::TimeAMPMLabels() noexcept
  {
    InitializeDateTimeFormat();
    return _timeAMPMLabels;
  }
}
