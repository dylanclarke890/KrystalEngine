#pragma once

#include "Krystal.Booey/HTML/BaseTextInputType.hpp"
#include "Krystal.Booey/HTML/InputTypeNames.hpp"

namespace krys::boo::html
{
  /// @brief The password input type.
  // FIXME: Masked text and the caps lock indicator come with text fields.
  class PasswordInputType final : public BaseTextInputType
  {
  public:
    explicit PasswordInputType(HTMLInputElement &element) noexcept
        : BaseTextInputType(Type::Password, element)
    {
    }

    KRYS_NODISCARD dom::DOMStringView FormControlType() const noexcept final
    {
      return InputTypeNames::Password;
    }
  };
}
