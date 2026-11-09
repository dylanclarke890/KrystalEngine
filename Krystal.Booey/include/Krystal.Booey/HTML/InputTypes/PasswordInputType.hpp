#pragma once

#include "Krystal.Booey/HTML/InputTypes/BaseTextInputType.hpp"
#include "Krystal.Booey/HTML/InputTypes/InputTypeNames.hpp"

namespace krys::boo::html
{
  /// @brief The password input type. There's no caps lock indicator, as in Chrome.
  class PasswordInputType final : public BaseTextInputType
  {
  public:
    explicit PasswordInputType(HTMLInputElement &element) noexcept
        : BaseTextInputType(Type::Password, element)
    {
    }

    KRYS_NODISCARD bool ShouldRespectListAttribute() const noexcept final
    {
      return false;
    }

    KRYS_NODISCARD dom::DOMStringView FormControlType() const noexcept final
    {
      return InputTypeNames::Password;
    }

    /// @brief Input methods are disabled for the password field because otherwise anyone can access the
    /// underlying password and display it in clear text.
    KRYS_NODISCARD bool ShouldUseInputMethod() const noexcept final
    {
      return false;
    }
  };
}
