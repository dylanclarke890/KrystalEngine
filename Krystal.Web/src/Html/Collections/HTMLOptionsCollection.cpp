#include "Krystal.Web/Html/Collections/HTMLOptionsCollection.hpp"
#include "Krystal.Web/Html/CustomElement/CustomElementReactionQueue.hpp"
#include "Krystal.Web/Html/Elements/HTMLOptGroupElement.hpp"
#include "Krystal.Web/Html/Elements/HTMLOptionElement.hpp"
#include "Krystal.Web/Html/Elements/HTMLSelectElement.hpp"

namespace krys::web::html
{
  HTMLOptionsCollection::HTMLOptionsCollection(HTMLSelectElement &select) noexcept
      : LiveHTMLCollection(select,
                           [&select](const Element &element)
                           {
                             auto *option = DynamicDowncast<HTMLOptionElement>(element);
                             return option && option->OwnerSelectElement() == &select;
                           }),
        _selectElement(ShareRef(select))
  {
  }

  ExceptionOr<void> HTMLOptionsCollection::Length(uint32 length) noexcept
  {
    CEReactionsScope ceReactions;
    return _selectElement->Length(length);
  }

  ExceptionOr<void> HTMLOptionsCollection::SetItem(uint32 index, HTMLOptionElement *option) noexcept
  {
    CEReactionsScope ceReactions;
    return _selectElement->SetItem(index, option);
  }

  ExceptionOr<void> HTMLOptionsCollection::Add(const OptionOrOptGroupElement &element) noexcept
  {
    return Add(element, null);
  }

  ExceptionOr<void> HTMLOptionsCollection::Add(const OptionOrOptGroupElement &element,
                                               const Maybe<HTMLElementOrInt> &before) noexcept
  {
    CEReactionsScope ceReactions;
    return _selectElement->Add(element, before);
  }

  void HTMLOptionsCollection::Remove(int32 index) noexcept
  {
    CEReactionsScope ceReactions;
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
