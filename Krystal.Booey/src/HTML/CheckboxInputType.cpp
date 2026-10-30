#include "Krystal.Booey/HTML/CheckboxInputType.hpp"
#include "Krystal.Booey/DOM/Event.hpp"
#include "Krystal.Booey/DOM/KeyboardEvent.hpp"
#include "Krystal.Booey/HTML/HTMLInputElement.hpp"
#include "Krystal.Booey/Platform/LocalizedStrings.hpp"

namespace krys::boo::html
{
  bool CheckboxInputType::ValueMissing(dom::DOMStringView) const noexcept
  {
    krys_debug_assert(Element());
    return Element()->IsRequired() && !Element()->Checked();
  }

  dom::DOMString CheckboxInputType::ValueMissingText() const noexcept
  {
    return ValidationMessageValueMissingForCheckboxText();
  }

  void CheckboxInputType::HandleKeyupEvent(dom::KeyboardEvent &event) noexcept
  {
    if (event.KeyIdentifier() != u8"U+0020")
    {
      return;
    }
    DispatchSimulatedClickIfActive(event);
  }

  void CheckboxInputType::WillDispatchClick(InputElementClickState &state) noexcept
  {
    // An event handler can use preventDefault or "return false" to reverse the checking we do here. The
    // InputElementClickState object contains what we need to undo what we did here in DidDispatchClick.

    krys_debug_assert(Element());
    auto &element = *Element();

    state.Checked = element.Checked();
    state.Indeterminate = element.Indeterminate();

    if (state.Indeterminate)
    {
      element.SetIndeterminate(false);
    }

    element.SetChecked(!state.Checked, state.Trusted ? WasSetByJavaScript::No : WasSetByJavaScript::Yes);
  }

  void CheckboxInputType::DidDispatchClick(dom::Event &event, const InputElementClickState &state) noexcept
  {
    if (event.DefaultPrevented() || event.DefaultHandled())
    {
      krys_debug_assert(Element());
      auto &element = *Element();
      element.SetIndeterminate(state.Indeterminate);
      element.SetChecked(state.Checked);
    }
    else
    {
      FireInputAndChangeEvents();
    }

    // The work we did in WillDispatchClick was default handling.
    event.SetDefaultHandled();
  }

  bool CheckboxInputType::MatchesIndeterminatePseudoClass() const noexcept
  {
    krys_debug_assert(Element());
    return Element()->Indeterminate();
  }
}
