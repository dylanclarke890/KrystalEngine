#include "Krystal.Booey/HTML/Collections/HTMLFormControlsCollection.hpp"
#include "Krystal.Booey/HTML/Collections/RadioNodeList.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLFormElement.hpp"
#include "Krystal.Booey/HTML/Forms/FormListedElement.hpp"
#include <algorithm>

namespace krys::boo::html
{
  HTMLFormControlsCollection::HTMLFormControlsCollection(HTMLFormElement &ownerNode) noexcept
      : LiveHTMLCollection(
          ownerNode, [](const dom::Element &) { return true; }, dom::LiveCollectionInvalidation::FormControls,
          // Its controls can be anywhere in the form's tree.
          dom::LiveCollectionTraversal::OwnerTree),
        _ownerNode(ShareRef(ownerNode))
  {
  }

  void HTMLFormControlsCollection::InvalidateCache() noexcept
  {
    _cachedElement = nullptr;
    _hasNamedItemMaps = false;
    _itemsById.clear();
    _itemsByName.clear();
    LiveHTMLCollection::InvalidateCache();
  }

  HTMLElement *HTMLFormControlsCollection::EnumeratableFrom(size_t offset) const noexcept
  {
    auto &elements = _ownerNode->ListedElements();
    for (size_t i = offset; i < elements.size(); ++i)
    {
      auto *element = elements[i].get();
      if (element && element->AsFormListedElement()->IsEnumeratable())
      {
        _cachedElement = element;
        _cachedElementOffset = i;
        return element;
      }
    }
    return nullptr;
  }

  dom::Element *HTMLFormControlsCollection::CollectionBegin() const noexcept
  {
    return EnumeratableFrom(0);
  }

  dom::Element *HTMLFormControlsCollection::CollectionNext(dom::Element &element) const noexcept
  {
    if (_cachedElement == &element)
    {
      return EnumeratableFrom(_cachedElementOffset + 1);
    }

    auto &elements = _ownerNode->ListedElements();
    auto position = std::ranges::find_if(elements, [&](auto &listed) { return listed.get() == &element; });
    krys_debug_assert(position != elements.end());
    return EnumeratableFrom(static_cast<size_t>(position - elements.begin()) + 1);
  }

  List<Ref<dom::Element>> HTMLFormControlsCollection::NamedItems(dom::DOMStringView name) const noexcept
  {
    List<Ref<dom::Element>> result;
    if (name.empty())
    {
      return result;
    }

    // FIXME: The form's image elements, which follow its listed elements, once images register with forms.
    if (!_hasNamedItemMaps)
    {
      for (size_t i = 0, length = Length(); i < length; ++i)
      {
        auto element = Item(i);
        auto id = element->GetAttribute(u8"id");
        if (id && !id->empty())
        {
          _itemsById[*id].push_back(const_cast<dom::Element *>(element.get()));
        }
        auto elementName = element->GetAttribute(u8"name");
        if (elementName && !elementName->empty() && id != elementName)
        {
          _itemsByName[*elementName].push_back(const_cast<dom::Element *>(element.get()));
        }
      }
      _hasNamedItemMaps = true;
    }

    auto key = dom::DOMString(name);
    for (auto *items : {&_itemsById, &_itemsByName})
    {
      if (auto it = items->find(key); it != items->end())
      {
        for (auto *element : it->second)
        {
          result.push_back(ShareRef(*element));
        }
      }
    }
    return result;
  }

  RefPtr<dom::Element> HTMLFormControlsCollection::NamedItem(dom::DOMStringView name) noexcept
  {
    auto namedItems = NamedItems(name);
    return namedItems.empty() ? nullptr : RefPtr<dom::Element>(namedItems.front());
  }

  Maybe<Variant<RefPtr<RadioNodeList>, RefPtr<dom::Element>>>
    HTMLFormControlsCollection::NamedItemOrItems(dom::DOMStringView name) const noexcept
  {
    // 1. If name is the empty string, return null and stop the algorithm. (NamedItems.)
    auto namedItems = NamedItems(name);

    // 3. Otherwise, if there are no nodes in the collection that have either an id attribute or a name
    //    attribute equal to name, then return null and stop the algorithm.
    if (namedItems.empty())
    {
      return std::nullopt;
    }

    // 2. If, at the time the method is called, there is exactly one node in the collection that has either an
    //    id attribute or a name attribute equal to name, then return that node and stop the algorithm.
    if (namedItems.size() == 1)
    {
      return Variant<RefPtr<RadioNodeList>, RefPtr<dom::Element>> {RefPtr<dom::Element>(namedItems[0])};
    }

    // 4. Otherwise, create a new RadioNodeList object representing a live view of the
    //    HTMLFormControlsCollection object, further filtered so that the only nodes in the RadioNodeList
    //    object are those that have either an id attribute or a name attribute equal to name. The nodes in
    //    the RadioNodeList object must be sorted in tree order.
    // 5. Return that RadioNodeList object.
    // (The form keeps one per name, so the same one is returned while it lives.)
    return Variant<RefPtr<RadioNodeList>, RefPtr<dom::Element>> {
      RefPtr<RadioNodeList>(_ownerNode->RadioNodeListForName(name))};
  }
}
