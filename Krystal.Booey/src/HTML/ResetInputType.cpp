#include "Krystal.Booey/HTML/ResetInputType.hpp"
#include "Krystal.Booey/DOM/Event.hpp"
#include "Krystal.Booey/HTML/HTMLFormElement.hpp"
#include "Krystal.Booey/HTML/HTMLInputElement.hpp"
#include "Krystal.Booey/Platform/LocalizedStrings.hpp"

namespace krys::boo::html
{
  void ResetInputType::HandleActivationBehavior(dom::Event &event) noexcept
  {
    krys_debug_assert(Element());
    if (Element()->IsDisabledFormControl())
    {
      return;
    }

    RefPtr<HTMLFormElement> form = ShareRefPtr(Element()->Form());
    if (!form)
    {
      return;
    }

    form->Reset();
    event.SetDefaultHandled();
  }

  Maybe<dom::DOMString> ResetInputType::DefaultValue() const noexcept
  {
    return ResetButtonDefaultLabel();
  }
}
