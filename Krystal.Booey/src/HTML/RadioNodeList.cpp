#include "Krystal.Booey/HTML/RadioNodeList.hpp"
#include "Krystal.Booey/DOM/Algorithms/SubtreeRanges.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/HTML/FormListedElement.hpp"
#include "Krystal.Booey/HTML/HTMLFormElement.hpp"
#include "Krystal.Booey/HTML/HTMLInputElement.hpp"

namespace krys::boo::html
{
  namespace
  {
    KRYS_NODISCARD HTMLInputElement *NonEmptyRadioButton(dom::Node &node) noexcept
    {
      auto *inputElement = DynamicDowncast<HTMLInputElement>(node);
      if (!inputElement)
      {
        return nullptr;
      }

      if (!inputElement->IsRadioButton() || inputElement->Value().empty())
      {
        return nullptr;
      }
      return inputElement;
    }
  }

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
    for (auto &element : Elements())
    {
      if (auto *button = NonEmptyRadioButton(*element))
      {
        if (button->Checked())
        {
          return button->Value();
        }
      }
    }
    return {};
  }

  void RadioNodeList::SetValue(dom::DOMStringView value) noexcept
  {
    for (auto &element : Elements())
    {
      if (auto *button = NonEmptyRadioButton(*element))
      {
        if (button->Value() == value)
        {
          button->SetChecked(true);
          return;
        }
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
