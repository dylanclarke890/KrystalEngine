#include "Krystal.HTML/DOM/Internals/HTMLCollection/LiveHTMLCollection.hpp"
#include "Krystal.HTML/DOM/AbortSignal.hpp"
#include "Krystal.HTML/DOM/Algorithms/ElementAlgorithms.hpp"
#include "Krystal.HTML/DOM/Algorithms/SubtreeRanges.hpp"
#include "Krystal.HTML/DOM/Attr.hpp"
#include "Krystal.HTML/DOM/Element.hpp"
#include "Krystal.HTML/DOM/ShadowRoot.hpp"
#include "Krystal.HTML/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/HTML/HTMLSlotElement.hpp"
#include "Krystal.HTML/Infra/Namespaces.hpp"

namespace Krys::HTML
{
  LiveHTMLCollection::LiveHTMLCollection(ContainerNode &root, LiveHTMLCollectionFilterFunc &&filter) noexcept
      : _root(ShareRef(root)), _filter(Krys::Move(filter))
  {
  }

  RefPtr<Element> LiveHTMLCollection::Item(size_t index) noexcept
  {
    size_t count = 0uz;
    for (Node &node : DescendantRange(*_root))
    {
      if (!Is<Element>(node))
      {
        continue;
      }

      auto &element = Downcast<Element>(node);
      if (!_filter(element))
      {
        continue;
      }

      ++count;
      if (count > index)
      {
        return ShareRef(element);
      }
    }

    return nullptr;
  }

  RefPtr<const Element> LiveHTMLCollection::Item(size_t index) const noexcept
  {
    return const_cast<LiveHTMLCollection *>(this)->Item(index);
  }

  RefPtr<Element> LiveHTMLCollection::operator[](size_t index) noexcept
  {
    return Item(index);
  }

  RefPtr<const Element> LiveHTMLCollection::operator[](size_t index) const noexcept
  {
    return Item(index);
  }

  RefPtr<Element> LiveHTMLCollection::NamedItem(DOMStringView name) noexcept
  {
    for (Node &node : DescendantRange(*_root))
    {
      if (!Is<Element>(node))
      {
        continue;
      }

      auto &element = Downcast<Element>(node);
      if (!_filter(element))
      {
        continue;
      }

      if (element.Id() == name)
      {
        return ShareRef(element);
      }

      if (element.NamespaceURI() == Namespace::HTML
          && ElementAlgorithms::GetAttributeValue(element, u8"name") == name)
      {
        return ShareRef(element);
      }
    }

    return nullptr;
  }

  RefPtr<const Element> LiveHTMLCollection::NamedItem(DOMStringView name) const noexcept
  {
    return const_cast<LiveHTMLCollection *>(this)->NamedItem(name);
  }

  RefPtr<Element> LiveHTMLCollection::operator[](DOMStringView name) noexcept
  {
    return NamedItem(name);
  }

  RefPtr<const Element> LiveHTMLCollection::operator[](DOMStringView name) const noexcept
  {
    return NamedItem(name);
  }

  size_t LiveHTMLCollection::Length() const noexcept
  {
    return Count(ConstDescendantRange(*_root),
                 [&](auto &&node) { return Is<Element>(node) && _filter(Downcast<Element>(node)); });
  }

  List<DOMString> LiveHTMLCollection::SupportedPropertyNames() const noexcept
  {
    List<DOMString> supportedNames;

    for (auto &node : DescendantRange(*_root))
    {
      if (!Is<Element>(node))
      {
        continue;
      }

      auto &element = Downcast<Element>(node);
      if (!_filter(element))
      {
        continue;
      }

      auto id = element.Id();
      if (!id.empty() && !std::ranges::contains(supportedNames, id))
      {
        supportedNames.push_back(id);
      }

      if (element.NamespaceURI() == Namespace::HTML)
      {
        auto name = ElementAlgorithms::GetAttributeValue(element, u8"name");
        if (!name.empty() && !std::ranges::contains(supportedNames, name))
        {
          supportedNames.push_back(name);
        }
      }
    }

    return supportedNames;
  }
}
