#include "Krystal.HTML/Algorithms/ElementAlgorithms.hpp"
#include "Krystal.HTML/Abort/AbortSignal.hpp"
#include "Krystal.HTML/Algorithms/ExtensibilityHooks.hpp"
#include "Krystal.HTML/Algorithms/MutationAlgorithms.hpp"
#include "Krystal.HTML/Algorithms/TreeMutationDispatcher.hpp"
#include "Krystal.HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/HTMLElement/HTMLSlotElement.hpp"
#include "Krystal.HTML/Namespaces.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.HTML/Node/HTMLDocument.hpp"
#include "Krystal.HTML/Node/NodeList.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"
#include "Krystal.Text/ASCII.hpp"

namespace Krys::HTML
{
  ExceptionOr<RawPtr<Node>> ElementAlgorithms::InsertAdjacent(Element &element, InsertAdjacentWhere where,
                                                              Node &node) noexcept
  {
    switch (where)
    {
      case InsertAdjacentWhere::BeforeBegin:
      {
        if (element.ParentNode() == nullptr)
        {
          return nullptr;
        }

        auto result = MutationAlgorithms::PreInsert(node, *element.ParentNode(), &element);
        if (result.HasException())
        {
          return result.ReleaseException();
        }

        return &result.Value();
      }
      case InsertAdjacentWhere::AfterBegin:
      {
        auto result = MutationAlgorithms::PreInsert(node, element, element.FirstChild());
        if (result.HasException())
        {
          return result.ReleaseException();
        }

        return &result.Value();
      }
      case InsertAdjacentWhere::BeforeEnd:
      {
        auto result = MutationAlgorithms::PreInsert(node, element, nullptr);
        if (result.HasException())
        {
          return result.ReleaseException();
        }

        return &result.Value();
      }
      case InsertAdjacentWhere::AfterEnd:
      {
        auto parent = element.ParentNode();
        if (parent == nullptr)
        {
          return nullptr;
        }

        auto result = MutationAlgorithms::PreInsert(node, *parent, element.NextSibling());
        if (result.HasException())
        {
          return result.ReleaseException();
        }

        return &result.Value();
      }
      default: return Exception {ExceptionCode::SyntaxError};
    }
  }

  void ElementAlgorithms::HandleAttributeChanges(Attr &attribute, Element &element, DOMStringView oldValue,
                                                 DOMStringView newValue) noexcept
  {
    TreeMutationDispatcher::QueueMutationRecord(u8"attributes", element, attribute.LocalName(),
                                                attribute.NamespaceURI(), oldValue, {}, {}, nullptr, nullptr);

    // TODO(impl): CUSTOM-ELEMENT
    // If element is custom, then enqueue a custom element callback reaction with element, callback name
    // "attributeChangedCallback", and « attribute’s local name, oldValue, newValue, attribute’s namespace ».

    ExtensibilityHooks::ElementAttributeChanged(element, attribute.LocalName(), oldValue, attribute.Value(),
                                                attribute.NamespaceURI());
  }

  void ElementAlgorithms::ChangeAttribute(Attr &attribute, DOMString &&value) noexcept
  {
    DOMString &&oldValue = Krys::Move(attribute._value);

    attribute._value = Krys::Move(value);

    HandleAttributeChanges(attribute, *attribute._ownerElement, oldValue, attribute._value);
  }

  void ElementAlgorithms::AppendAttribute(Attr &attribute, Element &element) noexcept
  {
    element._attributes.push_back(ShareRef(attribute));

    attribute._ownerElement = CreateWeakPtr(&element);
    attribute._ownerDocument = ShareRefPtr(&element.NodeDocument());

    HandleAttributeChanges(attribute, element, {}, attribute._value);
  }

  void ElementAlgorithms::RemoveAttribute(Attr &attribute) noexcept
  {
    assert(attribute._ownerElement);
    RefPtr<Element> element = attribute._ownerElement.lock();
    assert(element);

    Ref<Attr> attributeRef = ShareRef(attribute);
    element->_attributes.erase(std::remove_if(element->_attributes.begin(), element->_attributes.end(),
                                              [&](const Ref<Attr> &attr)
                                              { return attr.get() == &attribute; }));

    attribute._ownerElement.reset();

    HandleAttributeChanges(attribute, *element, attribute._value, {});
  }

