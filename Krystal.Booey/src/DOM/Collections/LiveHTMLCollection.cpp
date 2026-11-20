#include "Krystal.Booey/DOM/Collections/LiveHTMLCollection.hpp"
#include "Krystal.Booey/DOM/Algorithms/ElementAlgorithms.hpp"
#include "Krystal.Booey/DOM/Nodes/Attr.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/DOM/Nodes/ShadowRoot.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLSlotElement.hpp"
#include "Krystal.Booey/Infra/Namespaces.hpp"

namespace krys::boo
{
  dom::LiveHTMLCollection::LiveHTMLCollection(ContainerNode &root, dom::LiveHTMLCollectionFilterFunc &&filter,
                                              dom::LiveCollectionInvalidation invalidation,
                                              dom::LiveCollectionTraversal traversal) noexcept
      : dom::LiveCollection(root, invalidation, traversal), _filter(krys::move(filter))
  {
  }

  dom::LiveHTMLCollection::~LiveHTMLCollection() noexcept
  {
    ForgetNamedItems();
  }

  void dom::LiveHTMLCollection::InvalidateCache() noexcept
  {
    _indexCache.Invalidate();
    ForgetNamedItems();
    UnregisterCache();
  }

  void dom::LiveHTMLCollection::InvalidateCacheForAttribute(DOMStringAtom localName) noexcept
  {
    if (localName == u8"id" || localName == u8"name")
    {
      ForgetNamedItems();
    }
    dom::LiveCollection::InvalidateCacheForAttribute(localName);
  }

  void dom::LiveHTMLCollection::WillValidateIndexCache() const noexcept
  {
    RegisterCache();
  }

  Element *dom::LiveHTMLCollection::FirstMatchFrom(Node *node) const noexcept
  {
    for (; node; node = NextNode(*node))
    {
      if (auto *element = DynamicDowncast<Element>(node); element && _filter(*element))
      {
        return element;
      }
    }
    return nullptr;
  }

  Element *dom::LiveHTMLCollection::CollectionBegin() const noexcept
  {
    return FirstMatchFrom(FirstNode());
  }

  Element *dom::LiveHTMLCollection::CollectionNext(Element &element) const noexcept
  {
    return FirstMatchFrom(NextNode(element));
  }

  void dom::LiveHTMLCollection::ForgetNamedItems() noexcept
  {
    if (_hasNamedItems)
    {
      _hasNamedItems = false;
      _namedItems.clear();
      Root().NodeDocument().LiveCollectionCacheRemoved(dom::LiveCollectionInvalidation::IdOrName);
    }
  }

  RefPtr<Element> dom::LiveHTMLCollection::Item(size_t index) noexcept
  {
    return ShareRefPtr(_indexCache.At(*this, index));
  }

  RefPtr<const Element> dom::LiveHTMLCollection::Item(size_t index) const noexcept
  {
    return const_cast<dom::LiveHTMLCollection *>(this)->Item(index);
  }

  RefPtr<Element> dom::LiveHTMLCollection::operator[](size_t index) noexcept
  {
    return Item(index);
  }

  RefPtr<const Element> dom::LiveHTMLCollection::operator[](size_t index) const noexcept
  {
    return Item(index);
  }

  RefPtr<Element> dom::LiveHTMLCollection::NamedItem(DOMStringView name) noexcept
  {
    // 1. If key is the empty string, return null.
    if (name.empty())
    {
      return nullptr;
    }

    // 2. Return the first element in the collection for which at least one of the following is true:
    //    - it has an ID which is key;
    //    - it is in the HTML namespace and has a name attribute whose value is key;
    //    or null if there is no such element.
    if (!_hasNamedItems)
    {
      for (size_t i = 0, count = _indexCache.Count(*this); i < count; ++i)
      {
        auto *element = _indexCache.At(*this, i);
        if (auto id = element->Id(); !id.empty())
        {
          _namedItems.try_emplace(DOMString(id), element);
        }
        if (element->NamespaceURI() == infra::namespaces::HTML)
        {
          if (auto elementName = dom::ElementAlgorithms::GetAttributeValue(*element, u8"name");
              !elementName.empty())
          {
            _namedItems.try_emplace(DOMString(elementName), element);
          }
        }
      }
      _hasNamedItems = true;
      Root().NodeDocument().LiveCollectionCacheAdded(dom::LiveCollectionInvalidation::IdOrName);
    }

    auto it = _namedItems.find(DOMString(name));
    return it != _namedItems.end() ? ShareRefPtr(it->second) : nullptr;
  }

  RefPtr<const Element> dom::LiveHTMLCollection::NamedItem(DOMStringView name) const noexcept
  {
    return const_cast<dom::LiveHTMLCollection *>(this)->NamedItem(name);
  }

  RefPtr<Element> dom::LiveHTMLCollection::operator[](DOMStringView name) noexcept
  {
    return NamedItem(name);
  }

  RefPtr<const Element> dom::LiveHTMLCollection::operator[](DOMStringView name) const noexcept
  {
    return NamedItem(name);
  }

  size_t dom::LiveHTMLCollection::Length() const noexcept
  {
    return _indexCache.Count(*this);
  }

  List<DOMString> dom::LiveHTMLCollection::SupportedPropertyNames() const noexcept
  {
    // 1. Let result be an empty list.
    List<DOMString> supportedNames;

    // 2. For each element represented by the collection, in tree order:
    for (size_t i = 0, count = _indexCache.Count(*this); i < count; ++i)
    {
      auto &element = *_indexCache.At(*this, i);

      // 2a. If element has an ID which is not in result, append element's ID to result.
      auto id = element.Id();
      if (!id.empty() && !std::ranges::contains(supportedNames, id))
      {
        supportedNames.push_back(id);
      }

      // 2b. If element is in the HTML namespace and has a name attribute whose value is neither the empty
      // string
      //     nor is in result, append element's name attribute value to result.
      if (element.NamespaceURI() == infra::namespaces::HTML)
      {
        auto name = dom::ElementAlgorithms::GetAttributeValue(element, u8"name");
        if (!name.empty() && !std::ranges::contains(supportedNames, name))
        {
          supportedNames.push_back(name);
        }
      }
    }

    // 3. Return result.
    return supportedNames;
  }
}
