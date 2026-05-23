#include "Krystal.HTML/Node/NamedNodeMap.hpp"
#include "Krystal.HTML/Abort/AbortSignal.hpp"
#include "Krystal.HTML/Algorithms/ElementAlgorithms.hpp"
#include "Krystal.HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/HTMLElement/HTMLSlotElement.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.HTML/Node/NodeList.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"

namespace Krys::HTML
{
  size_t NamedNodeMap::Length() noexcept
  {
    return _associatedElement->_attributes.size();
  }

  RefPtr<Attr> NamedNodeMap::Item(size_t index) noexcept
  {
    if (index < _associatedElement->_attributes.size())
    {
      return _associatedElement->_attributes[index];
    }

    return nullptr;
  }

  RefPtr<Attr> NamedNodeMap::GetNamedItem(DOMStringAtom qualifiedName) noexcept
  {
    return ShareRefPtr(ElementAlgorithms::GetAttributeByName(qualifiedName, *_associatedElement));
  }

  RefPtr<Attr> NamedNodeMap::GetNamedItemNS(DOMStringAtom attrNamespace, DOMStringAtom localName) noexcept
  {
    return ShareRefPtr(
      ElementAlgorithms::GetAttributeByNamespace(attrNamespace, localName, *_associatedElement));
  }

  ExceptionOr<RefPtr<Attr>> NamedNodeMap::SetNamedItem(Attr &attr) noexcept
  {
    return ElementAlgorithms::SetAttribute(attr, *_associatedElement);
  }

  ExceptionOr<RefPtr<Attr>> NamedNodeMap::SetNamedItemNS(Attr &attr) noexcept
  {
    return ElementAlgorithms::SetAttribute(attr, *_associatedElement);
  }

  ExceptionOr<Ref<Attr>> NamedNodeMap::RemoveNamedItem(DOMStringAtom qualifiedName) noexcept
  {
    auto attr = ElementAlgorithms::RemoveAttributeByName(qualifiedName, *_associatedElement);
    if (attr == nullptr)
    {
      return Exception {ExceptionCode::NotFoundError};
    }

    return ShareRef(*attr);
  }

  ExceptionOr<Ref<Attr>> NamedNodeMap::RemoveNamedItemNS(DOMStringAtom attrNamespace,
                                                         DOMStringAtom localName) noexcept
  {
    auto attr = ElementAlgorithms::RemoveAttributeByNamespace(attrNamespace, localName, *_associatedElement);
    if (attr == nullptr)
    {
      return Exception {ExceptionCode::NotFoundError};
    }

    return ShareRef(*attr);
  }

  RefPtr<Attr> NamedNodeMap::operator[](size_t index) noexcept
  {
    return Item(index);
  }

  RefPtr<Attr> NamedNodeMap::operator[](DOMStringAtom qualifiedName) noexcept
  {
    return GetNamedItem(qualifiedName);
  }
}