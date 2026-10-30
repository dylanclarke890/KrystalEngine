#pragma once

#include "Krystal.Booey/HTML/BaseTextInputType.hpp"
#include "Krystal.Booey/HTML/InputTypeNames.hpp"

namespace krys::boo::html
{
  /// @brief The url input type.
  class URLInputType final : public BaseTextInputType
  {
  public:
    explicit URLInputType(HTMLInputElement &element) noexcept : BaseTextInputType(Type::URL, element)
    {
    }

    KRYS_NODISCARD dom::DOMStringView FormControlType() const noexcept final
    {
      return InputTypeNames::URL;
    }

    KRYS_NODISCARD bool TypeMismatchFor(dom::DOMStringView value) const noexcept final;
    KRYS_NODISCARD bool TypeMismatch() const noexcept final;
    KRYS_NODISCARD dom::DOMString TypeMismatchText() const noexcept final;
    KRYS_NODISCARD dom::DOMString SanitizeValue(dom::DOMStringView proposedValue) const noexcept final;
  };
}
