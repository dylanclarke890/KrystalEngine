#pragma once

#include "Krystal.Booey/Core/PlatformLocale.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include <string>
#include <unicode/udat.h>
#include <unicode/unum.h>

namespace krys::boo
{
  /// @brief A locale whose data comes from ICU.
  class LocaleICU final : public Locale
  {
  private:
    std::string _locale;

    UNumberFormat *_numberFormat {nullptr};
    bool _didCreateDecimalFormat {false};

    Maybe<List<utf8_string>> _monthLabels;
    Maybe<utf8_string> _dateFormat;
    Maybe<utf8_string> _monthFormat;
    Maybe<utf8_string> _shortMonthFormat;
    utf8_string _timeFormatWithSeconds;
    utf8_string _timeFormatWithoutSeconds;
    utf8_string _dateTimeFormatWithSeconds;
    utf8_string _dateTimeFormatWithoutSeconds;
    UDateFormat *_shortDateFormat {nullptr};
    UDateFormat *_mediumTimeFormat {nullptr};
    UDateFormat *_shortTimeFormat {nullptr};
    List<utf8_string> _shortMonthLabels;
    List<utf8_string> _standAloneMonthLabels;
    List<utf8_string> _shortStandAloneMonthLabels;
    List<utf8_string> _timeAMPMLabels;
    bool _didCreateShortDateFormat {false};
    bool _didCreateTimeFormat {false};

  public:
    explicit LocaleICU(std::string locale) noexcept;
    ~LocaleICU() noexcept override;

    KRYS_NODISCARD WritingDirection DefaultWritingDirection() const noexcept override;

    KRYS_NODISCARD utf8_string DateFormat() noexcept override;
    KRYS_NODISCARD utf8_string MonthFormat() noexcept override;
    KRYS_NODISCARD utf8_string ShortMonthFormat() noexcept override;
    KRYS_NODISCARD utf8_string TimeFormat() noexcept override;
    KRYS_NODISCARD utf8_string ShortTimeFormat() noexcept override;
    KRYS_NODISCARD utf8_string DateTimeFormatWithSeconds() noexcept override;
    KRYS_NODISCARD utf8_string DateTimeFormatWithoutSeconds() noexcept override;
    KRYS_NODISCARD const List<utf8_string> &MonthLabels() noexcept override;
    KRYS_NODISCARD const List<utf8_string> &ShortMonthLabels() noexcept override;
    KRYS_NODISCARD const List<utf8_string> &StandAloneMonthLabels() noexcept override;
    KRYS_NODISCARD const List<utf8_string> &ShortStandAloneMonthLabels() noexcept override;
    KRYS_NODISCARD const List<utf8_string> &TimeAMPMLabels() noexcept override;

  private:
    KRYS_NODISCARD utf8_string DecimalSymbol(UNumberFormatSymbol symbol) const noexcept;
    KRYS_NODISCARD utf8_string DecimalTextAttribute(UNumberFormatTextAttribute tag) const noexcept;
    void InitializeLocaleData() noexcept override;

    bool InitializeShortDateFormat() noexcept;
    KRYS_NODISCARD UDateFormat *OpenDateFormat(UDateFormatStyle timeStyle,
                                               UDateFormatStyle dateStyle) const noexcept;

    /// @brief The @p size labels of @p type from @p startIndex, or null if ICU doesn't have them.
    KRYS_NODISCARD static Maybe<List<utf8_string>> CreateLabelVector(const UDateFormat *dateFormat,
                                                                     UDateFormatSymbolType type,
                                                                     int32_t startIndex,
                                                                     int32_t size) noexcept;
    void InitializeDateTimeFormat() noexcept;
  };
}
