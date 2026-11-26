#include "Krystal.Web/Dom/Nodes/Attr.hpp"
#include "Krystal.Web/Dom/Nodes/Algorithms/ElementAlgorithms.hpp"
#include "Krystal.Web/Dom/Nodes/Element.hpp"
#include "Krystal.Web/Html/CustomElement/CustomElementReactionQueue.hpp"

namespace krys::web
{
  Attr::Attr(Document &document, const web::QualifiedName &qualifiedName, DOMString value) noexcept
      : Node(document, dom::NodeType::ATTRIBUTE_NODE, {}), _name {qualifiedName}, _value {krys::move(value)}
  {
    // 1. Let attribute be the result of creating a node that implements Attr, given document.
    // 2. Set attribute's namespace to namespace, namespace prefix to prefix, local name to localName, and
    //    value to value.
    // 3. Return attribute.
  }

  const DOMString &Attr::Value() const noexcept
  {
    if (auto *element = _ownerElement.get())
    {
      element->SynchronizeAllAttributes();
    }

    return _value;
  }

  Element *Attr::OwnerElement() const noexcept
  {
    return _ownerElement.get();
  }

  void Attr::Value(DOMString value) noexcept
  {
    html::CEReactionsScope ceReactions;
    SetExistingAttributeValue(*this, krys::move(value));
  }

  ExceptionOr<void> Attr::NodeValue(DOMString value) noexcept
  {
    html::CEReactionsScope ceReactions;
    SetExistingAttributeValue(*this, krys::move(value));
    return {};
  }

  ExceptionOr<void> Attr::TextContent(DOMString value) noexcept
  {
    html::CEReactionsScope ceReactions;
    SetExistingAttributeValue(*this, krys::move(value));
    return {};
  }

  void Attr::SetExistingAttributeValue(Attr &attribute, DOMString value) noexcept
  {
    // 1. If attribute's element is null, then set attribute's value to value and return.
    // 2. Let element be attribute's element.
    RefPtr<Element> element = attribute._ownerElement ? attribute._ownerElement.lock() : nullptr;
    if (element == nullptr)
    {
      attribute._value = krys::move(value);
    }
    else
    {
      // 3. (Skipped: Trusted Types aren't supported, so value is used as it is.)
      // 4. (Skipped: without step 3, attribute's element can't have changed.)

      // 5. Change attribute to verifiedValue.
      dom::ElementAlgorithms::ChangeAttribute(attribute, krys::move(value));
    }
  }
}
