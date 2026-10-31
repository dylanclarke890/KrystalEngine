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
    // 1. If element is a slot, localName is name, and namespace is null:
    if (localName != Attributes::Name || namespaceURI != dom::DOMStringAtom::Null())
    {
      return;
    }

    // 1a. If value is oldValue, then return.
    if (value == oldValue)
    {
      return;
    }

    // 1b. If value is null and oldValue is the empty string, then return.
    if (!value.has_value() && oldValue.has_value() && oldValue->empty())
    {
      return;
    }

    // 1c. If value is the empty string and oldValue is null, then return.
    if (value.has_value() && value->empty() && !oldValue.has_value())
    {
      return;
    }

    // 1d. If value is null or the empty string, then set element's name to the empty string.
    // 1e. Otherwise, set element's name to value.
    // The name is read from the attribute (Name()), which already holds value.

    // 1f. Run assign slottables for a tree with element's root.
    dom::SlotAlgorithms::AssignSlottablesForTree(dom::TreeQueries::Root(*this));
  }
}
