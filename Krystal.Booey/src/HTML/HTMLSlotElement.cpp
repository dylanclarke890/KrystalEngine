#include "Krystal.Booey/HTML/HTMLSlotElement.hpp"
#include "Krystal.Booey/Constants/Attributes.hpp"
#include "Krystal.Booey/DOM/Algorithms/SlotAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"

namespace krys::boo::html
{
  HTMLSlotElement::HTMLSlotElement(dom::Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::Slot, HTMLElementFlags::None,
                    dom::NodeFlags::IsHTMLSlotElement)
  {
  }

#pragma region HTMLSlotElement

  dom::DOMString HTMLSlotElement::Name() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"name").value_or(u8"");
  }

  void HTMLSlotElement::Name(dom::DOMString &&name) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"name", krys::move(name));
  }

#pragma endregion

  void HTMLSlotElement::OnAttributeChanged(dom::DOMStringAtom localName, Maybe<dom::DOMStringView> oldValue,
                                           Maybe<dom::DOMStringView> value,
                                           dom::DOMStringAtom namespaceURI) noexcept
  {
    HTMLElement::OnAttributeChanged(localName, oldValue, value, namespaceURI);

    // https://dom.spec.whatwg.org/#slot-name
    if (localName != Attributes::Name || namespaceURI != dom::DOMStringAtom::Null())
    {
      return;
    }

    if (value == oldValue)
    {
      return;
    }

    if (!value.has_value() && oldValue.has_value() && oldValue->empty())
    {
      return;
    }

    if (value.has_value() && value->empty() && !oldValue.has_value())
    {
      return;
    }

    // NOTE: the slot's name is read from the `name` attribute (see `Name()`), which already holds the new
    // value, so there's nothing to store.

    dom::SlotAlgorithms::AssignSlottablesForTree(dom::TreeQueries::Root(*this));
  }
}
