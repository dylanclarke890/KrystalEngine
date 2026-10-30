#pragma once

#include "Krystal.Booey/HTML/InputType.hpp"

namespace krys::boo::html
{
  /// @brief The base of the checkbox and radio input types.
  class BaseCheckableInputType : public InputType
  {
  public:
    KRYS_NODISCARD FormControlState SaveFormControlState() const noexcept final;
    void RestoreFormControlState(const FormControlState &state) noexcept final;
    bool AppendFormData(DOMFormData &formData) const noexcept override;
    ShouldCallBaseEventHandler HandleKeydownEvent(dom::KeyboardEvent &event) noexcept override;
    void HandleKeypressEvent(dom::KeyboardEvent &event) noexcept override;
    KRYS_NODISCARD bool CanSetStringValue() const noexcept override;
    KRYS_NODISCARD Maybe<dom::DOMString> FallbackValue() const noexcept override;
    KRYS_NODISCARD bool StoresValueSeparateFromAttribute() noexcept override;
    void SetValue(dom::DOMStringView sanitizedValue, bool valueChanged, TextFieldEventBehavior eventBehavior,
                  TextControlSetValueSelection selection) noexcept override;

  protected:
    BaseCheckableInputType(Type type, HTMLInputElement &element) noexcept : InputType(type, element)
    {
    }

    void FireInputAndChangeEvents() noexcept;
  };
}
