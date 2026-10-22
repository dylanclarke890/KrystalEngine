#pragma once

#include "Krystal.Booey/HTML/BaseButtonInputType.hpp"
#include "Krystal.Booey/HTML/InputTypeNames.hpp"

namespace krys::boo::html
{
  /// @brief The submit input type.
  class SubmitInputType final : public BaseButtonInputType
  {
  public:
    explicit SubmitInputType(HTMLInputElement &element) noexcept : BaseButtonInputType(Type::Submit, element)
    {
    }

    KRYS_NODISCARD dom::DOMStringView FormControlType() const noexcept final
    {
      return InputTypeNames::Submit;
    }

    bool AppendFormData(DOMFormData &formData) const noexcept final;
    KRYS_NODISCARD bool SupportsRequired() const noexcept final;
    void HandleActivationBehavior(dom::Event &event) noexcept final;
    KRYS_NODISCARD bool CanBeSuccessfulSubmitButton() noexcept final;
    KRYS_NODISCARD Maybe<dom::DOMString> DefaultValue() const noexcept final;
  };
}
