#include "Krystal.HTML/NodeList/HTMLCollection.hpp"
#include "Krystal.HTML/Abort/AbortSignal.hpp"
#include "Krystal.HTML/Algorithms/TreeQueries.hpp"
#include "Krystal.HTML/MutationObserver/MutationObserver.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/Node/ContainerNode.hpp"
#include "Krystal.HTML/Node/CustomElementRegistry.hpp"
#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"

namespace Krys::HTML
{
  HTMLCollection::HTMLCollection(ContainerNode &owner) noexcept : _owner(ShareRef(owner)), _invalid(true)
  {
  }

  RawPtr<Element> HTMLCollection::Item(size_t index) noexcept
  {
    if (_invalid)
    {
      BuildCollection();
    }

    if (index < _elements.size())
    {
      return _elements[index].get();
    }

    return nullptr;
  }

  RawPtr<Element> HTMLCollection::operator[](size_t index) noexcept
  {
    if (_invalid)
    {
      BuildCollection();
    }

    if (index < _elements.size())
    {
      return _elements[index].get();
    }

    return nullptr;
  }

  RawPtr<Element> HTMLCollection::NamedItem(const DOMString &idOrName) noexcept
  {
    if (_invalid)
    {
      BuildCollection();
    }

    // TODO(IMPL): Implement this method
    return nullptr;
  }

  RawPtr<Element> HTMLCollection::operator[](const DOMString &idOrName) noexcept
  {
    if (_invalid)
    {
      BuildCollection();
    }

    // TODO(IMPL): Implement this method
    return nullptr;
  }

  size_t HTMLCollection::Length() noexcept
  {
    if (_invalid)
    {
      BuildCollection();
    }

    return _elements.size();
  }

  void HTMLCollection::BuildCollection() noexcept
  {
    _invalid = false;
    _elements.clear();
    TreeQueries::CollectChildElements(*_owner, _elements);
  }
}
