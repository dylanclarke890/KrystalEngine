#pragma once

#include "Krystal.Booey/HTML/InputType.hpp"

namespace krys::boo::html
{
  /// @brief The base of the date and time input types: date, datetime-local, month, time and week.
  // FIXME: The editable fields (DateTimeEditElement), the pickers (the chrome client's date and time
  // chooser), bad input from partly filled fields, and localised values come with the other controls.
  class BaseDateAndTimeInputType : public InputType
  {
  public:
    KRYS_NODISCARD double ValueAsDate() const noexcept override;
    dom::ExceptionOr<void> SetValueAsDate(double value) const noexcept override;
    KRYS_NODISCARD double ValueAsDouble() const noexcept final;
    dom::ExceptionOr<void> SetValueAsDecimal(const Decimal &value,
                                             TextFieldEventBehavior eventBehavior) const noexcept final;
    KRYS_NODISCARD bool TypeMismatchFor(dom::DOMStringView value) const noexcept final;
    KRYS_NODISCARD bool TypeMismatch() const noexcept final;
    KRYS_NODISCARD Decimal DefaultValueForStepUp() const noexcept override;
    KRYS_NODISCARD Decimal ParseToNumber(dom::DOMStringView string,
                                         const Decimal &defaultValue) const noexcept override;
    KRYS_NODISCARD dom::DOMString Serialize(const Decimal &value) const noexcept final;
    KRYS_NODISCARD dom::DOMString SanitizeValue(dom::DOMStringView proposedValue) const noexcept override;
    KRYS_NODISCARD bool SupportsReadOnly() const noexcept final;
    KRYS_NODISCARD bool ValueMissing(dom::DOMStringView value) const noexcept final;

  protected:
    BaseDateAndTimeInputType(Type type, HTMLInputElement &element) noexcept : InputType(type, element)
    {
    }

    KRYS_NODISCARD dom::DOMString SerializeWithComponents(const DateComponents &date) const noexcept;
    KRYS_NODISCARD virtual dom::DOMString SerializeWithMilliseconds(double value) const noexcept;

    /// @brief The local time now, in milliseconds since the epoch.
    KRYS_NODISCARD static double LocalTimeNow() noexcept;

  private:
    KRYS_NODISCARD virtual Maybe<DateComponents>
      ParseToDateComponents(dom::DOMStringView source) const noexcept = 0;
    KRYS_NODISCARD virtual Maybe<DateComponents>
      SetMillisecondToDateComponents(double value) const noexcept = 0;
  };
}
