#include "Krystal.Booey/HTML/RadioInputType.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Event.hpp"
#include "Krystal.Booey/DOM/KeyboardEvent.hpp"
#include "Krystal.Booey/DOM/MouseEvent.hpp"
#include "Krystal.Booey/DOM/RadioButtonGroups.hpp"
#include "Krystal.Booey/HTML/Algorithms/FocusAlgorithms.hpp"
#include "Krystal.Booey/HTML/HTMLFormElement.hpp"
#include "Krystal.Booey/HTML/HTMLInputElement.hpp"
#include "Krystal.Booey/Page/Settings.hpp"
#include "Krystal.Booey/Platform/LocalizedStrings.hpp"

namespace krys::boo::html
{
  bool RadioInputType::ValueMissing(dom::DOMStringView) const noexcept
  {
    auto *element = Element();
    krys_debug_assert(element);
    auto name = element->Name();

    if (auto *buttons = element->RadioButtonGroups())
    {
      return !buttons->CheckedButtonForGroup(name) && buttons->IsInRequiredGroup(*element);
    }

    if (name.empty())
    {
      return false;
    }

    krys_debug_assert(!element->IsConnected());
    krys_debug_assert(!element->Form());

    bool isRequired = false;
    bool foundCheckedRadio = false;
    ForEachButtonInDetachedGroup(Downcast<dom::ContainerNode>(dom::TreeQueries::Root(*element)), name,
                                 [&](HTMLInputElement &input)
                                 {
                                   if (input.Checked())
                                   {
                                     foundCheckedRadio = true;
                                     return false;
                                   }
                                   if (input.IsRequired())
                                   {
                                     isRequired = true;
                                   }
                                   return true;
                                 });
    return isRequired && !foundCheckedRadio;
  }

  void RadioInputType::ForEachButtonInDetachedGroup(dom::ContainerNode &rootNode,
                                                    dom::DOMStringView groupName,
                                                    const Func<bool(HTMLInputElement &)> &apply) noexcept
  {
    krys_debug_assert(!groupName.empty());

    for (dom::Node *node = &rootNode; node;)
    {
      if (Is<HTMLFormElement>(*node))
      {
        // No need to consider the descendants of a form since they will have a form owner and we're only
        // interested in input elements without a form owner.
        node = dom::TreeQueries::NextSkippingChildren(*node, &rootNode);
        continue;
      }
      auto *input = DynamicDowncast<HTMLInputElement>(*node);
      if (input && input->IsRadioButton() && !input->Form() && input->Name() == groupName)
      {
        if (!apply(*input))
        {
          return;
        }
      }
      node = dom::TreeQueries::Next(*node, &rootNode);
    }
  }

  void RadioInputType::WillUpdateCheckedness(bool nowChecked, WasSetByJavaScript) noexcept
  {
    if (!nowChecked)
    {
      return;
    }
    auto &element = *Element();
    if (element.RadioButtonGroups())
    {
      // Buttons in RadioButtonGroups are handled in HTMLInputElement::SetChecked().
      return;
    }
    if (auto input = element.CheckedRadioButtonForGroup())
    {
      input->SetChecked(false);
    }
  }

  dom::DOMString RadioInputType::ValueMissingText() const noexcept
  {
    return ValidationMessageValueMissingForRadioText();
  }

  void RadioInputType::HandleClickEvent(dom::MouseEvent &event) noexcept
  {
    event.SetDefaultHandled();
  }

