#include "Krystal.Booey/HTML/Elements/HTMLDataListElement.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Collections/LiveHTMLCollection.hpp"
#include "Krystal.Booey/DOM/Internals/IdTargetObserverRegistry.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLOptionElement.hpp"

namespace krys::boo::html
{
  HTMLDataListElement::HTMLDataListElement(dom::Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::DataList)
  {
  }

  Ref<dom::HTMLCollection> HTMLDataListElement::Options() noexcept
  {
    if (auto *options = _options.get())
    {
      return ShareRef(*options);
    }
    // The options IDL attribute must return an HTMLCollection rooted at the datalist node, whose filter
    // matches option elements.
    Ref<dom::HTMLCollection> options = CreateRef<dom::LiveHTMLCollection>(
      *this, [](const dom::Element &element) { return Is<HTMLOptionElement>(element); });
    _options = CreateWeakPtr<dom::HTMLCollection>(options.get());
    return options;
  }

  List<Ref<HTMLOptionElement>> HTMLDataListElement::Suggestions() const noexcept
  {
    List<Ref<HTMLOptionElement>> suggestions;
    for (auto *node = dom::TreeQueries::Next(*this, this); node; node = dom::TreeQueries::Next(*node, this))
    {
      if (auto *option = DynamicDowncast<HTMLOptionElement>(*node);
          option && !option->IsDisabledFormControl() && !option->Value().empty())
      {
        suggestions.push_back(ShareRef(*option));
      }
    }
    return suggestions;
  }

  void HTMLDataListElement::OptionElementChildrenChanged() noexcept
  {
    if (auto id = GetAttribute(u8"id"); id && !id->empty() && IsConnected())
    {
      TreeScope().IdTargetObserverRegistry().NotifyObservers(*this, dom::DOMStringAtom(*id));
    }
  }

  void HTMLDataListElement::OnChildrenChanged() noexcept
  {
    HTMLElement::OnChildrenChanged();
    OptionElementChildrenChanged();
  }
}