#pragma once

#include "Krystal.Booey/HTML/BaseDateAndTimeInputType.hpp"
#include "Krystal.Booey/HTML/InputTypeNames.hpp"

namespace krys::boo::html
{
  /// @brief The week input type.
  class WeekInputType final : public BaseDateAndTimeInputType
  {
  public:
    explicit WeekInputType(HTMLInputElement &element) noexcept : BaseDateAndTimeInputType(Type::Week, element)
    {
    }

    KRYS_NODISCARD dom::DOMStringView FormControlType() const noexcept final
    {
      return InputTypeNames::Week;
    }

    KRYS_NODISCARD DateComponentsType DateType() const noexcept final
    {
      return DateComponentsType::Week;
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
