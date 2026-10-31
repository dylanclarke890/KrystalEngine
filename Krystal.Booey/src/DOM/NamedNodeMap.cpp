#include "Krystal.Booey/DOM/NamedNodeMap.hpp"
#include "Krystal.Booey/DOM/Algorithms/ElementAlgorithms.hpp"
#include "Krystal.Booey/DOM/Attr.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/HTMLDocument.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementReactionQueue.hpp"
#include "Krystal.Booey/Infra/Namespaces.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include <ranges>

namespace krys::boo::dom
{
  size_t NamedNodeMap::Length() const noexcept
  {
    // Return the attribute list's size.
    _associatedElement->SynchronizeAllAttributes();
    return _associatedElement->_attributes.size();
  }

  RefPtr<Attr> NamedNodeMap::Item(size_t index) noexcept
  {
    _associatedElement->SynchronizeAllAttributes();
    // 1. If index is equal to or greater than this's attribute list's size, then return null.
    // 2. Otherwise, return this's attribute list[index].
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
    html::CEReactionsScope ceReactions;
    return ElementAlgorithms::SetAttribute(attr, *_associatedElement);
  }

  ExceptionOr<RefPtr<Attr>> NamedNodeMap::SetNamedItemNS(Attr &attr) noexcept
  {
    html::CEReactionsScope ceReactions;
    return ElementAlgorithms::SetAttribute(attr, *_associatedElement);
  }

  ExceptionOr<Ref<Attr>> NamedNodeMap::RemoveNamedItem(DOMStringAtom qualifiedName) noexcept
  {
    html::CEReactionsScope ceReactions;
    // 1. Let attr be the result of removing an attribute given qualifiedName and element.
    auto attr = ElementAlgorithms::RemoveAttributeByName(qualifiedName, *_associatedElement);
    // 2. If attr is null, then throw a "NotFoundError" DOMException.
    if (attr == nullptr)
    {
      return Exception {ExceptionCode::NotFoundError};
    }

    // 3. Return attr.
    return ShareRef(*attr);
  }

  ExceptionOr<Ref<Attr>> NamedNodeMap::RemoveNamedItemNS(DOMStringAtom attrNamespace,
                                                         DOMStringAtom localName) noexcept
  {
    html::CEReactionsScope ceReactions;
    // 1. Let attr be the result of removing an attribute given namespace, localName, and element.
    auto attr = ElementAlgorithms::RemoveAttributeByNamespace(attrNamespace, localName, *_associatedElement);
    // 2. If attr is null, then throw a "NotFoundError" DOMException.
    if (attr == nullptr)
    {
      return Exception {ExceptionCode::NotFoundError};
    }

    // 3. Return attr.
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
    // 1. Let names be the qualified names of the attributes in this NamedNodeMap object's attribute list,
    //    with duplicates omitted, in order.
    _associatedElement->SynchronizeAllAttributes();
    List<DOMString> names;
    for (auto &attr : _associatedElement->_attributes)
    {
      auto name = attr->Name();
      if (std::ranges::find(names, name) == names.end())
      {
        names.push_back(krys::move(name));
      }
    }

    // 2. If this NamedNodeMap object's element is in the HTML namespace and its node document is an HTML
    //    document, then for each name of names:
    if (_associatedElement->NamespaceURI() == infra::Namespaces::HTML
        && Is<HTMLDocument>(_associatedElement->NodeDocument()))
    {
      // 2a. Let lowercaseName be name, in ASCII lowercase.
      // 2b. If lowercaseName is not equal to name, remove name from names.
      std::erase_if(names, [](const DOMString &name) { return name != krys::text::ToASCIILower(name); });
    }

    // 3. Return names.
    return names;
  }
}