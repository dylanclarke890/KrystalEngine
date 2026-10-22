#pragma once

#include "Krystal.Booey/HTML/BaseClickableWithKeyInputType.hpp"

namespace krys::boo::html
{
  /// @brief The base of the button input types: submit, reset, button and image.
  // FIXME: The button renderer (RenderButton) comes with the other controls.
  class BaseButtonInputType : public BaseClickableWithKeyInputType
  {
  public:
    bool AppendFormData(DOMFormData &formData) const noexcept override;
    KRYS_NODISCARD bool StoresValueSeparateFromAttribute() noexcept override;
    void SetValue(dom::DOMStringView sanitizedValue, bool valueChanged, TextFieldEventBehavior eventBehavior,
                  TextControlSetValueSelection selection) noexcept override;
    KRYS_NODISCARD bool DirAutoUsesValue() const noexcept override;

  protected:
    BaseButtonInputType(Type type, HTMLInputElement &element) noexcept
        : BaseClickableWithKeyInputType(type, element)
    {
    }
  };
}
