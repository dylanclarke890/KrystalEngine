#pragma once

#include "Krystal.Booey/Graphics/IntPoint.hpp"
#include "Krystal.Booey/HTML/BaseButtonInputType.hpp"
#include "Krystal.Booey/HTML/InputTypeNames.hpp"

namespace krys::boo::html
{
  /// @brief The image input type: a submit button that submits where it was clicked.
  // FIXME: Loading and drawing the image (its image loader and RenderImage), its alt text, and its width and
  // height come with the other controls.
  class ImageInputType final : public BaseButtonInputType
  {
  private:
    IntPoint _clickLocation;

  public:
    explicit ImageInputType(HTMLInputElement &element) noexcept : BaseButtonInputType(Type::Image, element)
    {
    }

    KRYS_NODISCARD dom::DOMStringView FormControlType() const noexcept final
    {
      return InputTypeNames::Image;
    }

    KRYS_NODISCARD bool IsFormDataAppendable() const noexcept final;
    bool AppendFormData(DOMFormData &formData) const noexcept final;
    /// @brief Where the button was clicked, as `"x,y"`.
    KRYS_NODISCARD Maybe<dom::DOMString> ResultForDialogSubmit() const noexcept final;
    KRYS_NODISCARD bool SupportsValidationForBindings() const noexcept
    {
      return false;
    }
    void HandleActivationBehavior(dom::Event &event) noexcept final;
    KRYS_NODISCARD bool ShouldRespectHeightAndWidthAttributes() noexcept final;
    KRYS_NODISCARD bool CanBeSuccessfulSubmitButton() noexcept final;
    KRYS_NODISCARD RenderPtr<RenderElement> CreateInputRenderer(RenderStyle &&style) noexcept final;
  };
}
