#pragma once

#include "Krystal.Booey/HTML/BaseTextInputType.hpp"
#include "Krystal.Booey/HTML/InputTypeNames.hpp"

namespace krys::boo::html
{
  /// @brief The email input type.
  class EmailInputType final : public BaseTextInputType
  {
  public:
    explicit EmailInputType(HTMLInputElement &element) noexcept : BaseTextInputType(Type::Email, element)
    {
    }

    KRYS_NODISCARD dom::DOMStringView FormControlType() const noexcept final
    {
      return InputTypeNames::Email;
    }

    KRYS_NODISCARD bool TypeMismatchFor(dom::DOMStringView value) const noexcept final;
    KRYS_NODISCARD bool TypeMismatch() const noexcept final;
    KRYS_NODISCARD dom::DOMString TypeMismatchText() const noexcept final;
    KRYS_NODISCARD bool SupportsSelectionAPI() const noexcept final;
    void AttributeChanged(dom::DOMStringAtom localName) noexcept final;
    KRYS_NODISCARD dom::DOMString SanitizeValue(dom::DOMStringView proposedValue) const noexcept final;
  };
}
