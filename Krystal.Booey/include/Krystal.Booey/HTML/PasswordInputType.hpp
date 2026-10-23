#pragma once

#include "Krystal.Booey/HTML/BaseTextInputType.hpp"
#include "Krystal.Booey/HTML/InputTypeNames.hpp"

namespace krys::boo::html
{
  /// @brief The password input type.
  // FIXME: The caps lock indicator.
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
