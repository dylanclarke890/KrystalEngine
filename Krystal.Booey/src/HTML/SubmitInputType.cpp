#include "Krystal.Booey/HTML/SubmitInputType.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Event.hpp"
#include "Krystal.Booey/HTML/DOMFormData.hpp"
#include "Krystal.Booey/HTML/HTMLFormElement.hpp"
#include "Krystal.Booey/HTML/HTMLInputElement.hpp"
#include "Krystal.Booey/Platform/LocalizedStrings.hpp"

namespace krys::boo::html
{
  bool SubmitInputType::AppendFormData(DOMFormData &formData) const noexcept
  {
    krys_debug_assert(Element());
    auto &element = *Element();
    if (!element.IsActivatedSubmit())
    {
      return false;
    }
    formData.Append(element.Name(), element.ValueWithDefault());
    if (auto dirname = element.GetAttribute(u8"dirname"))
    {
      formData.Append(*dirname, element.DirectionForFormData());
    }
    return true;
  }

  bool SubmitInputType::SupportsRequired() const noexcept
  {
    return false;
  }

  void SubmitInputType::HandleActivationBehavior(dom::Event &event) noexcept
  {
    krys_debug_assert(Element());
    Ref<HTMLInputElement> element = ShareRef(*Element());
    if (element->IsDisabledFormControl() || !element->Form())
    {
      return;
    }

    Ref<HTMLFormElement> protectedForm = ShareRef(*element->Form());

    // Update layout before processing form actions in case the style changes the form or button
    // relationships.
    (void)element->NodeDocument().UpdateLayoutIgnorePendingStylesheets();

    if (RefPtr<HTMLFormElement> currentForm = ShareRefPtr(element->Form()))
    {
      currentForm->SubmitIfPossible(&event, element.get()); // Event handlers can run.
    }
    event.SetDefaultHandled();
  }

  bool SubmitInputType::CanBeSuccessfulSubmitButton() noexcept
  {
    return true;
  }

  Maybe<dom::DOMString> SubmitInputType::DefaultValue() const noexcept
  {
    return SubmitButtonDefaultLabel();
  }
}
