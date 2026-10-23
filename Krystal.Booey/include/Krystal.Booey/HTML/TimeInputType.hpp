#pragma once

#include "Krystal.Booey/HTML/BaseDateAndTimeInputType.hpp"
#include "Krystal.Booey/HTML/InputTypeNames.hpp"

namespace krys::boo::html
{
  /// @brief The time input type.
  class TimeInputType final : public BaseDateAndTimeInputType
  {
  public:
    explicit TimeInputType(HTMLInputElement &element) noexcept : BaseDateAndTimeInputType(Type::Time, element)
    {
    }

    KRYS_NODISCARD dom::DOMStringView FormControlType() const noexcept final
    {
      return InputTypeNames::Time;
    }

    KRYS_NODISCARD DateComponentsType DateType() const noexcept final
    {
      return DateComponentsType::Time;
    }

    KRYS_NODISCARD StepRange CreateStepRange(AnyStepHandling anyStepHandling) const noexcept final;
    KRYS_NODISCARD Decimal DefaultValueForStepUp() const noexcept final;

  private:
    KRYS_NODISCARD Maybe<DateComponents>
      ParseToDateComponents(dom::DOMStringView source) const noexcept final;
    KRYS_NODISCARD Maybe<DateComponents> SetMillisecondToDateComponents(double value) const noexcept final;

    // Time inputs have no picker.
    void HandleActivationBehavior(dom::Event &event) noexcept final;
    void ShowPicker() noexcept final;

    // DateTimeEditElementEditControlOwner functions:
    KRYS_NODISCARD dom::DOMString
      FormatDateTimeFieldsState(const DateTimeFieldsState &state) const noexcept final;

    // BaseDateAndTimeInputType functions:
    void SetupLayoutParameters(DateTimeEditElement::LayoutParameters &layoutParameters,
                               const DateComponents &date) const noexcept final;
    KRYS_NODISCARD bool IsValidFormat(EnumSet<DateTimeFormatValidationResults> results) const noexcept final;
  };
}
