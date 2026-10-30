#include "Krystal.Booey/HTML/BaseCheckableInputType.hpp"
#include "Krystal.Booey/Constants/EventNames.hpp"
#include "Krystal.Booey/DOM/Algorithms/EventDispatcher.hpp"
#include "Krystal.Booey/DOM/KeyboardEvent.hpp"
#include "Krystal.Booey/HTML/DOMFormData.hpp"
#include "Krystal.Booey/HTML/HTMLInputElement.hpp"

namespace krys::boo::html
{
  FormControlState BaseCheckableInputType::SaveFormControlState() const noexcept
  {
    krys_debug_assert(Element());
    return {dom::DOMString(Element()->Checked() ? u8"on" : u8"off")};
  }

  void BaseCheckableInputType::RestoreFormControlState(const FormControlState &state) noexcept
  {
    krys_debug_assert(Element());
    Element()->SetChecked(state[0] == u8"on");
  }

  bool BaseCheckableInputType::AppendFormData(DOMFormData &formData) const noexcept
  {
    krys_debug_assert(Element());
    auto &element = *Element();
    if (!element.Checked())
    {
      return false;
    }
    formData.Append(element.Name(), element.Value());
    return true;
  }

  InputType::ShouldCallBaseEventHandler
    BaseCheckableInputType::HandleKeydownEvent(dom::KeyboardEvent &event) noexcept
  {
    if (event.KeyIdentifier() == u8"U+0020")
    {
      krys_debug_assert(Element());
      Element()->SetActive(true);
      // No SetDefaultHandled(): a keypress follows only if the keydown isn't handled.
      return ShouldCallBaseEventHandler::No;
    }
    return ShouldCallBaseEventHandler::Yes;
  }

  void BaseCheckableInputType::HandleKeypressEvent(dom::KeyboardEvent &event) noexcept
  {
    if (event.CharCode() == ' ')
    {
      // Prevent scrolling down the page.
      event.SetDefaultHandled();
    }
  }

  bool BaseCheckableInputType::CanSetStringValue() const noexcept
  {
    return false;
  }

  Maybe<dom::DOMString> BaseCheckableInputType::FallbackValue() const noexcept
  {
    return dom::DOMString(u8"on");
  }

  bool BaseCheckableInputType::StoresValueSeparateFromAttribute() noexcept
  {
    return false;
  }

  void BaseCheckableInputType::SetValue(dom::DOMStringView sanitizedValue, bool, TextFieldEventBehavior,
                                        TextControlSetValueSelection) noexcept
  {
    krys_debug_assert(Element());
    (void)Element()->SetAttribute(u8"value", dom::DOMString(sanitizedValue));
  }

  void BaseCheckableInputType::FireInputAndChangeEvents() noexcept
  {
    Ref<HTMLInputElement> element = ShareRef(*Element());
    if (!element->IsConnected())
    {
      return;
    }

    if (!ShouldSendChangeEventAfterCheckedChanged())
    {
      return;
    }

    Ref<InputType> protectedThis = ShareRef(*this);
    element->SetTextAsOfLastFormControlChangeEvent({});
    auto event = dom::Event::Create(dom::DOMStringAtom(EventNames::Input), true, false, true);
    (void)dom::EventDispatcher::DispatchToTarget(*event, element.get());
    if (auto *currentElement = Element())
    {
      currentElement->DispatchFormControlChangeEvent();
    }
  }
}
