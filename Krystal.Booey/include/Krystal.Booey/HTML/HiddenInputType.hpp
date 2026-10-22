#pragma once

#include "Krystal.Booey/HTML/InputType.hpp"
#include "Krystal.Booey/HTML/InputTypeNames.hpp"

namespace krys::boo::html
{
  /// @brief The hidden input type.
  class HiddenInputType final : public InputType
  {
  public:
    explicit HiddenInputType(HTMLInputElement &element) noexcept : InputType(Type::Hidden, element)
    {
    }

    KRYS_NODISCARD dom::DOMStringView FormControlType() const noexcept final
    {
      return InputTypeNames::Hidden;
    }

    KRYS_NODISCARD bool RendererIsNeeded() noexcept final;
    KRYS_NODISCARD bool StoresValueSeparateFromAttribute() noexcept final;
    void SetValue(dom::DOMStringView sanitizedValue, bool valueChanged, TextFieldEventBehavior eventBehavior,
                  TextControlSetValueSelection selection) noexcept final;
    bool AppendFormData(DOMFormData &formData) const noexcept final;
    KRYS_NODISCARD bool ShouldRespectHeightAndWidthAttributes() noexcept final;
    KRYS_NODISCARD bool DirAutoUsesValue() const noexcept final;
  };
}
