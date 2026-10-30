#pragma once

#include "Krystal.Booey/HTML/BaseTextInputType.hpp"
#include "Krystal.Booey/HTML/InputTypeNames.hpp"

namespace krys::boo::html
{
  /// @brief The text input type, which is also the type of inputs with a missing or unknown type.
  class TextInputType final : public BaseTextInputType
  {
  public:
    explicit TextInputType(HTMLInputElement &element) noexcept : BaseTextInputType(Type::Text, element)
    {
    }

    KRYS_NODISCARD dom::DOMStringView FormControlType() const noexcept final
    {
      return InputTypeNames::Text;
    }
  };
}