  void ElementAlgorithms::ReplaceAttribute(Attr &oldAttribute, Attr &newAttribute) noexcept
  {
    assert(oldAttribute._ownerElement);
    RefPtr<Element> element = oldAttribute._ownerElement.lock();
    assert(element);

    std::replace_if(
      element->_attributes.begin(), element->_attributes.end(),
      [&](const Ref<Attr> &attr) { return attr.get() == &oldAttribute; }, ShareRef(newAttribute));

    newAttribute._ownerElement = CreateWeakPtr(element.get());
    newAttribute._ownerDocument = ShareRefPtr(&element->NodeDocument());
    oldAttribute._ownerElement.reset();

    HandleAttributeChanges(oldAttribute, *element, oldAttribute._value, newAttribute._value);
  }

  RawPtr<Attr> ElementAlgorithms::GetAttributeByName(DOMStringAtom qualifiedName,
                                                     const Element &element) noexcept
  {
    if (element.NamespaceURI() == Namespaces::HTML && Is<HTMLDocument>(element.NodeDocument()))
    {
      qualifiedName = Krys::Text::ToASCIILowercase(qualifiedName.View());
    }

    auto it = std::find_if(element._attributes.begin(), element._attributes.end(),
                           [&](const Ref<Attr> &attr) { return attr->Name() == qualifiedName; });
    return it != element._attributes.end() ? it->get() : nullptr;
  }

  RawPtr<Attr> ElementAlgorithms::GetAttributeByNamespace(DOMStringAtom namespaceURI, DOMStringAtom localName,
                                                          const Element &element) noexcept
  {
    if (namespaceURI == DOMStringAtom::Empty())
    {
      namespaceURI = DOMStringAtom::Null();
    }

    auto it =
      std::find_if(element._attributes.begin(), element._attributes.end(), [&](const Ref<Attr> &attr)
                   { return attr->LocalName() == localName && attr->NamespaceURI() == namespaceURI; });
    return it != element._attributes.end() ? it->get() : nullptr;
  }

  DOMString ElementAlgorithms::GetAttributeValue(const Element &element, DOMStringAtom localName,
                                                 DOMStringAtom namespaceURI) noexcept
  {
    RawPtr<Attr> attr = GetAttributeByNamespace(namespaceURI, localName, element);
    if (attr == nullptr)
    {
      return u8"";
    }

    return attr->Value();
  }

  ExceptionOr<RefPtr<Attr>> ElementAlgorithms::SetAttribute(Attr &attr, Element &element) noexcept
  {
    // SPEC-VIOLATION(TRUSTED-TYPES): Let verifiedValue be the result of calling get trusted type compliant
    // attribute value with attr’s local name, attr’s namespace, element, and attr’s value.

    if (attr._ownerElement != nullptr && attr._ownerElement.get() != &element)
    {
      return Exception {ExceptionCode::InUseAttributeError};
    }

    auto oldAttr = ShareRefPtr(GetAttributeByNamespace(attr.NamespaceURI(), attr.LocalName(), element));
    if (&attr == oldAttr)
    {
      return oldAttr;
    }

    // SPEC-VIOLATION(TRUSTED-TYPES): Set attr’s value to verifiedValue.

    if (oldAttr != nullptr)
    {
      ReplaceAttribute(*oldAttr, attr);
    }
    else
    {
      AppendAttribute(attr, element);
    }

    return oldAttr;
  }

  void ElementAlgorithms::SetAttributeValue(Element &element, DOMStringAtom localName, DOMString &&value,
                                            DOMStringAtom prefix, DOMStringAtom namespaceURI) noexcept
  {
    RawPtr<Attr> attribute = GetAttributeByNamespace(namespaceURI, localName, element);
    if (attribute == nullptr)
    {
      auto attr = AdoptRef<Attr>(*new Attr(
        element.NodeDocument(), QualifiedName {namespaceURI, prefix, localName}, Krys::Move(value)));
      AppendAttribute(*attr, element);
    }
    else
    {
      ChangeAttribute(*attribute, Krys::Move(value));
    }
  }

  RefPtr<Attr> ElementAlgorithms::RemoveAttributeByName(DOMStringAtom qualifiedName,
                                                        Element &element) noexcept
  {
    auto attr = ShareRefPtr(GetAttributeByName(qualifiedName, element));
    if (attr != nullptr)
    {
      RemoveAttribute(*attr);
    }

    return attr;
  }

  RefPtr<Attr> ElementAlgorithms::RemoveAttributeByNamespace(DOMStringAtom namespaceURI,
                                                             DOMStringAtom localName,
                                                             Element &element) noexcept
  {
    auto attr = ShareRefPtr(GetAttributeByNamespace(namespaceURI, localName, element));
    if (attr != nullptr)
    {
      RemoveAttribute(*attr);
    }

    return attr;
  }
}