#include "Krystal.Booey/HTML/RadioNodeList.hpp"
#include "Krystal.Booey/DOM/Algorithms/SubtreeRanges.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/HTML/FormListedElement.hpp"
#include "Krystal.Booey/HTML/HTMLFormElement.hpp"
#include "Krystal.Booey/HTML/HTMLInputElement.hpp"

namespace krys::boo::html
{
  RadioNodeList::RadioNodeList(dom::ContainerNode &ownerNode, dom::DOMStringView name) noexcept
      : _ownerNode(ShareRef(ownerNode)), _name(name), _isRootedAtTreeScope(Is<HTMLFormElement>(ownerNode))
  {
  }

  List<Ref<dom::Element>> RadioNodeList::Elements() const noexcept
  {
    // A form's list is rooted at its tree, as its controls can be anywhere in it.
    dom::Node &root = _isRootedAtTreeScope ? dom::TreeQueries::Root(*_ownerNode) : *_ownerNode;
    List<Ref<dom::Element>> elements;
    for (auto &node : dom::DescendantRange(root))
    {
      if (auto *element = DynamicDowncast<dom::Element>(node); element && ElementMatches(*element))
      {
        elements.push_back(ShareRef(*element));
      }
    }
    return elements;
  }

  RefPtr<dom::Node> RadioNodeList::Item(size_t index) noexcept
  {
    auto elements = Elements();
    return index < elements.size() ? RefPtr<dom::Node>(elements[index]) : nullptr;
  }

  RefPtr<const dom::Node> RadioNodeList::Item(size_t index) const noexcept
  {
    return const_cast<RadioNodeList *>(this)->Item(index);
  }

  RefPtr<dom::Node> RadioNodeList::operator[](size_t index) noexcept
  {
    return Item(index);
  }

  RefPtr<const dom::Node> RadioNodeList::operator[](size_t index) const noexcept
  {
    return Item(index);
  }

  size_t RadioNodeList::Length() const noexcept
  {
    return Elements().size();
  }

  dom::DOMString RadioNodeList::Value() const noexcept
  {
    // 1. Let element be the first element in tree order represented by the RadioNodeList object that is an
    //    input element whose type attribute is in the Radio Button state and whose checkedness is true.
    //    Otherwise, let it be null.
    for (auto &element : Elements())
    {
      auto *input = DynamicDowncast<HTMLInputElement>(element.get());
      if (input && input->IsRadioButton() && input->Checked())
      {
        // 3. If element is an element with no value attribute, return the string "on".
        // 4. Otherwise, return the value of element's value attribute.
        return input->GetAttribute(u8"value").value_or(u8"on");
      }
    }

    // 2. If element is null, return the empty string.
    return {};
  }

  void RadioNodeList::SetValue(dom::DOMStringView value) noexcept
  {
    // 1. If the new value is the string "on": let element be the first element in tree order represented by
    //    the RadioNodeList object that is an input element whose type attribute is in the Radio Button state
    //    and whose value content attribute is either absent, or present and equal to the new value, if any.
    //    If no such element exists, then instead let element be null. Otherwise: let element be the first
    //    element in tree order represented by the RadioNodeList object that is an input element whose type
    //    attribute is in the Radio Button state and whose value content attribute is present and equal to the
    //    new value, if any. If no such element exists, then instead let element be null.
    for (auto &element : Elements())
    {
      auto *input = DynamicDowncast<HTMLInputElement>(element.get());
      if (!input || !input->IsRadioButton())
      {
        continue;
      }
      auto valueAttribute = input->GetAttribute(u8"value");
      if (valueAttribute ? *valueAttribute == value : value == u8"on")
      {
        // 2. If element is not null, then set its checkedness to true.
        input->SetChecked(true);
        return;
      }
    }
  }

  bool RadioNodeList::ElementMatches(const dom::Element &element) const noexcept
  {
    if (!element.IsFormListedElement())
    {
      return false;
    }

    if (auto *input = DynamicDowncast<HTMLInputElement>(element); input && input->IsImageButton())
    {
      return false;
    }

    if (Is<HTMLFormElement>(*_ownerNode))
    {
      auto *form = const_cast<HTMLElement &>(Downcast<HTMLElement>(element)).AsFormListedElement()->Form();
      if (form != _ownerNode.get())
      {
        return false;
      }
    }

    return element.GetAttribute(u8"id") == _name || element.GetAttribute(u8"name") == _name;
  }
}
