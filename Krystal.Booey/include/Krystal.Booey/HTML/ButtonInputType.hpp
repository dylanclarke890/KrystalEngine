#pragma once

#include "Krystal.Booey/HTML/BaseButtonInputType.hpp"
#include "Krystal.Booey/HTML/InputTypeNames.hpp"

namespace krys::boo::html
{
  /// @brief The button input type.
  class ButtonInputType final : public BaseButtonInputType
  {
  public:
    explicit ButtonInputType(HTMLInputElement &element) noexcept : BaseButtonInputType(Type::Button, element)
    {
    }

    KRYS_NODISCARD dom::DOMStringView FormControlType() const noexcept final
    {
      return InputTypeNames::Button;
    }
  };
}
