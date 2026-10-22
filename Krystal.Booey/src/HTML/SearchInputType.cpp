#include "Krystal.Booey/HTML/SearchInputType.hpp"
#include "Krystal.Booey/DOM/KeyboardEvent.hpp"
#include "Krystal.Booey/HTML/HTMLInputElement.hpp"

namespace krys::boo::html
{
  InputType::ShouldCallBaseEventHandler
    SearchInputType::HandleKeydownEvent(dom::KeyboardEvent &event) noexcept
  {
    krys_debug_assert(Element());
    auto &element = *Element();
    if (!element.IsMutable())
    {
      return TextFieldInputType::HandleKeydownEvent(event);
    }

    if (event.KeyIdentifier() == u8"U+001B")
    {
      (void)element.SetValue({}, TextFieldEventBehavior::DispatchChangeEvent);
      event.SetDefaultHandled();
      return ShouldCallBaseEventHandler::Yes;
    }
    return TextFieldInputType::HandleKeydownEvent(event);
  }
}
