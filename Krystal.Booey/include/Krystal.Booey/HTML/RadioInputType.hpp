#pragma once

#include "Krystal.Booey/HTML/BaseCheckableInputType.hpp"
#include "Krystal.Booey/HTML/InputTypeNames.hpp"
#include "Krystal.Core/Types/Func.hpp"

namespace krys::boo::html
{
  /// @brief The radio input type.
  class RadioInputType final : public BaseCheckableInputType
  {
  public:
    explicit RadioInputType(HTMLInputElement &element) noexcept : BaseCheckableInputType(Type::Radio, element)
    {
    }

    /// @brief Calls @p apply for the radio buttons named @p groupName without a form owner in @p rootNode's
    /// tree, until it returns false.
    static void ForEachButtonInDetachedGroup(dom::ContainerNode &rootNode, dom::DOMStringView groupName,
                                             const Func<bool(HTMLInputElement &)> &apply) noexcept;

    KRYS_NODISCARD dom::DOMStringView FormControlType() const noexcept final
    {
      return InputTypeNames::Radio;
    }

    KRYS_NODISCARD bool ValueMissing(dom::DOMStringView value) const noexcept final;
    KRYS_NODISCARD dom::DOMString ValueMissingText() const noexcept final;
    void HandleClickEvent(dom::MouseEvent &event) noexcept final;
    ShouldCallBaseEventHandler HandleKeydownEvent(dom::KeyboardEvent &event) noexcept final;
    void HandleKeyupEvent(dom::KeyboardEvent &event) noexcept final;
    KRYS_NODISCARD bool ShouldSendChangeEventAfterCheckedChanged() noexcept final;
    void WillDispatchClick(InputElementClickState &state) noexcept final;
    void DidDispatchClick(dom::Event &event, const InputElementClickState &state) noexcept final;
    KRYS_NODISCARD bool MatchesIndeterminatePseudoClass() const noexcept final;
    void WillUpdateCheckedness(bool nowChecked, WasSetByJavaScript wasCheckedByJavaScript) noexcept final;
  };
}
