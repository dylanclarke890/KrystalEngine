#include "Krystal.HTML/DOM/Attr.hpp"
#include "Krystal.HTML/DOM/Algorithms/ElementAlgorithms.hpp"

namespace Krys::HTML
{
  Attr::Attr(Document &document, const QualifiedName &qualifiedName, DOMString &&value) noexcept
      : Node(document, NodeType::ATTRIBUTE_NODE, NodeFlags::None), _name {qualifiedName},
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