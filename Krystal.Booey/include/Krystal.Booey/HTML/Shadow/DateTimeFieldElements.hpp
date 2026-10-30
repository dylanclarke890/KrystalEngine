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
