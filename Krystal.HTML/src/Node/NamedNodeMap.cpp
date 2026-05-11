#include "Krystal.HTML/Node/NamedNodeMap.hpp"
#include "Krystal.HTML/Abort/AbortSignal.hpp"
#include "Krystal.HTML/Algorithms/ElementAlgorithms.hpp"
#include "Krystal.HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.HTML/Node/NodeList.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"

namespace Krys::HTML
{
  size_t NamedNodeMap::Length() noexcept
  {
    if (auto element = _associatedElement.lock())
    {
      return element->_attributes.size();
    }

    return 0uz;
  }

  RefPtr<Attr> NamedNodeMap::Item(size_t index) noexcept
  {
    if (auto element = _associatedElement.lock())
    {
      if (index < element->_attributes.size())
      {
        return element->_attributes[index];
      }
    }

    return nullptr;
  }

  RefPtr<Attr> NamedNodeMap::GetNamedItem(DOMStringAtom qualifiedName) noexcept
  {
    if (auto element = _associatedElement.lock())
    {
      return ShareRefPtr(ElementAlgorithms::GetAttributeByName(qualifiedName, *element));
    }

    return nullptr;
  }

  RefPtr<Attr> NamedNodeMap::GetNamedItemNS(DOMStringAtom attrNamespace, DOMStringAtom localName) noexcept
  {
    if (auto element = _associatedElement.lock())
    {
      return ShareRefPtr(ElementAlgorithms::GetAttributeByNamespace(attrNamespace, localName, *element));
    }

    return nullptr;
  }

  ExceptionOr<RefPtr<Attr>> NamedNodeMap::SetNamedItem(Attr &attr) noexcept
  {
    if (auto element = _associatedElement.lock())
    {
      return ElementAlgorithms::SetAttribute(attr, *element);
    }

    return nullptr;
  }

  ExceptionOr<RefPtr<Attr>> NamedNodeMap::SetNamedItemNS(Attr &attr) noexcept
  {
    if (auto element = _associatedElement.lock())
    {
      return ElementAlgorithms::SetAttribute(attr, *element);
    }

    return nullptr;
  }

  ExceptionOr<Ref<Attr>> NamedNodeMap::RemoveNamedItem(DOMStringAtom qualifiedName) noexcept
  {
    if (auto element = _associatedElement.lock())
    {
      auto attr = ElementAlgorithms::RemoveAttributeByName(qualifiedName, *element);
      if (attr == nullptr)
      {
        return Exception {ExceptionCode::NotFoundError};
      }

      return ShareRef(*attr);
    }

    return Exception {ExceptionCode::InvalidStateError};
  }

  ExceptionOr<Ref<Attr>> NamedNodeMap::RemoveNamedItemNS(DOMStringAtom attrNamespace,
                                                         DOMStringAtom localName) noexcept
  {
    if (auto element = _associatedElement.lock())
    {
      auto attr = ElementAlgorithms::RemoveAttributeByNamespace(attrNamespace, localName, *element);
      if (attr == nullptr)
      {
        return Exception {ExceptionCode::NotFoundError};
      }

      return ShareRef(*attr);
    }

    return Exception {ExceptionCode::InvalidStateError};
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