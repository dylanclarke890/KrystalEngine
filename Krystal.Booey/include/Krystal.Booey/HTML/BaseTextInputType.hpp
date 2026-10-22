#pragma once

#include "Krystal.Booey/HTML/TextFieldInputType.hpp"

namespace krys::boo::html
{
  /// @brief The base of the input types with a single line of text and a pattern: text, search, tel, url,
  /// email and password.
  class BaseTextInputType : public TextFieldInputType
  {
  public:
    KRYS_NODISCARD bool PatternMismatch(dom::DOMStringView value) const noexcept override;
    KRYS_NODISCARD bool SupportsPlaceholder() const noexcept override;
    KRYS_NODISCARD bool SupportsSelectionAPI() const noexcept override;
    KRYS_NODISCARD bool DirAutoUsesValue() const noexcept override;

  protected:
    BaseTextInputType(Type type, HTMLInputElement &element) noexcept : TextFieldInputType(type, element)
    {
    }
  };
}
