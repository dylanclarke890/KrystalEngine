#include "Krystal.Web/Html/InputTypes/RadioInputType.hpp"
#include "Krystal.Web/Dom/Algorithms/TreeQueries.hpp"
#include "Krystal.Web/Dom/Events/Event.hpp"
#include "Krystal.Web/Events/Pointer/MouseEvent.hpp"
#include "Krystal.Web/Events/UI/KeyboardEvent.hpp"
#include "Krystal.Web/Html/Algorithms/FocusAlgorithms.hpp"
#include "Krystal.Web/Html/Elements/HTMLFormElement.hpp"
#include "Krystal.Web/Html/Elements/HTMLInputElement.hpp"
#include "Krystal.Web/Html/Forms/RadioButtonGroups.hpp"
#include "Krystal.Web/Page/Settings.hpp"
#include "Krystal.Web/Platform/LocalizedStrings.hpp"

namespace krys::web::html
{
  bool RadioInputType::ValueMissing(DOMStringView) const noexcept
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
    ForEachButtonInDetachedGroup(Downcast<ContainerNode>(dom::TreeQueries::Root(*element)), name,
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

  void RadioInputType::ForEachButtonInDetachedGroup(ContainerNode &rootNode, DOMStringView groupName,
                                                    const Func<bool(HTMLInputElement &)> &apply) noexcept
  {
    krys_debug_assert(!groupName.empty());

    for (Node *node = &rootNode; node;)
    {
      if (Is<HTMLFormElement>(*node))
      {
        // A form's descendants have it as their form owner, and only buttons without one are wanted.
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
      // A button in a group set is unchecked by the group (HTMLInputElement::SetChecked).
      return;
    }
    if (auto input = element.CheckedRadioButtonForGroup())
    {
      input->SetChecked(false);
    }
  }

  DOMString RadioInputType::ValueMissingText() const noexcept
  {
    return ValidationMessageValueMissingForRadioText();
  }

  void RadioInputType::HandleClickEvent(MouseEvent &event) noexcept
  {
    event.SetDefaultHandled();
  }

  InputType::ShouldCallBaseEventHandler RadioInputType::HandleKeydownEvent(KeyboardEvent &event) noexcept
  {
    if (BaseCheckableInputType::HandleKeydownEvent(event) == ShouldCallBaseEventHandler::No)
    {
      return ShouldCallBaseEventHandler::No;
    }

    if (event.DefaultHandled())
    {
      return ShouldCallBaseEventHandler::Yes;
    }

    const auto &key = event.Key();
    if (key != u8"ArrowUp" && key != u8"ArrowDown" && key != u8"ArrowLeft" && key != u8"ArrowRight")
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
    bool forward = key == u8"ArrowDown" || key == u8"ArrowRight";

    // The walk goes through the document, stopping at a form: the buttons of a form may not all be its
    // descendants.
    for (Node *node = forward ? dom::TreeQueries::Next(*element) : dom::TreeQueries::Previous(*element); node;
         node = forward ? dom::TreeQueries::Next(*node) : dom::TreeQueries::Previous(*node))
    {
      if (Is<HTMLFormElement>(*node))
      {
        break;
      }

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
        // If the focused radio button is not visible (e.g., during arrow key navigation), scroll it into
        // view.
        inputElement->ScrollIntoViewIfNotVisible(false);
        (void)inputElement->DispatchSimulatedClick(&event,
                                                   html::SimulatedClickMouseEventOptions::SendNoEvents,
                                                   html::SimulatedClickVisualOptions::DoNotShowPressedLook);
        event.SetDefaultHandled();
        return ShouldCallBaseEventHandler::Yes;
      }
    }
    return ShouldCallBaseEventHandler::Yes;
  }

  void RadioInputType::HandleKeyupEvent(KeyboardEvent &event) noexcept
  {
    if (event.Key() != u8" ")
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

  void RadioInputType::DidDispatchClick(Event &event, const InputElementClickState &state) noexcept
  {
    auto *element = Element();
    if (event.DefaultPrevented() || event.DefaultHandled())
    {
      // A canceled click checks the group's previous button again, if it's still a radio button in the group.
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

    // Checking the button before the click was its default handling.
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
