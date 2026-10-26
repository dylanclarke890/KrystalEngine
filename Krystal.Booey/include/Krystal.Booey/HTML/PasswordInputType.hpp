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

    /// @brief Password fields are never saved or restored.
    KRYS_NODISCARD bool ShouldSaveAndRestoreFormControlState() const noexcept final
    {
      return false;
    }

    /// @brief Input methods are disabled for the password field because otherwise anyone can access the
    /// underlying password and display it in clear text.
    KRYS_NODISCARD bool ShouldUseInputMethod() const noexcept final
    {
      return false;
    }
  };
}
