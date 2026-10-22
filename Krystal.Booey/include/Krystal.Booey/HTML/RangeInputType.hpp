#pragma once

#include "Krystal.Booey/HTML/InputType.hpp"
#include "Krystal.Booey/HTML/InputTypeNames.hpp"

namespace krys::boo::html
{
  /// @brief The range input type.
  // FIXME: The slider (its shadow tree, renderer and thumb dragging) and its tick marks come with the other
  // controls.
  class RangeInputType final : public InputType
  {
  public:
    explicit RangeInputType(HTMLInputElement &element) noexcept : InputType(Type::Range, element)
    {
    }

    KRYS_NODISCARD dom::DOMStringView FormControlType() const noexcept final
    {
      return InputTypeNames::Range;
    }

    KRYS_NODISCARD double ValueAsDouble() const noexcept final;
    dom::ExceptionOr<void> SetValueAsDecimal(const Decimal &value,
                                             TextFieldEventBehavior eventBehavior) const noexcept final;
    KRYS_NODISCARD bool TypeMismatchFor(dom::DOMStringView value) const noexcept final;
    KRYS_NODISCARD bool SupportsRequired() const noexcept final;
    KRYS_NODISCARD StepRange CreateStepRange(AnyStepHandling anyStepHandling) const noexcept final;
    ShouldCallBaseEventHandler HandleKeydownEvent(dom::KeyboardEvent &event) noexcept final;
    KRYS_NODISCARD Decimal ParseToNumber(dom::DOMStringView string,
                                         const Decimal &defaultValue) const noexcept final;
    KRYS_NODISCARD dom::DOMString Serialize(const Decimal &value) const noexcept final;
    void AttributeChanged(dom::DOMStringAtom localName) noexcept final;
    void SetValue(dom::DOMStringView value, bool valueChanged, TextFieldEventBehavior eventBehavior,
                  TextControlSetValueSelection selection) noexcept final;
    KRYS_NODISCARD Maybe<dom::DOMString> FallbackValue() const noexcept final;
    KRYS_NODISCARD dom::DOMString SanitizeValue(dom::DOMStringView proposedValue) const noexcept final;
  };
}
