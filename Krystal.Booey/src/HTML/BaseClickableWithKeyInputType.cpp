#include "Krystal.Booey/HTML/BaseClickableWithKeyInputType.hpp"
#include "Krystal.Booey/DOM/KeyboardEvent.hpp"
#include "Krystal.Booey/HTML/HTMLInputElement.hpp"

namespace krys::boo::html
{
  InputType::ShouldCallBaseEventHandler
    BaseClickableWithKeyInputType::HandleKeydownEvent(HTMLInputElement &element,
                                                      dom::KeyboardEvent &event) noexcept
  {
    if (event.KeyIdentifier() == u8"U+0020")
    {
      element.SetActive(true);
      // No SetDefaultHandled(): a keypress follows only if the keydown isn't handled.
      return ShouldCallBaseEventHandler::No;
    }
    return ShouldCallBaseEventHandler::Yes;
  }

  void BaseClickableWithKeyInputType::HandleKeypressEvent(HTMLInputElement &element,
                                                          dom::KeyboardEvent &event) noexcept
  {
    int32 charCode = event.CharCode();
    if (charCode == '\r')
    {
      (void)element.DispatchSimulatedClick(&event);
      event.SetDefaultHandled();
      return;
    }
    if (charCode == ' ')
    {
      // Prevent scrolling down the page.
      event.SetDefaultHandled();
    }
  }

  void BaseClickableWithKeyInputType::HandleKeyupEvent(InputType &inputType,
                                                       dom::KeyboardEvent &event) noexcept
  {
    if (event.KeyIdentifier() != u8"U+0020")
    {
      return;
    }
    // Simulate mouse click for spacebar for button types.
    inputType.DispatchSimulatedClickIfActive(event);
  }

  InputType::ShouldCallBaseEventHandler
    BaseClickableWithKeyInputType::HandleKeydownEvent(dom::KeyboardEvent &event) noexcept
  {
    krys_debug_assert(Element());
    return HandleKeydownEvent(*Element(), event);
  }

  void BaseClickableWithKeyInputType::HandleKeypressEvent(dom::KeyboardEvent &event) noexcept
  {
    krys_debug_assert(Element());
    HandleKeypressEvent(*Element(), event);
  }

  void BaseClickableWithKeyInputType::HandleKeyupEvent(dom::KeyboardEvent &event) noexcept
  {
    HandleKeyupEvent(*this, event);
  }
}
