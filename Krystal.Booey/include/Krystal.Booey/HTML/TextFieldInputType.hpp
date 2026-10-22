#pragma once

#include "Krystal.Booey/HTML/InputType.hpp"

namespace krys::boo::html
{
  /// @brief The base of the text-like input types and number.
  // FIXME: The shadow tree (the inner text, the placeholder, the spin button, the autofill button and the
  // datalist dropdown), the renderer, typing and the before-text-inserted handling come with text fields.
  class TextFieldInputType : public InputType
  {
  public:
    KRYS_NODISCARD bool ValueMissing(dom::DOMStringView value) const noexcept override;
    void SetValue(dom::DOMStringView sanitizedValue, bool valueChanged, TextFieldEventBehavior eventBehavior,
                  TextControlSetValueSelection selection) noexcept override;
    KRYS_NODISCARD bool ShouldSubmitImplicitly(dom::Event &event) noexcept override;
    KRYS_NODISCARD bool SupportsReadOnly() const noexcept override;
    KRYS_NODISCARD bool IsEmptyValue() const noexcept final;
    KRYS_NODISCARD dom::DOMString SanitizeValue(dom::DOMStringView proposedValue) const noexcept override;
    bool AppendFormData(DOMFormData &formData) const noexcept override;

  protected:
    TextFieldInputType(Type type, HTMLInputElement &element) noexcept : InputType(type, element)
    {
    }
  };
}
