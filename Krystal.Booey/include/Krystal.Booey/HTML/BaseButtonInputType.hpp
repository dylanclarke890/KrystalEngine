#pragma once

#include "Krystal.Booey/HTML/BaseClickableWithKeyInputType.hpp"

namespace krys::boo::html
{
  /// @brief The base of the button input types: submit, reset, button and image.
  class BaseButtonInputType : public BaseClickableWithKeyInputType
  {
  public:
    KRYS_NODISCARD bool ShouldSaveAndRestoreFormControlState() const noexcept final
    {
      return false;
    }

    bool AppendFormData(DOMFormData &formData) const noexcept override;
    KRYS_NODISCARD bool StoresValueSeparateFromAttribute() noexcept override;
    void SetValue(dom::DOMStringView sanitizedValue, bool valueChanged, TextFieldEventBehavior eventBehavior,
                  TextControlSetValueSelection selection) noexcept override;
    KRYS_NODISCARD bool DirAutoUsesValue() const noexcept override;
    KRYS_NODISCARD RenderPtr<RenderElement> CreateInputRenderer(RenderStyle &&style) noexcept override;

  protected:
    BaseButtonInputType(Type type, HTMLInputElement &element) noexcept
        : BaseClickableWithKeyInputType(type, element)
    {
    }
  };
}
