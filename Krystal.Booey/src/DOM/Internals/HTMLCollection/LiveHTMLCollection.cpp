#include "Krystal.Booey/DOM/Internals/HTMLCollection/LiveHTMLCollection.hpp"
#include "Krystal.Booey/DOM/AbortSignal.hpp"
#include "Krystal.Booey/DOM/Algorithms/ElementAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/SubtreeRanges.hpp"
#include "Krystal.Booey/DOM/Attr.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/HTML/HTMLSlotElement.hpp"
#include "Krystal.Booey/Infra/Namespaces.hpp"

namespace krys::boo::dom
{
  LiveHTMLCollection::LiveHTMLCollection(ContainerNode &root, LiveHTMLCollectionFilterFunc &&filter) noexcept
      : _root(ShareRef(root)), _filter(krys::move(filter))
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
    // 1. If key is the empty string, return null.
    if (name.empty())
    {
      return nullptr;
    }

    // 2. Return the first element in the collection for which at least one of the following is true:
    //    - it has an ID which is key;
    //    - it is in the HTML namespace and has a name attribute whose value is key;
    //    or null if there is no such element.
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

      if (element.NamespaceURI() == infra::Namespaces::HTML
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
    // 1. Let result be an empty list.
    List<DOMString> supportedNames;

    // 2. For each element represented by the collection, in tree order:
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

      // 2a. If element has an ID which is not in result, append element's ID to result.
      auto id = element.Id();
      if (!id.empty() && !std::ranges::contains(supportedNames, id))
      {
        supportedNames.push_back(id);
      }

      // 2b. If element is in the HTML namespace and has a name attribute whose value is neither the empty
      // string
      //     nor is in result, append element's name attribute value to result.
      if (element.NamespaceURI() == infra::Namespaces::HTML)
      {
        auto name = ElementAlgorithms::GetAttributeValue(element, u8"name");
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
