#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/String.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"
#include <array>

namespace krys::boo
{
  class DateComponents;

  /// @brief The language of the user interface: the system's default locale, as a BCP 47 language tag.
  KRYS_NODISCARD utf8_string DefaultLanguage() noexcept;

  /// @brief A locale's numbers, date formats and month and period labels, for form controls.
  class Locale
  {
    KRYS_NON_COPYABLE(Locale);

  public:
    enum class WritingDirection : uint8
    {
      Default,
      LeftToRight,
      RightToLeft
    };

    enum class FormatType : uint8
    {
      Unspecified,
      Short,
      Medium
    };

  protected:
    enum
    {
      // 0-9 for digits.
      DecimalSeparatorIndex = 10,
      GroupSeparatorIndex = 11,
      DecimalSymbolsSize
    };

  private:
    std::array<utf8_string, DecimalSymbolsSize> _decimalSymbols;
    utf8_string _positivePrefix;
    utf8_string _positiveSuffix;
    utf8_string _negativePrefix;
    utf8_string _negativeSuffix;
    bool _hasLocaleData {false};

  public:
    KRYS_NODISCARD static UniquePtr<Locale> Create(utf8_stringview localeIdentifier) noexcept;
    KRYS_NODISCARD static UniquePtr<Locale> CreateDefault() noexcept;

    virtual ~Locale() noexcept;

    /// @brief Converts the specified number string to another number string localized for this locale. The
    /// input string must conform to HTML floating-point numbers, and is not empty.
    KRYS_NODISCARD utf8_string ConvertToLocalizedNumber(utf8_stringview input) noexcept;

    /// @brief Converts the specified localized number string to a number string in the HTML floating-point
    /// number format. The input string is provided by a end user, and might not be a number string. It's ok
    /// that the function returns a string which is not conforms to the HTML floating-point number format,
    /// callers of this function are responsible to check the format of the resultant string.
    KRYS_NODISCARD utf8_string ConvertFromLocalizedNumber(utf8_stringview localized) noexcept;

    /// @brief The default writing direction for the locale.
    KRYS_NODISCARD virtual WritingDirection DefaultWritingDirection() const noexcept;

    /// @brief The date format in Unicode TR35 LDML containing day of month, month, and year, e.g.
    /// "dd/mm/yyyy".
    /// @see http://unicode.org/reports/tr35/#Date_Format_Patterns
    KRYS_NODISCARD virtual utf8_string DateFormat() noexcept = 0;

    /// @brief A year-month format in Unicode TR35 LDML.
    KRYS_NODISCARD virtual utf8_string MonthFormat() noexcept = 0;

    /// @brief A year-month format using short month labels in Unicode TR35 LDML.
    KRYS_NODISCARD virtual utf8_string ShortMonthFormat() noexcept = 0;

    /// @brief The time format in Unicode TR35 LDML containing hour, minute, and second with optional
    /// period (AM/PM), e.g. "h:mm:ss a".
    KRYS_NODISCARD virtual utf8_string TimeFormat() noexcept = 0;

    /// @brief The time format in Unicode TR35 LDML containing hour, and minute with optional period
    /// (AM/PM), e.g. "h:mm a".
    /// @note Some platforms return the same value as TimeFormat().
    KRYS_NODISCARD virtual utf8_string ShortTimeFormat() noexcept = 0;

    /// @brief A date-time format in Unicode TR35 LDML. It should have a seconds field.
    KRYS_NODISCARD virtual utf8_string DateTimeFormatWithSeconds() noexcept = 0;

    /// @brief A date-time format in Unicode TR35 LDML. It should have no seconds field.
    KRYS_NODISCARD virtual utf8_string DateTimeFormatWithoutSeconds() noexcept = 0;

    /// @brief Twelve strings: the first is a localized string of Jan and the last a localized string of
    /// Dec. These strings should be short.
    KRYS_NODISCARD virtual const List<utf8_string> &ShortMonthLabels() noexcept = 0;

    /// @brief Twelve strings: the first is a stand-alone localized string of January and the last a
    /// stand-alone localized string of December. These strings should not be abbreviations.
    KRYS_NODISCARD virtual const List<utf8_string> &StandAloneMonthLabels() noexcept = 0;

    /// @brief The stand-alone version of ShortMonthLabels().
    KRYS_NODISCARD virtual const List<utf8_string> &ShortStandAloneMonthLabels() noexcept = 0;

    /// @brief The localized period field (AM/PM) strings.
    KRYS_NODISCARD virtual const List<utf8_string> &TimeAMPMLabels() noexcept = 0;

    /// @brief Twelve strings: the first is a localized string of January, and the last a localized string
    /// of December. These strings should not be abbreviations.
    KRYS_NODISCARD virtual const List<utf8_string> &MonthLabels() noexcept = 0;

    KRYS_NODISCARD const utf8_string &LocalizedDecimalSeparator() noexcept;

    /// @brief Serializes the specified date into a formatted date string to display to the user. If an
    /// implementation doesn't support localized dates the function should return an empty string.
    /// @p formatType can be used to specify if you want the short format.
    KRYS_NODISCARD virtual utf8_string
      FormatDateTime(const DateComponents &date, FormatType formatType = FormatType::Unspecified) noexcept;

  protected:
    Locale() noexcept = default;
    virtual void InitializeLocaleData() noexcept = 0;
    void SetLocaleData(const std::array<utf8_string, DecimalSymbolsSize> &symbols,
                       utf8_stringview positivePrefix, utf8_stringview positiveSuffix,
                       utf8_stringview negativePrefix, utf8_stringview negativeSuffix) noexcept;

  private:
    KRYS_NODISCARD bool DetectSignAndGetDigitRange(utf8_stringview input, bool &isNegative,
                                                   size_t &startIndex, size_t &endIndex) const noexcept;
    KRYS_NODISCARD size_t MatchedDecimalSymbolIndex(utf8_stringview input, size_t &position) const noexcept;
  };
}
