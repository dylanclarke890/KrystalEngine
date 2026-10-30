#pragma once

#include "Krystal.Booey/HTML/BaseCheckableInputType.hpp"
#include "Krystal.Booey/HTML/InputTypeNames.hpp"

namespace krys::boo::html
{
  /// @brief The checkbox input type.
  class CheckboxInputType final : public BaseCheckableInputType
  {
  public:
    explicit CheckboxInputType(HTMLInputElement &element) noexcept
        : BaseCheckableInputType(Type::Checkbox, element)
    {
    }

    KRYS_NODISCARD dom::DOMStringView FormControlType() const noexcept final
    {
      return InputTypeNames::Checkbox;
    }

    KRYS_NODISCARD bool ValueMissing(dom::DOMStringView value) const noexcept final;
    KRYS_NODISCARD dom::DOMString ValueMissingText() const noexcept final;
    void HandleKeyupEvent(dom::KeyboardEvent &event) noexcept final;
    void WillDispatchClick(InputElementClickState &state) noexcept final;
    void DidDispatchClick(dom::Event &event, const InputElementClickState &state) noexcept final;
    KRYS_NODISCARD bool MatchesIndeterminatePseudoClass() const noexcept final;
  };
}
