#pragma once

#include "Krystal.Booey/HTML/BaseTextInputType.hpp"
#include "Krystal.Booey/HTML/InputTypeNames.hpp"

namespace krys::boo::html
{
  /// @brief The tel input type.
  class TelephoneInputType final : public BaseTextInputType
  {
  public:
    explicit TelephoneInputType(HTMLInputElement &element) noexcept
        : BaseTextInputType(Type::Telephone, element)
    {
    }

    KRYS_NODISCARD dom::DOMStringView FormControlType() const noexcept final
    {
      return InputTypeNames::Telephone;
    }
  };
}
