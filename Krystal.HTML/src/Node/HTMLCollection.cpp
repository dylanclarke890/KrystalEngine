#include "Krystal.HTML/Node/HTMLCollection.hpp"
#include "Krystal.HTML/Abort/AbortSignal.hpp"
#include "Krystal.HTML/Algorithms/TreeQueries.hpp"
#include "Krystal.HTML/MutationObserver/MutationObserver.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/Node/ContainerNode.hpp"
#include "Krystal.HTML/Node/CustomElementRegistry.hpp"
#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.HTML/Node/NodeList.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"
#include "Krystal.HTML/Utils/SubtreeRanges.hpp"

namespace Krys::HTML
{
  LiveHTMLCollection::LiveHTMLCollection(WeakRef<ContainerNode> &&root,
                                         LiveHTMLCollectionFilterFunc &&filter) noexcept
      : HTMLCollection(NodeCollectionLiveness::Live), _root(Krys::Move(root)), _filter(Krys::Move(filter))
  {
  }

  RawPtr<Element> LiveHTMLCollection::Item(size_t index) noexcept
  {
    if (auto root = _root.lock())
    {
      size_t count = 0;
      for (Node &node : DescendantRange(*root))
      {
        if (auto element = DynamicDowncast<Element>(node))
        {
          if (_filter(*element))
          {
            ++count;
          }

          if (count > index)
          {
            return element;
          }
        }
      }
    }

    return nullptr;
  }

  RawPtr<const Element> LiveHTMLCollection::Item(size_t index) const noexcept
  {
    if (auto root = _root.lock())
    {
      size_t count = 0;
      for (const Node &node : DescendantRange(*root))
      {
        if (auto element = DynamicDowncast<Element>(node))
        {
          if (_filter(*element))
          {
            ++count;
          }

          if (count > index)
          {
            return element;
          }
        }
      }
    }

    return nullptr;
  }

  RawPtr<Element> LiveHTMLCollection::operator[](size_t index) noexcept
  {
    return Item(index);
  }

  RawPtr<const Element> LiveHTMLCollection::operator[](size_t index) const noexcept
  {
    return Item(index);
  }

  RawPtr<Element> LiveHTMLCollection::NamedItem(DOMStringView name) noexcept
  {
    // TODO(impl):
    return nullptr;
  }

  RawPtr<const Element> LiveHTMLCollection::NamedItem(DOMStringView name) const noexcept
  {
    // TODO(impl):
    return nullptr;
  }

  RawPtr<Element> LiveHTMLCollection::operator[](DOMStringView name) noexcept
  {
    return NamedItem(name);
  }

  RawPtr<const Element> LiveHTMLCollection::operator[](DOMStringView name) const noexcept
  {
    return NamedItem(name);
  }

  size_t LiveHTMLCollection::Length() const noexcept
  {
    if (auto root = _root.lock())
    {
      return Count(ConstDescendantRange(*root),
                   [&](auto &&node) { return Is<Element>(node) && _filter(Downcast<Element>(node)); });
    }

    return 0uz;
  }

  StaticHTMLCollection::StaticHTMLCollection(SmallElementList &&elements) noexcept
      : HTMLCollection(NodeCollectionLiveness::Static), _elements(Krys::Move(elements))
  {
  }

  RawPtr<Element> StaticHTMLCollection::Item(size_t index) noexcept
  {
    if (index < _elements.size())
    {
      return _elements[index].get();
    }

    return nullptr;
  }

  RawPtr<const Element> StaticHTMLCollection::Item(size_t index) const noexcept
  {
    if (index < _elements.size())
    {
      return _elements[index].get();
    }

    return nullptr;
  }

  RawPtr<Element> StaticHTMLCollection::operator[](size_t index) noexcept
  {
    return Item(index);
  }

  RawPtr<const Element> StaticHTMLCollection::operator[](size_t index) const noexcept
  {
    return Item(index);
  }

  RawPtr<Element> StaticHTMLCollection::NamedItem(DOMStringView name) noexcept
  {
    // TODO(impl):
    return nullptr;
  }

  RawPtr<const Element> StaticHTMLCollection::NamedItem(DOMStringView name) const noexcept
  {
    // TODO(impl):
    return nullptr;
  }

  RawPtr<Element> StaticHTMLCollection::operator[](DOMStringView name) noexcept
  {
    return NamedItem(name);
  }

  RawPtr<const Element> StaticHTMLCollection::operator[](DOMStringView name) const noexcept
  {
    return NamedItem(name);
  }

  size_t StaticHTMLCollection::Length() const noexcept
  {
    return _elements.size();
  }
}
