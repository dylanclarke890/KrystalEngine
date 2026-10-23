#include "Krystal.Booey/HTML/HTMLOptionsCollection.hpp"
#include "Krystal.Booey/HTML/HTMLOptGroupElement.hpp"
#include "Krystal.Booey/HTML/HTMLOptionElement.hpp"
#include "Krystal.Booey/HTML/HTMLSelectElement.hpp"

namespace krys::boo::html
{
  HTMLOptionsCollection::HTMLOptionsCollection(HTMLSelectElement &select) noexcept
      : LiveHTMLCollection(select,
                           [&select](const dom::Element &element)
                           {
                             auto *option = DynamicDowncast<HTMLOptionElement>(element);
                             return option && option->OwnerSelectElement() == &select;
                           }),
        _selectElement(ShareRef(select))
  {
  }

  dom::ExceptionOr<void> HTMLOptionsCollection::Length(uint32 length) noexcept
  {
    return _selectElement->Length(length);
  }

  dom::ExceptionOr<void> HTMLOptionsCollection::SetItem(uint32 index, HTMLOptionElement *option) noexcept
  {
    return _selectElement->SetItem(index, option);
  }

  dom::ExceptionOr<void> HTMLOptionsCollection::Add(const OptionOrOptGroupElement &element,
                                                    const Maybe<HTMLElementOrInt> &before) noexcept
  {
    return _selectElement->Add(element, before);
  }

  void HTMLOptionsCollection::Remove(int32 index) noexcept
  {
    _selectElement->Remove(index);
  }

  int32 HTMLOptionsCollection::SelectedIndex() const noexcept
  {
    return _selectElement->SelectedIndex();
  }

  void HTMLOptionsCollection::SelectedIndex(int32 index) noexcept
  {
    _selectElement->SelectedIndex(index);
  }
}
