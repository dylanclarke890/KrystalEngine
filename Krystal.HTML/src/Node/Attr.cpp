#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/DOM/AbortSignal.hpp"
#include "Krystal.HTML/Algorithms/ElementAlgorithms.hpp"
#include "Krystal.HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/HTMLElement/HTMLSlotElement.hpp"
#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.HTML/Node/NodeList.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"

namespace Krys::HTML
{
  Attr::Attr(Document &document, const QualifiedName &qualifiedName, DOMString &&value) noexcept
      : Node(document, NodeType::ATTRIBUTE_NODE, NodeFlag::None), _name {qualifiedName},
        _value {Krys::Move(value)}
  {
  }

  void Attr::SetExistingAttributeValue(Attr &attribute, DOMString &&value) noexcept
  {
    RefPtr<Element> element = attribute._ownerElement ? attribute._ownerElement.lock() : nullptr;
    if (element == nullptr)
    {
      attribute._value = Krys::Move(value);
    }
    else
    {
      // SPEC-VIOLATION(TRUSTED-TYPES): Let verifiedValue be the result of calling get trusted type compliant
      // attribute value with attribute’s local name, attribute’s namespace, element, and value.

      ElementAlgorithms::ChangeAttribute(attribute, Krys::Move(value));
    }
  }
}