#include "Krystal.Web/Html/Elements/HTMLDataListElement.hpp"
#include "Krystal.Web/Dom/Collections/LiveHTMLCollection.hpp"
#include "Krystal.Web/Dom/Internals/IdTargetObserverRegistry.hpp"
#include "Krystal.Web/Dom/Trees/TreeQueries.hpp"
#include "Krystal.Web/Html/Elements/HTMLOptionElement.hpp"

namespace krys::web
{
  HTMLDataListElement::HTMLDataListElement(Document &document) noexcept
      : HTMLElement(document, html::HTMLElementInterface::DataList)
  {
  }

  Ref<HTMLCollection> HTMLDataListElement::Options() noexcept
  {
    if (auto *options = _options.get())
    {
      return ShareRef(*options);
    }
    // The options IDL attribute must return an HTMLCollection rooted at the datalist node, whose filter
    // matches option elements.
    Ref<HTMLCollection> options = CreateRef<LiveHTMLCollection>(*this, [](const Element &element)
                                                                { return Is<HTMLOptionElement>(element); });
    _options = CreateWeakPtr<HTMLCollection>(options.get());
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
      TreeScope().NotifyIdTargetObservers(*this, DOMStringAtom(*id));
    }
  }

  void HTMLDataListElement::OnChildrenChanged() noexcept
  {
    HTMLElement::OnChildrenChanged();
    OptionElementChildrenChanged();
  }
}
