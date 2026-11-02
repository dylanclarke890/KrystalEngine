#include "Krystal.Booey/HTML/HTMLDataListElement.hpp"
#include "Krystal.Booey/DOM/Collections/LiveHTMLCollection.hpp"
#include "Krystal.Booey/HTML/HTMLOptionElement.hpp"

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
}