  InputType::ShouldCallBaseEventHandler RadioInputType::HandleKeydownEvent(dom::KeyboardEvent &event) noexcept
  {
    if (BaseCheckableInputType::HandleKeydownEvent(event) == ShouldCallBaseEventHandler::No)
    {
      return ShouldCallBaseEventHandler::No;
    }

    if (event.DefaultHandled())
    {
      return ShouldCallBaseEventHandler::Yes;
    }

    const auto &key = event.KeyIdentifier();
    if (key != u8"Up" && key != u8"Down" && key != u8"Left" && key != u8"Right")
    {
      return ShouldCallBaseEventHandler::Yes;
    }

    auto *element = Element();
    krys_debug_assert(element);

    // Left and up mean "previous radio button". Right and down mean "next radio button". Tested in WinIE, and
    // even for RTL, left still means previous radio button (and so moves to the right). Seems strange, but
    // we'll match it. However, when using Spatial Navigation, we need to be able to navigate without changing
    // the selection.
    if (element->NodeDocument().GetSettings().SpatialNavigationEnabled)
    {
      return ShouldCallBaseEventHandler::Yes;
    }
    bool forward = key == u8"Down" || key == u8"Right";

    // We can only stay within the form's children if the form hasn't been demoted to a leaf because of
    // malformed HTML.
    for (dom::Node *node = forward ? dom::TreeQueries::Next(*element) : dom::TreeQueries::Previous(*element);
         node; node = forward ? dom::TreeQueries::Next(*node) : dom::TreeQueries::Previous(*node))
    {
      // Once we encounter a form element, we know we're through.
      if (Is<HTMLFormElement>(*node))
      {
        break;
      }

      // Look for more radio buttons.
      auto *inputElement = DynamicDowncast<HTMLInputElement>(*node);
      if (!inputElement)
      {
        continue;
      }

      if (inputElement->Form() != element->Form())
      {
        break;
      }

      if (inputElement->IsRadioButton() && inputElement->Name() == element->Name()
          && FocusAlgorithms::IsFocusableArea(*inputElement))
      {
        Ref<HTMLInputElement> protectedInput = ShareRef(*inputElement);
        inputElement->Focus();
        // FIXME: Scrolling the button into view if it isn't visible (scrollIntoViewIfNotVisible).
        (void)inputElement->DispatchSimulatedClick(&event, dom::SimulatedClickMouseEventOptions::SendNoEvents,
                                                   dom::SimulatedClickVisualOptions::DoNotShowPressedLook);
        event.SetDefaultHandled();
        return ShouldCallBaseEventHandler::Yes;
      }
    }
    return ShouldCallBaseEventHandler::Yes;
  }

  void RadioInputType::HandleKeyupEvent(dom::KeyboardEvent &event) noexcept
  {
    if (event.KeyIdentifier() != u8"U+0020")
    {
      return;
    }

    auto *element = Element();
    krys_debug_assert(element);

    // If an unselected radio is tabbed into (because the entire group has nothing checked, or because of some
    // explicit .focus() call), then allow space to check it.
    if (element->Checked())
    {
      // If we are going to skip DispatchSimulatedClick, then at least call SetActive(false) to prevent the
      // radio from being stuck in the active state.
      element->SetActive(false);
      return;
    }
    DispatchSimulatedClickIfActive(event);
  }

  bool RadioInputType::ShouldSendChangeEventAfterCheckedChanged() noexcept
  {
    // Don't send a change event for a radio button that's getting unchecked. This was done to match the
    // behavior of other browsers.
    krys_debug_assert(Element());
    return Element()->Checked();
  }

  void RadioInputType::WillDispatchClick(InputElementClickState &state) noexcept
  {
    // An event handler can use preventDefault or "return false" to reverse the selection we do here. The
    // InputElementClickState object contains what we need to undo what we did here in DidDispatchClick.

    // We want radio groups to end up in sane states, i.e., to have something checked. Therefore if nothing is
    // currently selected, we won't allow the upcoming action to be "undone", since we want some object in the
    // radio group to actually get selected.

    auto *element = Element();
    krys_debug_assert(element);

    state.Checked = element->Checked();
    state.CheckedRadioButton = element->CheckedRadioButtonForGroup();
    element->SetChecked(true);
  }

  void RadioInputType::DidDispatchClick(dom::Event &event, const InputElementClickState &state) noexcept
  {
    auto *element = Element();
    if (event.DefaultPrevented() || event.DefaultHandled())
    {
      // Restore the original selected radio button if possible.
      // Make sure it is still a radio button and only do the restoration if it still belongs to our group.
      auto &button = state.CheckedRadioButton;
      krys_debug_assert(element);
      if (button && button->IsRadioButton() && button->Form() == element->Form()
          && button->Name() == element->Name())
      {
        button->SetChecked(true);
      }
      else
      {
        element->SetChecked(false);
      }
    }
    else if (state.Checked != element->Checked())
    {
      FireInputAndChangeEvents();
    }

    // The work we did in WillDispatchClick was default handling.
    event.SetDefaultHandled();
  }

  bool RadioInputType::MatchesIndeterminatePseudoClass() const noexcept
  {
    krys_debug_assert(Element());
    auto &element = *Element();
    if (auto *radioButtonGroups = element.RadioButtonGroups())
    {
      return !radioButtonGroups->HasCheckedButton(element);
    }
    return !element.Checked();
  }
}
