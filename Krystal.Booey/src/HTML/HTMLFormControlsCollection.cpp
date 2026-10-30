#include "Krystal.Booey/HTML/HTMLFormControlsCollection.hpp"
#include "Krystal.Booey/HTML/FormListedElement.hpp"
#include "Krystal.Booey/HTML/HTMLFormElement.hpp"
#include "Krystal.Booey/HTML/RadioNodeList.hpp"
#include <algorithm>

namespace krys::boo::html
{
  HTMLFormControlsCollection::HTMLFormControlsCollection(HTMLFormElement &ownerNode) noexcept
      : _ownerNode(ShareRef(ownerNode))
  {
  }

  List<Ref<HTMLElement>> HTMLFormControlsCollection::Elements() const noexcept
  {
    List<Ref<HTMLElement>> elements;
    for (auto &element : _ownerNode->CopyListedElementsVector())
    {
      if (element->AsFormListedElement()->IsEnumeratable())
      {
        elements.push_back(element);
      }
    }
    return elements;
  }

  RefPtr<dom::Element> HTMLFormControlsCollection::Item(size_t index) noexcept
  {
    auto elements = Elements();
    return index < elements.size() ? RefPtr<dom::Element>(elements[index]) : nullptr;
  }

  RefPtr<const dom::Element> HTMLFormControlsCollection::Item(size_t index) const noexcept
  {
    return const_cast<HTMLFormControlsCollection *>(this)->Item(index);
  }

  RefPtr<dom::Element> HTMLFormControlsCollection::operator[](size_t index) noexcept
  {
    return Item(index);
  }

  RefPtr<const dom::Element> HTMLFormControlsCollection::operator[](size_t index) const noexcept
  {
    return Item(index);
  }

  size_t HTMLFormControlsCollection::Length() const noexcept
  {
    return Elements().size();
  }

  List<Ref<dom::Element>> HTMLFormControlsCollection::NamedItems(dom::DOMStringView name) const noexcept
  {
    List<Ref<dom::Element>> result;
    if (name.empty())
    {
      return result;
    }

    // FIXME: The form's image elements, which follow its listed elements, once images register with forms.
    auto elements = Elements();
    for (auto &element : elements)
    {
      if (element->GetAttribute(u8"id") == name)
      {
        result.push_back(element);
      }
    }
    for (auto &element : elements)
    {
      auto id = element->GetAttribute(u8"id");
      if (element->GetAttribute(u8"name") == name && id != name)
      {
        result.push_back(element);
      }
    }
    return result;
  }

  RefPtr<dom::Element> HTMLFormControlsCollection::NamedItem(dom::DOMStringView name) noexcept
  {
    auto namedItems = NamedItems(name);
    return namedItems.empty() ? nullptr : RefPtr<dom::Element>(namedItems.front());
  }

  RefPtr<const dom::Element> HTMLFormControlsCollection::NamedItem(dom::DOMStringView name) const noexcept
  {
    return const_cast<HTMLFormControlsCollection *>(this)->NamedItem(name);
  }

  RefPtr<dom::Element> HTMLFormControlsCollection::operator[](dom::DOMStringView name) noexcept
  {
    return NamedItem(name);
  }

  RefPtr<const dom::Element> HTMLFormControlsCollection::operator[](dom::DOMStringView name) const noexcept
  {
    return NamedItem(name);
  }

  Maybe<Variant<RefPtr<RadioNodeList>, RefPtr<dom::Element>>>
    HTMLFormControlsCollection::NamedItemOrItems(dom::DOMStringView name) const noexcept
  {
    auto namedItems = NamedItems(name);
    if (namedItems.empty())
    {
      return std::nullopt;
    }

    if (namedItems.size() == 1)
    {
      return Variant<RefPtr<RadioNodeList>, RefPtr<dom::Element>> {RefPtr<dom::Element>(namedItems[0])};
    }

    return Variant<RefPtr<RadioNodeList>, RefPtr<dom::Element>> {
      RefPtr<RadioNodeList>(_ownerNode->RadioNodeListForName(name))};
  }

  List<dom::DOMString> HTMLFormControlsCollection::SupportedPropertyNames() const noexcept
  {
    List<dom::DOMString> names;
    auto append = [&](Maybe<dom::DOMString> &&name)
    {
      if (name && !name->empty() && std::ranges::find(names, *name) == names.end())
      {
        names.push_back(krys::move(*name));
      }
    };
    for (auto &element : Elements())
    {
      append(element->GetAttribute(u8"id"));
      append(element->GetAttribute(u8"name"));
    }
    return names;
  }
}
