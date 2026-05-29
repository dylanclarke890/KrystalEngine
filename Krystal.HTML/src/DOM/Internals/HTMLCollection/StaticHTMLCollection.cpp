#include "Krystal.HTML/DOM/Internals/HTMLCollection/StaticHTMLCollection.hpp"
#include "Krystal.HTML/DOM/AbortSignal.hpp"
#include "Krystal.HTML/DOM/Algorithms/ElementAlgorithms.hpp"
#include "Krystal.HTML/DOM/Algorithms/SubtreeRanges.hpp"
#include "Krystal.HTML/DOM/Attr.hpp"
#include "Krystal.HTML/DOM/ContainerNode.hpp"
#include "Krystal.HTML/DOM/Document.hpp"
#include "Krystal.HTML/DOM/Element.hpp"
#include "Krystal.HTML/DOM/ShadowRoot.hpp"
#include "Krystal.HTML/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/HTML/HTMLSlotElement.hpp"
#include "Krystal.HTML/Infra/Namespaces.hpp"

namespace Krys::HTML
{
  StaticHTMLCollection::StaticHTMLCollection(SmallElementList &&elements) noexcept
      : _elements(Krys::Move(elements))
  {
  }

  RefPtr<Element> StaticHTMLCollection::Item(size_t index) noexcept
  {
    if (index < _elements.size())
    {
      return _elements[index];
    }

    return nullptr;
  }

  RefPtr<const Element> StaticHTMLCollection::Item(size_t index) const noexcept
  {
    return const_cast<StaticHTMLCollection *>(this)->Item(index);
  }

  RefPtr<Element> StaticHTMLCollection::operator[](size_t index) noexcept
  {
    return Item(index);
  }

  RefPtr<const Element> StaticHTMLCollection::operator[](size_t index) const noexcept
  {
    return Item(index);
  }

  RefPtr<Element> StaticHTMLCollection::NamedItem(DOMStringView name) noexcept
  {
    for (auto &element : _elements)
    {
      if (element->Id() == name)
      {
        return element;
      }
      if (element->NamespaceURI() == Namespace::HTML
          && ElementAlgorithms::GetAttributeValue(*element, u8"name") == name)
      {
        return element;
      }
    }

    return nullptr;
  }

  RefPtr<const Element> StaticHTMLCollection::NamedItem(DOMStringView name) const noexcept
  {
    return const_cast<StaticHTMLCollection *>(this)->NamedItem(name);
  }

  RefPtr<Element> StaticHTMLCollection::operator[](DOMStringView name) noexcept
  {
    return NamedItem(name);
  }

  RefPtr<const Element> StaticHTMLCollection::operator[](DOMStringView name) const noexcept
  {
    return NamedItem(name);
  }

  size_t StaticHTMLCollection::Length() const noexcept
  {
    return _elements.size();
  }

  List<DOMString> StaticHTMLCollection::SupportedPropertyNames() const noexcept
  {
    List<DOMString> supportedNames;

    for (auto &element : _elements)
    {
      auto id = element->Id();
      if (!id.empty() && !std::ranges::contains(supportedNames, id))
      {
        supportedNames.push_back(id);
      }

      if (element->NamespaceURI() == Namespace::HTML)
      {
        auto name = ElementAlgorithms::GetAttributeValue(*element, u8"name");
        if (!name.empty() && !std::ranges::contains(supportedNames, name))
        {
          supportedNames.push_back(name);
        }
      }
    }

    return supportedNames;
  }
}
