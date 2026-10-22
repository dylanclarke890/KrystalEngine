#include "Krystal.Booey/HTML/HTMLLegendElement.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"
#include "Krystal.Booey/HTML/HTMLFieldSetElement.hpp"
#include "Krystal.Booey/HTML/HTMLFormElement.hpp"

namespace krys::boo::html
{
  HTMLLegendElement::HTMLLegendElement(dom::Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::Legend)
  {
  }

#pragma region HTMLLegendElement

  HTMLFormElement *HTMLLegendElement::Form() const noexcept
  {
    // According to the specification, If the legend has a fieldset element as its parent, then the form
    // attribute must return the same value as the form attribute on that fieldset element. Otherwise, it must
    // return null.
    auto *fieldset = DynamicDowncast<HTMLFieldSetElement>(ParentNode());
    return fieldset ? fieldset->Form() : nullptr;
  }

  RefPtr<HTMLFormElement> HTMLLegendElement::FormForBindings() const noexcept
  {
    // FIXME: Retargeting against the legend's tree, once there are reference targets.
    return ShareRefPtr(Form());
  }

#pragma endregion

#pragma region HTMLLegendElement Obsolete members

  dom::DOMString HTMLLegendElement::Align() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"align").value_or(u8"");
  }

  void HTMLLegendElement::Align(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"align", krys::move(value));
  }

#pragma endregion
}