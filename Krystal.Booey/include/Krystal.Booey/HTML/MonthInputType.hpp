#pragma once

#include "Krystal.Booey/HTML/BaseDateAndTimeInputType.hpp"
#include "Krystal.Booey/HTML/InputTypeNames.hpp"

namespace krys::boo::html
{
  /// @brief The month input type.
  class MonthInputType final : public BaseDateAndTimeInputType
  {
  public:
    explicit MonthInputType(HTMLInputElement &element) noexcept
        : BaseDateAndTimeInputType(Type::Month, element)
    {
    }

    KRYS_NODISCARD dom::DOMStringView FormControlType() const noexcept final
    {
      return InputTypeNames::Month;
    }

    KRYS_NODISCARD DateComponentsType DateType() const noexcept final
    {
      return DateComponentsType::Month;
    }

    KRYS_NODISCARD StepRange CreateStepRange(AnyStepHandling anyStepHandling) const noexcept final;
    KRYS_NODISCARD double ValueAsDate() const noexcept final;
    KRYS_NODISCARD Decimal DefaultValueForStepUp() const noexcept final;
    KRYS_NODISCARD Decimal ParseToNumber(dom::DOMStringView string,
                                         const Decimal &defaultValue) const noexcept final;

  protected:
    KRYS_NODISCARD dom::DOMString SerializeWithMilliseconds(double value) const noexcept final;

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
