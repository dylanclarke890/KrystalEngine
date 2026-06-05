#include "Krystal.HTML/DOM/NamedNodeMap.hpp"
#include "Krystal.HTML/DOM/Algorithms/ElementAlgorithms.hpp"
#include "Krystal.HTML/DOM/Attr.hpp"
#include "Krystal.HTML/DOM/Element.hpp"
#include "Krystal.HTML/DOM/HTMLDocument.hpp"
#include "Krystal.HTML/Infra/Namespaces.hpp"
#include "Krystal.Text/ASCII.hpp"
#include <ranges>

namespace Krys::HTML
{
  size_t NamedNodeMap::Length() const noexcept
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

  KRYS_NODISCARD RefPtr<const Attr> NamedNodeMap::Item(size_t index) const noexcept
  {
    return const_cast<NamedNodeMap *>(this)->Item(index);
  }

  RefPtr<Attr> NamedNodeMap::GetNamedItem(DOMStringAtom qualifiedName) noexcept
  {
    return ShareRefPtr(ElementAlgorithms::GetAttributeByName(qualifiedName, *_associatedElement));
  }

  KRYS_NODISCARD RefPtr<const Attr> NamedNodeMap::GetNamedItem(DOMStringAtom qualifiedName) const noexcept
  {
    return const_cast<NamedNodeMap *>(this)->GetNamedItem(qualifiedName);
  }

  RefPtr<Attr> NamedNodeMap::GetNamedItemNS(DOMStringAtom attrNamespace, DOMStringAtom localName) noexcept
  {
    return ShareRefPtr(
      ElementAlgorithms::GetAttributeByNamespace(attrNamespace, localName, *_associatedElement));
  }

  KRYS_NODISCARD RefPtr<const Attr> NamedNodeMap::GetNamedItemNS(DOMStringAtom attrNamespace,
                                                                 DOMStringAtom localName) const noexcept
  {
    return const_cast<NamedNodeMap *>(this)->GetNamedItemNS(attrNamespace, localName);
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

  KRYS_NODISCARD RefPtr<const Attr> NamedNodeMap::operator[](size_t index) const noexcept
  {
    return Item(index);
  }

  RefPtr<Attr> NamedNodeMap::operator[](DOMStringAtom qualifiedName) noexcept
  {
    return GetNamedItem(qualifiedName);
  }

  RefPtr<const Attr> NamedNodeMap::operator[](DOMStringAtom qualifiedName) const noexcept
  {
    return GetNamedItem(qualifiedName);
  }

  List<DOMString> NamedNodeMap::SupportedPropertyNames() const noexcept
  {
    auto names = std::ranges::views::transform(_associatedElement->_attributes,
                                               [](const Ref<Attr> &attr) { return attr->Name(); })
                 | std::ranges::to<List<DOMString>>();

    if (_associatedElement->_qualifiedName.NamespaceURI == Namespace::HTML
        && Is<HTMLDocument>(_associatedElement->NodeDocument()))
    {
      List<DOMString> newNames;
      for (auto &name : names)
      {
        if (name == Krys::Text::ToASCIILowercase(name))
        {
          newNames.push_back(name);
        }
      }

      return newNames;
    }

    return names;
  }
}