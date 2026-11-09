/*
 * Copyright (C) 2012 Google Inc. All rights reserved.
 * Copyright (C) 2020 Apple Inc. All rights reserved.
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

#pragma once

#include "Krystal.Booey/HTML/Shadow/DateTimeNumericFieldElement.hpp"
#include "Krystal.Booey/HTML/Shadow/DateTimeSymbolicFieldElement.hpp"

namespace krys::boo::html
{
  /// @brief The day of the month field.
  class DateTimeDayFieldElement final : public DateTimeNumericFieldElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(DateTimeDayFieldElement);

  public:
    DateTimeDayFieldElement(dom::Document &document, DateTimeFieldElementFieldOwner &fieldOwner) noexcept;
    KRYS_NODISCARD static Ref<DateTimeDayFieldElement>
      Create(dom::Document &document, DateTimeFieldElementFieldOwner &fieldOwner) noexcept;

  private:
    // DateTimeFieldElement functions:
    void SetValueAsDate(const DateComponents &date) noexcept final;
    void PopulateDateTimeFieldsState(DateTimeFieldsState &state,
                                     DateTimePlaceholderIfNoValue placeholderIfNoValue) noexcept final;
  };

  /// @brief The hour field, from 0 or 1 to 11, 12, 23 or 24 as the locale's format says.
  class DateTimeHourFieldElement final : public DateTimeNumericFieldElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(DateTimeHourFieldElement);

  public:
    DateTimeHourFieldElement(dom::Document &document, DateTimeFieldElementFieldOwner &fieldOwner, int minimum,
                             int maximum) noexcept;
    KRYS_NODISCARD static Ref<DateTimeHourFieldElement> Create(dom::Document &document,
                                                               DateTimeFieldElementFieldOwner &fieldOwner,
                                                               int minimum, int maximum) noexcept;

  private:
    // DateTimeFieldElement functions:
    void SetValueAsDate(const DateComponents &date) noexcept final;
    void PopulateDateTimeFieldsState(DateTimeFieldsState &state,
                                     DateTimePlaceholderIfNoValue placeholderIfNoValue) noexcept final;
  };

  /// @brief The AM/PM field.
  class DateTimeMeridiemFieldElement final : public DateTimeSymbolicFieldElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(DateTimeMeridiemFieldElement);

  public:
    DateTimeMeridiemFieldElement(dom::Document &document, DateTimeFieldElementFieldOwner &fieldOwner,
                                 const List<utf8_string> &labels) noexcept;
    KRYS_NODISCARD static Ref<DateTimeMeridiemFieldElement> Create(dom::Document &document,
                                                                   DateTimeFieldElementFieldOwner &fieldOwner,
                                                                   const List<utf8_string> &labels) noexcept;

  private:
    // DateTimeFieldElement functions:
    void SetValueAsDate(const DateComponents &date) noexcept final;
    void PopulateDateTimeFieldsState(DateTimeFieldsState &state,
                                     DateTimePlaceholderIfNoValue placeholderIfNoValue) noexcept final;
  };

  /// @brief The millisecond field.
  class DateTimeMillisecondFieldElement final : public DateTimeNumericFieldElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(DateTimeMillisecondFieldElement);

  public:
    DateTimeMillisecondFieldElement(dom::Document &document,
                                    DateTimeFieldElementFieldOwner &fieldOwner) noexcept;
    KRYS_NODISCARD static Ref<DateTimeMillisecondFieldElement>
      Create(dom::Document &document, DateTimeFieldElementFieldOwner &fieldOwner) noexcept;

  private:
    // DateTimeFieldElement functions:
    void SetValueAsDate(const DateComponents &date) noexcept final;
    void PopulateDateTimeFieldsState(DateTimeFieldsState &state,
                                     DateTimePlaceholderIfNoValue placeholderIfNoValue) noexcept final;
  };

  /// @brief The minute field.
  class DateTimeMinuteFieldElement final : public DateTimeNumericFieldElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(DateTimeMinuteFieldElement);

  public:
    DateTimeMinuteFieldElement(dom::Document &document, DateTimeFieldElementFieldOwner &fieldOwner) noexcept;
    KRYS_NODISCARD static Ref<DateTimeMinuteFieldElement>
      Create(dom::Document &document, DateTimeFieldElementFieldOwner &fieldOwner) noexcept;

  private:
    // DateTimeFieldElement functions:
    void SetValueAsDate(const DateComponents &date) noexcept final;
    void PopulateDateTimeFieldsState(DateTimeFieldsState &state,
                                     DateTimePlaceholderIfNoValue placeholderIfNoValue) noexcept final;
  };

  /// @brief The month field, as a number.
  class DateTimeMonthFieldElement final : public DateTimeNumericFieldElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(DateTimeMonthFieldElement);

  public:
    DateTimeMonthFieldElement(dom::Document &document, DateTimeFieldElementFieldOwner &fieldOwner) noexcept;
    KRYS_NODISCARD static Ref<DateTimeMonthFieldElement>
      Create(dom::Document &document, DateTimeFieldElementFieldOwner &fieldOwner) noexcept;

  private:
    // DateTimeFieldElement functions:
    void SetValueAsDate(const DateComponents &date) noexcept final;
    void PopulateDateTimeFieldsState(DateTimeFieldsState &state,
                                     DateTimePlaceholderIfNoValue placeholderIfNoValue) noexcept final;
  };

  /// @brief The second field.
  class DateTimeSecondFieldElement final : public DateTimeNumericFieldElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(DateTimeSecondFieldElement);

  public:
    DateTimeSecondFieldElement(dom::Document &document, DateTimeFieldElementFieldOwner &fieldOwner) noexcept;
    KRYS_NODISCARD static Ref<DateTimeSecondFieldElement>
      Create(dom::Document &document, DateTimeFieldElementFieldOwner &fieldOwner) noexcept;

  private:
    // DateTimeFieldElement functions:
    void SetValueAsDate(const DateComponents &date) noexcept final;
    void PopulateDateTimeFieldsState(DateTimeFieldsState &state,
                                     DateTimePlaceholderIfNoValue placeholderIfNoValue) noexcept final;
  };

  /// @brief The month field, as a name.
  class DateTimeSymbolicMonthFieldElement final : public DateTimeSymbolicFieldElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(DateTimeSymbolicMonthFieldElement);

  public:
    DateTimeSymbolicMonthFieldElement(dom::Document &document, DateTimeFieldElementFieldOwner &fieldOwner,
                                      const List<utf8_string> &labels) noexcept;
    KRYS_NODISCARD static Ref<DateTimeSymbolicMonthFieldElement>
      Create(dom::Document &document, DateTimeFieldElementFieldOwner &fieldOwner,
             const List<utf8_string> &labels) noexcept;

  private:
    // DateTimeFieldElement functions:
    void SetValueAsDate(const DateComponents &date) noexcept final;
    void PopulateDateTimeFieldsState(DateTimeFieldsState &state,
                                     DateTimePlaceholderIfNoValue placeholderIfNoValue) noexcept final;
  };

  /// @brief The week of the year field, of week inputs.
  class DateTimeWeekFieldElement final : public DateTimeNumericFieldElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(DateTimeWeekFieldElement);

  public:
    DateTimeWeekFieldElement(dom::Document &document, DateTimeFieldElementFieldOwner &fieldOwner) noexcept;
    KRYS_NODISCARD static Ref<DateTimeWeekFieldElement>
      Create(dom::Document &document, DateTimeFieldElementFieldOwner &fieldOwner) noexcept;

  private:
    // DateTimeFieldElement functions:
    void SetValueAsDate(const DateComponents &date) noexcept final;
    void PopulateDateTimeFieldsState(DateTimeFieldsState &state,
                                     DateTimePlaceholderIfNoValue placeholderIfNoValue) noexcept final;
  };

  /// @brief The year field.
  class DateTimeYearFieldElement final : public DateTimeNumericFieldElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(DateTimeYearFieldElement);

  public:
    DateTimeYearFieldElement(dom::Document &document, DateTimeFieldElementFieldOwner &fieldOwner) noexcept;
    KRYS_NODISCARD static Ref<DateTimeYearFieldElement>
      Create(dom::Document &document, DateTimeFieldElementFieldOwner &fieldOwner) noexcept;

    void StepDown() noexcept final;
    void StepUp() noexcept final;

  private:
    // DateTimeFieldElement functions:
    void SetValueAsDate(const DateComponents &date) noexcept final;
    void PopulateDateTimeFieldsState(DateTimeFieldsState &state,
                                     DateTimePlaceholderIfNoValue placeholderIfNoValue) noexcept final;
  };
}
