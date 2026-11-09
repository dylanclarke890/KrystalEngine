#pragma once

#include "Krystal.Booey/Graphics/Geometry/IntPoint.hpp"
#include "Krystal.Booey/HTML/InputTypes/BaseButtonInputType.hpp"
#include "Krystal.Booey/HTML/InputTypes/InputTypeNames.hpp"

namespace krys::boo::html
{
  /// @brief The image input type: a submit button showing an image, that submits where it was clicked.
  /// @see https://html.spec.whatwg.org/#image-button-state-(type=image)
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
    KRYS_NODISCARD bool DirAutoUsesValue() const noexcept final;
    KRYS_NODISCARD bool SupportsValidationForBindings() const noexcept
    {
      return false;
    }
    void HandleActivationBehavior(dom::Event &event) noexcept final;
    KRYS_NODISCARD bool ShouldRespectHeightAndWidthAttributes() noexcept final;
    KRYS_NODISCARD bool CanBeSuccessfulSubmitButton() noexcept final;
    KRYS_NODISCARD uint32 Height() const noexcept final;
    KRYS_NODISCARD uint32 Width() const noexcept final;
    KRYS_NODISCARD RenderPtr<RenderElement> CreateInputRenderer(RenderStyle &&style) noexcept final;
    void DidBecomeElementType() noexcept final;
    void Attach() noexcept final;
    void AttributeChanged(dom::DOMStringAtom localName) noexcept final;
  };
}
