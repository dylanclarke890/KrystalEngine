#pragma once

#include "Krystal.Booey/HTML/BaseDateAndTimeInputType.hpp"
#include "Krystal.Booey/HTML/InputTypeNames.hpp"

namespace krys::boo::html
{
  /// @brief The date input type.
  class DateInputType final : public BaseDateAndTimeInputType
  {
  public:
    explicit DateInputType(HTMLInputElement &element) noexcept : BaseDateAndTimeInputType(Type::Date, element)
    {
    }

    KRYS_NODISCARD dom::DOMStringView FormControlType() const noexcept final
    {
      return InputTypeNames::Date;
    }

    KRYS_NODISCARD DateComponentsType DateType() const noexcept final
    {
      return DateComponentsType::Date;
    }

    KRYS_NODISCARD StepRange CreateStepRange(AnyStepHandling anyStepHandling) const noexcept final;

  private:
    KRYS_NODISCARD Maybe<DateComponents>
      ParseToDateComponents(dom::DOMStringView source) const noexcept final;
    KRYS_NODISCARD Maybe<DateComponents> SetMillisecondToDateComponents(double value) const noexcept final;

    // DateTimeEditElementEditControlOwner functions:
    KRYS_NODISCARD dom::DOMString
      FormatDateTimeFieldsState(const DateTimeFieldsState &state) const noexcept final;

    // BaseDateAndTimeInputType functions:
    void SetupLayoutParameters(DateTimeEditElement::LayoutParameters &layoutParameters,
                               const DateComponents &date) const noexcept final;
    KRYS_NODISCARD bool IsValidFormat(EnumSet<DateTimeFormatValidationResults> results) const noexcept final;
  };
}
