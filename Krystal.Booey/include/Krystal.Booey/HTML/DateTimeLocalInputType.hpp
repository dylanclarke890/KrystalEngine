#pragma once

#include "Krystal.Booey/HTML/BaseDateAndTimeInputType.hpp"
#include "Krystal.Booey/HTML/InputTypeNames.hpp"

namespace krys::boo::html
{
  /// @brief The datetimelocal input type.
  class DateTimeLocalInputType final : public BaseDateAndTimeInputType
  {
  public:
    explicit DateTimeLocalInputType(HTMLInputElement &element) noexcept
        : BaseDateAndTimeInputType(Type::DateTimeLocal, element)
    {
    }

    KRYS_NODISCARD dom::DOMStringView FormControlType() const noexcept final
    {
      return InputTypeNames::DateTimeLocal;
    }

    KRYS_NODISCARD DateComponentsType DateType() const noexcept final
    {
      return DateComponentsType::DateTimeLocal;
    }

    KRYS_NODISCARD StepRange CreateStepRange(AnyStepHandling anyStepHandling) const noexcept final;
    KRYS_NODISCARD double ValueAsDate() const noexcept final;
    dom::ExceptionOr<void> SetValueAsDate(double value) const noexcept final;
    KRYS_NODISCARD dom::DOMString SanitizeValue(dom::DOMStringView proposedValue) const noexcept final;

  private:
    KRYS_NODISCARD Maybe<DateComponents>
      ParseToDateComponents(dom::DOMStringView source) const noexcept final;
    KRYS_NODISCARD Maybe<DateComponents> SetMillisecondToDateComponents(double value) const noexcept final;
  };
}
