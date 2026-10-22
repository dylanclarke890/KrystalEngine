#pragma once

#include "Krystal.Booey/HTML/InputTypeNames.hpp"
#include "Krystal.Booey/HTML/TextFieldInputType.hpp"

namespace krys::boo::html
{
  /// @brief The number input type.
  // FIXME: The spin button, bad input from what the user typed, and localised numbers come with text fields.
  class NumberInputType final : public TextFieldInputType
  {
  public:
    explicit NumberInputType(HTMLInputElement &element) noexcept : TextFieldInputType(Type::Number, element)
    {
    }

    KRYS_NODISCARD dom::DOMStringView FormControlType() const noexcept final
    {
      return InputTypeNames::Number;
    }

    KRYS_NODISCARD double ValueAsDouble() const noexcept final;
    dom::ExceptionOr<void> SetValueAsDouble(double value,
                                            TextFieldEventBehavior eventBehavior) const noexcept final;
    dom::ExceptionOr<void> SetValueAsDecimal(const Decimal &value,
                                             TextFieldEventBehavior eventBehavior) const noexcept final;
    KRYS_NODISCARD bool TypeMismatchFor(dom::DOMStringView value) const noexcept final;
    KRYS_NODISCARD bool TypeMismatch() const noexcept final;
    KRYS_NODISCARD StepRange CreateStepRange(AnyStepHandling anyStepHandling) const noexcept final;
    KRYS_NODISCARD Decimal ParseToNumber(dom::DOMStringView string,
                                         const Decimal &defaultValue) const noexcept final;
    KRYS_NODISCARD dom::DOMString Serialize(const Decimal &value) const noexcept final;
    KRYS_NODISCARD dom::DOMString SanitizeValue(dom::DOMStringView proposedValue) const noexcept final;
    KRYS_NODISCARD bool HasBadInput() const noexcept final;
    KRYS_NODISCARD dom::DOMString BadInputText() const noexcept final;
    KRYS_NODISCARD bool SupportsPlaceholder() const noexcept final;
  };
}
