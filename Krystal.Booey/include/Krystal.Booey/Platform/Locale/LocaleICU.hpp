/*
 * Copyright (C) 2012 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/Platform/Locale/PlatformLocale.hpp"
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
