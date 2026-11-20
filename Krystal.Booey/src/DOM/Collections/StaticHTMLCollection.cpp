#include "Krystal.Booey/DOM/Collections/StaticHTMLCollection.hpp"
#include "Krystal.Booey/DOM/Algorithms/ElementAlgorithms.hpp"
#include "Krystal.Booey/DOM/Iterators/SubtreeRanges.hpp"
#include "Krystal.Booey/DOM/Nodes/Attr.hpp"
#include "Krystal.Booey/DOM/Nodes/ContainerNode.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/DOM/Nodes/ShadowRoot.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLSlotElement.hpp"
#include "Krystal.Booey/Infra/Namespaces.hpp"

namespace krys::boo
{
  dom::StaticHTMLCollection::StaticHTMLCollection(dom::SmallElementList &&elements) noexcept
      : _elements(krys::move(elements))
  {
  }

  RefPtr<Element> dom::StaticHTMLCollection::Item(size_t index) noexcept
  {
    if (index < _elements.size())
    {
      return _elements[index];
    }

    return nullptr;
  }

  RefPtr<const Element> dom::StaticHTMLCollection::Item(size_t index) const noexcept
  {
    return const_cast<dom::StaticHTMLCollection *>(this)->Item(index);
  }

  RefPtr<Element> dom::StaticHTMLCollection::operator[](size_t index) noexcept
  {
    return Item(index);
  }

  RefPtr<const Element> dom::StaticHTMLCollection::operator[](size_t index) const noexcept
  {
    return Item(index);
  }

  RefPtr<Element> dom::StaticHTMLCollection::NamedItem(DOMStringView name) noexcept
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
    for (auto &element : _elements)
    {
      if (element->Id() == name)
      {
        return element;
      }
      if (element->NamespaceURI() == infra::namespaces::HTML
          && dom::ElementAlgorithms::GetAttributeValue(*element, u8"name") == name)
      {
        return element;
      }
    }

    return nullptr;
  }

  RefPtr<const Element> dom::StaticHTMLCollection::NamedItem(DOMStringView name) const noexcept
  {
    return const_cast<dom::StaticHTMLCollection *>(this)->NamedItem(name);
  }

  RefPtr<Element> dom::StaticHTMLCollection::operator[](DOMStringView name) noexcept
  {
    return NamedItem(name);
  }

  RefPtr<const Element> dom::StaticHTMLCollection::operator[](DOMStringView name) const noexcept
  {
    return NamedItem(name);
  }

  size_t dom::StaticHTMLCollection::Length() const noexcept
  {
    return _elements.size();
  }

  List<DOMString> dom::StaticHTMLCollection::SupportedPropertyNames() const noexcept
  {
    // 1. Let result be an empty list.
    List<DOMString> supportedNames;

    // 2. For each element represented by the collection, in tree order:
    for (auto &element : _elements)
    {
      // 2a. If element has an ID which is not in result, append element's ID to result.
      auto id = element->Id();
      if (!id.empty() && !std::ranges::contains(supportedNames, id))
      {
        supportedNames.push_back(id);
      }

      // 2b. If element is in the HTML namespace and has a name attribute whose value is neither the empty
      // string
      //     nor is in result, append element's name attribute value to result.
      if (element->NamespaceURI() == infra::namespaces::HTML)
      {
        auto name = dom::ElementAlgorithms::GetAttributeValue(*element, u8"name");
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
