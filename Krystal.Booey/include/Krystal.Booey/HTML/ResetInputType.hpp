#pragma once

#include "Krystal.Booey/HTML/BaseButtonInputType.hpp"
#include "Krystal.Booey/HTML/InputTypeNames.hpp"

namespace krys::boo::html
{
  /// @brief The reset input type.
  class ResetInputType final : public BaseButtonInputType
  {
  public:
    explicit ResetInputType(HTMLInputElement &element) noexcept : BaseButtonInputType(Type::Reset, element)
    {
    }

    KRYS_NODISCARD dom::DOMStringView FormControlType() const noexcept final
    {
      return InputTypeNames::Reset;
    }

    void HandleActivationBehavior(dom::Event &event) noexcept final;
    KRYS_NODISCARD Maybe<dom::DOMString> DefaultValue() const noexcept final;
  };
}
